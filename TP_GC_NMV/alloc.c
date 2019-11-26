#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "alloc.h"
#include "debug.h"

#undef dprintf
#define dprintf(val, msg, args...)

// une page fait 4096 octets = 1 << 12
#define PAGE_SHIFT              12
#define PAGE_SIZE               (1 << PAGE_SHIFT)

// tous les objets doivent être alignés sur au moins 8 octets
#define OBJECT_ALIGN_SHIFT  2
#define OBJECT_ALIGN        (2<<OBJECT_ALIGN_SHIFT)

//    une adresse est décomposée ainsi
// [partie haut - partie basse - partie offset]
// la partie haute donne une entrée dans le tableau heap_desc (structure bank_desc)
// la partie basse donne une entrée dans le bank_desc associé
#define OFFSET_BITS   PAGE_SHIFT
#define LOW_BITS      10
#define HIGH_BITS     (32 - LOW_BITS - PAGE_SHIFT)

// raccourci pour mmap
// TODO mmap 0 va allouer a 0 ou au plus proche de 0.
#define MMAP(size)         (mmap(0, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0))
#define MUNMAP(addr, size) munmap(addr, size) // TODO est-ce que c'est addr = 0

// Décrit un ensemble de pages
// La mémoire est découpée en zone de plusieurs pages.
// Tous les objets d'une zone contiennent des objets de la même taille.
// Chaque page d'une zone est hashée dans la table de hash heap_desc en utilisant
// les parties hautes/basses des pages qui la compose.
// Une entrée contient soit
// - 0 : ca veut dire que la page n'est pas mappée en mémoire
// - un nombre positif : donne la taille des éléments qui se trouvent dans cette zone.
// - un nombre négatif : donne le décalage (en page) par rapport au début de la zone.
struct bank_desc {
	uint64_t pages[1ULL<<LOW_BITS];
};

// structure d'allocation d'un objet. Gère une taille particulière pour les objets de petites tailles.
// l'allocation se passe en deux temps.
// On a d'abord une liste chaînée d'objets inutilisés. Un objet ne peut se retrouver là que lorsqu'il a été libéré.
// Si cette liste est vide, on associe une zone à un object_allocator.
// On initialise current_zone vers le début de la zone et end_zone vers la fin de la zone.
// Lors d'une allocation, on incrémente current_zone de la taille des objets qui se trouvent dans la zone
// et si on dépasse le end_zone, on se réalloue une nouvelle zone.
struct object_allocator {
	struct object_header *first;         // premier élement libre (objets qui ont été alloués puis libérés)
	char *                current_zone;  // endroit où se trouve le premier objet qui n'a jamais été alloué dans la zone courante
	char *                end_zone;      // adresse max de la zone courante
};

// description du tas : c'est juste un tableau de bank_desc
static struct bank_desc            *heap_desc; //[1ULL<<(HIGH_BITS)];
void init_bank_desc()
{
	heap_desc = (struct bank_desc*)MMAP(sizeof(struct bank_desc)*(1ULL << (HIGH_BITS)));
}

// mutex global de l'allocateur
static pthread_mutex_t             mutex = PTHREAD_MUTEX_INITIALIZER;

// On a NB_ZONES différentes, de 0 à NB_ZONES*OBJECT_ALIGN par pas de OBJECT_ALIGN
#define NB_ZONES 2048                        // techniquement = 16384
struct object_allocator allocator[NB_ZONES]; // et un object_allocator par zone

// information statistiques
static unsigned long long nb_allocated;
static unsigned long long nb_free;
static unsigned long long size_allocated;
static unsigned long long size_free;
static unsigned long long size_mapped;

// alignement sur align. align doit être une puissance de deux
static inline unsigned int align(unsigned int size, unsigned int align)
{
	return (size + align - 1) & -align;
}

