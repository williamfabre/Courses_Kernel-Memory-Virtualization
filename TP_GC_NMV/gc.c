#include "gc.h"
#include "alloc.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "debug.h"

// Descripteur de thread
// On a un descripteur par thread
// Ils sont chaînés pour que le collecteur puisse se promener dedans
struct thread_descriptor {
    struct thread_descriptor *next;      // descripteur précédent
    struct thread_descriptor *prev;      // descripteur suivant
    char *                    top_stack; // haut de pile
    // ajoutez vos champs de threads ici
    // vous avez besoin d'une structure pour mettre vos racines sur votre pile
    // et vous avez besoin de drapeau pour indiquer qu collecteur que vous avez fini de
    // placer les racines la dedans
    // A FAIRE : EXO 1 - Q2
};

// Les threads d'un même processus partagent le même espace d'adressage, les données
// contenus dans une variable globale/statique se trouvent exactement au
// même emplacement mémoire pour tous les threads.
// "Thread Local Storage" est un mecanisme permettant d'avoir une instance
// de la variable par thread : ce sont des variables locales au thread.
// (exemple: la variable "errno" du langage)
// Plus d'infos : https://gcc.gnu.org/onlinedocs/gcc-3.3.1/gcc/Thread-Local.html
static __thread struct thread_descriptor tls;

// Point d'entrée de la liste chaînées de tous les threads.
// Ce noeud est vide, il sert de référence.
// c'est juste une simplification pour les listes doublement chaînées circulaires.
static struct thread_descriptor          all_threads;

// Et un petit mutex lorsqu'on touche à cette liste.
static pthread_mutex_t                   thread_mutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef _DEBUG
// permet d'afficher les threads actifs
// ça vous donne un exemple de parcours de la liste chaînée all_threads
void print_threads() {
    pthread_mutex_lock(&thread_mutex);
    dprintf("thread", "Current threads:");
    for(struct thread_descriptor *cur=all_threads.next; cur!=&all_threads; cur=cur->next)
        dprintf("thread", "\tthread with tls at: %p", &cur);
    pthread_mutex_unlock(&thread_mutex);
}
#else
#define print_threads()
#endif

// Voir description dans gc.h
// Vous devez remplir cette fonction
void *gcmalloc(unsigned int size)
{
    // L'allocation se fait en suivant l'algo de hash de Boehm
    struct object_header *header = pre_malloc(size);
    
    // ensuite, vous devrez mettre cette entête dans une liste des objets vivants...
    // vous pouvez prendre un verrou ici, mais vous pouvez aussi utiliser le tls: l'ensemble des objets vivants
    // est stocké dans l'ensemble des tls. Ca vous évite un verrou de plus
    
    // A FAIRE : EXO 1 - Q 3
    
    // pour le retour, l'utilisateur est intéressé par l'objet, pas par son entête
    return toObject(header);
}

// la fonction writeBarrier qui doit assurer l'invariant tri-couleurs de boehm
void _writeBarrier(void *dst, void *src) {
    // A FAIRE
    struct object_header *header_dst = toHeader(dst);  // entête de la destination
    struct object_header *header_src = toHeader(src);  // entête de la source
    
    assert(header_dst && header_src);                  // si vous n'avez pas de bug, cet invariant est respecté
}

// le handshake pour accumuler les racines du thread. Vous les stockerez dans la variable tls
// celle-ci est ensuite accédée par le collecteur via la variable all_threads
void handShake() {
    // A FAIRE
}

static void *collector(void *arg)
{
    // A FAIRE : EXO 1 - Q5
    return 0;
}

struct stack_frame {
    struct stack_frame *next;
    char               *co;
};

// Attache un nouveau thread au GC.
// Techniquement, ajoute son tls à all_threads
void attach_thread(void *top)
{
    // 1ere etape : trouver le haut de la pile
    //struct stack_frame *top = __builtin_frame_address(0);
    //while(top->next)
    //    top = top->next;
    
    pthread_mutex_lock(&thread_mutex);
    tls.top_stack = (char *)top;
    dprintf("thread", "Attach thread with tls at %p and top stack at %p", &tls, tls.top_stack);
    (tls.prev = all_threads.prev)->next = &tls;
    (tls.next = &all_threads)->prev = &tls;
    pthread_mutex_unlock(&thread_mutex);
}

// Détache le thread, i.e retire le tls de all_threads
void detach_thread()
{
    pthread_mutex_lock(&thread_mutex);
    dprintf("thread", "Detach thread with tls at: %p", &tls);
    tls.next->prev = tls.prev;
    tls.prev->next = tls.next;
    tls.next = tls.prev = &tls;
    pthread_mutex_unlock(&thread_mutex);
}

// Fonction d'initialisation du GC :
// - Création du thread collecteur
// - Initialisation de la variable all_threads
// A FAIRE Vous devez completer cette fonction pour y ajouter vos structures de données
void initialise_gc() {
    init_bank_desc();
    pthread_t tid;
    all_threads.prev = all_threads.next = &all_threads;
    pthread_create(&tid, 0, &collector, 0);
}