// Donne la taille réellement occupée par un objet :
// -> Taille de l'objet + taille de l'entête, aligné sur OBJECT_ALIGN
static inline unsigned int total_size(int size)
{
	// le 8 sert à chaîner les objets de taille 0 lorsqu'ils sont libres
	// object align c'est un alignement de 2 << 2
	return align((size ? size : 8) + sizeof(struct object_header), OBJECT_ALIGN);
}

// renvoie le bon object allocator
static struct object_allocator *get_object_allocator(int total_size)
{
	// est-ce par zone ?
	if(total_size < (NB_ZONES << OBJECT_ALIGN_SHIFT))
		return &allocator[total_size >> OBJECT_ALIGN_SHIFT]; // donne la bonne entrée
	else
		return 0;
}

// positionne l'entrée de la page dans table de hash du tas à value
static inline void set_page_desc(char *page, uint64_t value)
{
	// n est le numéro de la page
	uint64_t n = ((uint64_t)page << 32) >> (OFFSET_BITS+32);

	// n >> LOW_BITS donne l'entrée dans heap_desc
	// et n & ((1 << LOW_BITS) - 1) l'entrée dans ce descripteur
	heap_desc[n >> LOW_BITS].pages[n & ((1 << LOW_BITS) - 1)] = value;
}

// hash une zone complete.
// La zone commence en zone, fait size octets, elle contient des elements de
// elmt_size
// les entrées suivantes sont positionnées à depl, 2*depl, 3*depl etc...
// si elmt_size > 0 et depl = -1, permet de hashser
// si elmt_size = 0 et depl = 0, permet de vider les entrées
static inline void hash_zone(char *zone, unsigned int size, int elmt_size, int depl)
{
	// met zone dans la table de hash et associe la taille
	set_page_desc(zone, elmt_size);

	// et remplit les entrées suivantes pour retrouver le début de la zone
	int n=depl;
	char *cur = zone + PAGE_SIZE;
	char *end = zone + size;

	while(cur < end) {
		set_page_desc(cur, n);
		cur += PAGE_SIZE;
		n += depl;
	}
}

// Alloue une nouvelle zone de taille size contenant des éléments de taille
// elmt_size (taille complète avec header et aligné)
static char *new_zone(int size, int elmt_size)
{
	// zone est l'adresse de la nouvelle zone
	char *zone = MMAP(size);

	size_mapped += size;

	dprintf("allocator", "New_zone of size 0x%x for element of size %d at %p", size, elmt_size, zone);

	//printf("zone: %p\n", zone);
	if(zone == MAP_FAILED)
		error("unable to mmap %d bytes", size);
	else
		hash_zone(zone, size, elmt_size, -1);

	return zone;
}

// Réalise l'allocation de size octets.
// zero_filled indique si on doit la remplir avec des zéros.
static inline struct object_header *allocate_internal(unsigned int size,
						      int zero_filled)
{
	// taille réellement occupée par l'objet
	int tot = total_size(size);
	// allocateur associé (0 si size est trop grand)
	struct object_allocator *allocator = get_object_allocator(size);
	// entête du résultat
	struct object_header *res;

	pthread_mutex_lock(&mutex);

	// si c'est un petit objet géré dans une zone
	if(allocator) {
		// si on a une liste chaînée d'objets vides
		if(allocator->first) {
			// on l'utilise
			res = allocator->first;
			allocator->first = *(struct object_header **)(res + 1);
			if(zero_filled)
				memset(toObject(res), 0, size);
		} else {
			// sinon, il faut utiliser la zone courante
			res = (struct object_header *)allocator->current_zone;
			// qui peut éventuellement être totalement remplie
			if((allocator->current_zone += tot) > allocator->end_zone) {
				// dans ce cas, on alloue une nouvelle zone.
				// On considère qu'il faut au moins 32 objets dans la zone
				// pour le morcelement interne.
				int size = align(tot * 32, PAGE_SIZE);
				// notre zone (elle est déjà hashée)
				char *zone = new_zone(size, tot);

				// le résultat est le début de la zone
				res = (struct object_header *)zone;
				// on prépare la zone pour les appels suivante
				allocator->current_zone = zone + tot;
				allocator->end_zone = zone + size;
			}
		}
	} else {
		// sinon, on alloue en vrac un ensemble de pages (et on les hash)
		res = (struct object_header *)new_zone(align(tot, PAGE_SIZE), tot);
	}

	pthread_mutex_unlock(&mutex);

	// on met la taille réelle de l'objet ici (sans l'entête et pas aligné)
	size_allocated += size;
	nb_allocated ++;

	res->object_size = size;
	return res;
}

// réalise l'allocation et remplit de zéro
struct object_header *pre_malloc(unsigned int size)
{
	return allocate_internal(size, 1);
}

// libération d'un objet
void pre_free(void *p) {
	// trouve l'entête associée à l'objet
	struct object_header *header = toHeader(p);

	// si c'est un objet
	if(header) {
		// taille effective de l'objet
		int size = header->object_size;
		int tot  = total_size(size);

		// et son allocateur associé
		struct object_allocator *allocator = get_object_allocator(size);

		// si c'est un petit objet
		if(allocator) {
			// on met sa taille à 0
			header->object_size = 0;
			// et on l'ajoute à la liste chaînée allocateur->first
			*(struct object_header **)(header + 1) = allocator->first;
			allocator->first = header;
		} else {
			// taille totale alignée
			tot = align(tot, PAGE_SIZE);
			// supprime les entrées de la zone de la table de hash
			hash_zone((char *)header, tot, 0, 0);
			// et libère effectivement les pages
			MUNMAP(header, tot);
		}

		nb_free ++;
		size_free += size;
	} else
		error("%p is not an object", p);
}

// retrouve l'entête à partir du pointeur d'objet
struct object_header *toHeader(void *p) {
	// n est le numéro de page de l'objet
	uint64_t n         = ((uint64_t)p << 32) >> (OFFSET_BITS+32);
	// trouve le elmt_size à partir de la table de hash
	int elmt_size = heap_desc[n>>LOW_BITS].pages[n & ((1 << LOW_BITS) - 1)];

	// si c'est < 0, c'est qu'on est au milieu de la zone
	if(elmt_size < 0) {
		// page du début de la zone
		n += elmt_size;
		// et elmt_size est la taille des objets de cette zone
		elmt_size = heap_desc[n>>LOW_BITS].pages[n & ((1 << LOW_BITS) - 1)];
		if(elmt_size <= 0)
			error("inconsistent hash table");
	}

	// si ça correspond réellement à une zone mappée
	if(elmt_size) {
		// adresse du début de la zone
		uint64_t page_addr = n << OFFSET_BITS;

		// retrouve l'entête en suivant la règle suivante
		struct object_header *header =
			(struct object_header *)(page_addr + elmt_size * (((uint64_t)p - page_addr) / elmt_size));

		// vérifie que p est bien dans l'objet. En particulier, si header->object_size = 0, cette expression répond toujours faux
		// remarquez que si p est plus petit que le début de l'objet, ce nombre est négatif
		if((uint64_t)p - (uint64_t)toObject(header) < header->object_size)
			return header;
		else
			return 0;
	} else
		return 0;
}

// renvoie l'objet à partir de l'entête
inline void *toObject(struct object_header *header)
{
	return header + 1;
}

void print_stats() {
	uint64_t mega = 1024*1024;

	printf("Nombre total d'objets alloués : %llu objets pour %llu octets (%llu Mo)\n",
	       nb_allocated, size_allocated, size_allocated/mega);
	printf("Nombre courant d'objets alloués : %llu objets pour %llu octets (%llu Mo)\n",
	       nb_allocated - nb_free, size_allocated - size_free, (size_allocated - size_free)/mega);
	printf("Mémoire effectivement mappée : %llu octets (%llu Mo)\n", size_mapped, size_mapped/mega);
}

