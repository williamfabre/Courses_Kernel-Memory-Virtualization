# CR TP MMU

## Commandes utile
info registers Affiche l’état les principaux registres de la machine.
xp/8g addr Affiche le contenu de la mémoire à l’adresse virtuelle addr. 

## Definitions :
- adresse physique :

- adresse physique d’un niveau de la table des pages : un niveau intermédiaire
- de la table des pages est mappé
par une identité (il est accessible par une adresse virtuelle égale à son
adresse physique)

- hauteur d'un niveau :


## Exercice 1
Ce premier exercice vous permet de vous familiariser avec la structure d’une
table des pages. On vous propose d’implémenter une fonction qui affiche la
structure de la table des pages courante. Cette fonction a le prototype : void
print pgt(paddr t pml, uint8 t lvl). Cette fonction recursive prend comme
paramètre l’adresse physique d’un niveau de la table des pages et la hauteur de
ce niveau.


### Question 1
Dans le cadre de ce TP on supposera toujours qu’un niveau intermédiaire de la
table des pages est mappé par une identité (il est accessible par une adresse
virtuelle égale à son adresse physique). Pourquoi cette supposition est-elle
importante ?
- Cette supposition permet de ne pas avoir a acceder a la mmu pour traduire une
- adresse qui sert
a acceder a la table de la mmu (beaucoup plus difficile)


### Question 2
Combien y a-t-il d’entrée par niveau de table des pages pour une architecture
x86 64 bits ? Comment la MMU détermine-t-elle si une entrée d’un niveau de
table des pages est valide ? Si elle est terminale ?

| nom table | taille | taille de pages | terminal/non terminal |
|---|---|---|---|
| PML4 | 2^8=256 |     | non terminal |
| PML3 | 2^8=256 |     | non terminal | 
| PML2 | 2^8=256 | 2MO | potentiellement terminal (HUGE page) |
| PML1 | 2^8=256 | 4KO | terminal |

| bit | def |
|---|---|
| P   | l’entrée est valide (QUESTION2)|
| R/W | la page est accessible en lecture/écriture |
| U/S | la page est accessible en mode utilisateur |
| PWT | les données écrites dans la page ne sont pas mises en cache |
| PCD | les données de la page sont lues depuis la RAM |
| A   | mis à 1 par le processeur quand la page est accédée |
| D   | mis à 1 par le processeur quand la page est modifiée |
| PS  | la page pointée est une huge page (QUESTION2) |
| G   | la page reste dans la TLB quand le CR3 est modifié |
| NX  | la page est inaccessible en exécution |

### Question 3
Dans kernel/main.c, programmez et testez la nouvelle fonction print pgt. Vous
testerez votre nou- velle fonction en l’appellant depuis la fonction main
multiboot2 située dans le fichier kernel/main.c juste avant le chargement des
tâches utilisateur. Pour cela vous pouvez utiliser la fonction uint64_t store
cr3(void) qui retourne le contenu du registre CR3.

* movq %rax, %rbx :
		moving a quadword from %rax to %rbx results in the instruction

* movl <source>, <dest>.......transfert
		Met le contenu de <source> dans <dest>. Équivalent C:
		<dest>=<source>.  Ex: movl %esp, %ebp sauvegarde le pointeur de
		pile %esp dans le registre %ebp.  Ex: movl %eax, 12(%ebp)
		stocke le contenu de %eax dans les quatre octets commençant à
		%ebp+12.


## Exercice 2

Dans cet exercice on implémente la fonction qui permet de mapper une adresse
virtuelle sur une adresse physique. Par soucis de simplicité, on fera les
suppositions suivantes :
— Les niveaux intermédiaires de la table des pages
sont tous mappés par une identité (l’adresse virtuelle est égale à l’adresse
physique).
— Tous les nouveaux mappings seront faits avec les droits
utilisateurs (bit 2) et en écriture (bit 1).
— On ne cherche pas à gérer les huge pages.
— Si un mapping est demandé pour une adresse virtuelle α,
l’adresse virtuelle α n’est pas déjà mappée.  Vous avez à votre disposition la
fonction paddr t alloc page() qui alloue une nouvelle page déjà mappée par une
identité. Le contenu de cette page est indéfini.

### Question 1
Étant donné une adresse virtuelle vaddr à mapper et la hauteur courante dans la
table des pages lvl (avec lvl = 4 pour le niveau indiqué dans le CR3), donnez
la formule qui indique l’index de l’entrée correspondante dans le niveau
courant.

```c

#define INDEX(vaddr, lvl)    (((((vaddr<<16)>>28)>>(lvl-1))<<54)>>54)

```
### Qestion 2
(voir code)

### Question 3
il faut faire xp/8g @physique pour avoir la reponse.

## Exercice 3

### Question 1
A cette étape du TP, l’exécution de Rackdoll doit afficher sur le moniteur
qu’une faute de page se produit à l’adresse virtuelle 0x2000000030. Étant donné
le modèle mémoire, indiquez ce qui provoque la faute de page.

```  c

//+----------------------+ 0x00007fffffffffff	140To	= 140 737 488 355 327 octets
//| User                 |
//| (text + data + heap) | ERREUR		0x20000000030	= 2 199 023 255 600 octets
//+----------------------+ 0x2000000000	2To=2x2^40	= 2 199 023 255 552 octets

```

### Question 2
La première étape de la création d’une nouvelle tâche en mémoire est de dériver
la table des pages courante en une nouvelle table des pages. Expliquez quelles
plages d’adresses de la table courante doivent être conservées dans la nouvelle
table et pourquoi.  Une tâche utilisateur ctx est constituée de deux parties :
— Le payload situé dans la mémoire physique entre ctx->load paddr et ctx->load
end paddr qui contient le code et les données.  — Le bss qui doit commencer en
espace virtuel immédiatement après le payload et s’arrêter à l’adresse
virtuelle ctx->bss end vaddr.  On rappelle que le bss est une zone qui doit
être initialisée à zero au lancement d’un tâche. Il est possible que certaines
tâches aient un bss vide.


Il faut conserver les adresses entre 0x0 et 0x40000000 (1Go=2^30) qui
contiennent le code du kernel. Qui doit rester accessible meme si on change de
contexte.
https://unix.stackexchange.com/questions/72680/how-does-linux-update-page-table-after-context-switch




### Question 3
Donnez les adresses virtuelles de début et de fin du payload et du bss d’une
tâche, calculées en fonction du modèle mémoire et des champs d’une tâche ctx.

* Task-State Segment (TSS)—A segment that holds the processor state associated
* with a task.  TSS Descriptor—A segment descriptor that defines the task-state
* segment.  TSS Selector—A segment selector that references the TSS descriptor
* located in the GDT.  Task  Register—A register that holds the TSS selector
* and TSS descriptor for the current task.

* VMA : virtual memory area

``` c

//-----------------------------------------------------------------------------
task.ld
    TASK_VMA = 0x2000000000;
    ...
      .bss : ALIGN(0x1000) {
    ...

//-----------------------------------------------------------------------------
task/sieve.c
    vaddr_t heap = (vaddr_t) &__bss_end;


//-----------------------------------------------------------------------------
kernel/entry.S
    .section ".bss"
    .space  0x1000, 0     # initial stack of 4 KiB

//-----------------------------------------------------------------------------

struct task
{
	//...
	paddr_t load_paddr;    /* paddr of the task code =>0x00007fffffffffff */
	paddr_t load_end_paddr;/* paddr following code  => 0x2000000000 */
	vaddr_t load_vaddr;    /* vaddr for load_paddr */
	vaddr_t bss_end_vaddr; /* vaddr following bss */
	//...
};
*-----------------------------------------------------------------------------
/*                           Task memory layout FROM SMAIL
 *
 *                       adressage                     adressage
 *                         virt                           Phy
 *                +----------------------+
 *                | HEAP                 |
 *                |                      |
 *bss_end_vaddr-> +----------------------+     +----------------------+ <--- load_end_paddr
 *                | BSS                  |     |     code & data      |
 *                |                      |     |       PAYLOAD        |
 *                +----------------------+     +----------------------+ <--- load_paddr
 *                |      code & data     |     |                      |
 *                |        PAYLOAD       |     |                      |
 * load_vaddr ->  +----------------------+     +----------------------+
 *
 *-----------------------------------------------------------------------------
 * Memory model for Rackdoll OS
 *

 * +----------------------+++0xffffffffffffffff
 * | Higher half          |
 * | (unused)             |
 * +----------------------+++0xffff800000000000	16Pico		= 18 446 603 336 221 196 288
 * | (impossible address) |
 * +----------------------+++0x00007fffffffffff	128To		=        140 737 488 355 327
 * | User            .:.  | // Between 128 GiB and 128 TiB is the heap addresses for user procs
 * | (text+data+bss+heap) | // ERREUR		0x20000000030	=          2 199 023 255 600
 * +----------------------+++0x2000000000	128Go		=            137 438 953 472
 * | User      |          | // Between 1 GiB and 128 GiB is the stack
 * | (stack)   v          | // addresses for user processes growing down from 128 GiB.
 * +----------------------+++0x40000000		1Go=2^30	=              1 073 741 824
 * | Kernel               | // Between 2 MiB and 1 GiB, there are kernel
 * | (valloc)             | // addresses which are not mapped with an identity table.
 * +----------------------+++0x201000				=                  2 101 248
 * | Kernel               | // Between 2 MiB and 1 GiB, there are kernel
 * | (APIC)               | // addresses which are not mapped with an identity table.
 * +----------------------+++0x200000		2(Mo)=2*2^20	=                  2 097 152
 * | Kernel               | // The first 2 MiB are identity mapped and not cached.
 * | (text + data)        |
 * +----------------------+++0x100000		1(Mo)=2^20	=                  1 048 576
 * | Kernel               | // The first 2 MiB are identity mapped and not cached.
 * | (BIOS + VGA)         |
 * +----------------------+ 0x0
 */


adversary.elf:     file format elf64-x86-64
// Dans le OBJdump on peut voir la section text a 200000000030

Contents of section .header:
 2000000000 ffdec04b a6ad10ff 00000000 20000000  ...K........ ...
Contents of section .text:
 2000000030 49bb3801 00000000 00005348 8d1deeff  I.8.......SH....
 ...
Contents of section .data:
 2000000100 20203d3d 3e204164 76657273 61727920    ==> Adversary
 ...
Contents of section .got.plt:
 2000000168 00000000 00000000 00000000 00000000  ................
 ...



```

* size  = load_end_paddr - load_paddr
* Début payload: load_vaddr
* Fin payload: load_vaddr + size
* bss_start_vaddr = load_vaddr + size.


### Question 4
Implémentez la fonction void load task(struct task *ctx) qui initialise une
nouvelle tâche en mémoire sans toutefois charger sa table des pages dans le
CR3.
OK


### Question 5
Implémentez la fonction void set task(struct task *ctx) qui charge une nouvelle
tâche en mémoire en modifiant le CR3.
OK

### Question 1
Implémentez la fonction void mmap(struct task *ctx, vaddr t vaddr) qui alloue
une page phy- sique, l’initialise à zero et la mappe à l’adresse virtuelle
donnée pour la tâche donnée.
OK

### Question 2
Á cette étape du TP, l’exécution de Rackdoll doit afficher sur le moniteur
qu’une faute de page se produit à l’adresse virtuelle 0x1ffffffff8. Étant donné
le modèle mémoire, indiquez ce qui provoque la faute de page. D’après vous,
cette faute est-elle causée par un accès mémoire légitime ?
C'est une adresse de la pile.

### Question 3
D’après le modèle mémoire de Rackdoll, la pile d’une tâche utilisateur a une
taille de 127 GiB, c’est à dire bien plus que la mémoire physique disponible
dans la machine virtuelle. La pile est donc allouée de manière paresseuse.
Expliquez en quoi consiste l’allocation paresseuse.

"Sous Linux,l’allocation mémoire est paresseuseConséquence :malloc(2^ 40)
retourne une adresse utilisableLa mémoire ne sera allouée quand l’adresse
sera utilisée"

### Question 4
Implémentez la fonction void pgfault(struct interrupt context *ctx) qui traite
une faute de page dont le contexte est stocké dans ctx et où l’adresse qui a
causé la faute est stockée dans le registre CR2 accessible via la fonction
uint64 t store cr2(void). Rappellez-vous que les seules fautes de page
légitimes sont celles de la pile. Toute faute à une adresse en dehors de la
pile doit causer une faute de segmentation de la tâche courante (vous pouvez
utiliser la fonction void exit task(struct interrupt context *ctx) qui termine
la tâche courante).
OK

## Exercice 5
Dans cet exercice on implémente la fonction de libération. Á cette étape du TP,
les tâches doivent s’exécuter brièvement avant d’échouer. Des messages
d’avertissement indiquant une pénurie mémoire doivent aussi s’afficher sur le
moniteur. Cette pénurie est causée par la tâche “Sieve” qui fait de nombreux
appels système mmap et munmap. Puisque munmap n’est pas encore implémenté,
aucune page n’est libérée : c’est une fuite mémoire.

### Question 1
Implémentez la fonction void munmap(struct task *ctx, vaddr t vaddr) qui permet
de supprimer le mapping d’une adresse virtuelle donnée pour une tâche donnée.
Cette fonction doit aussi libérer les pages mémoire qui ne sont plus utilisées
à l’aide de la fonction void free page(paddr t addr).  Une fois la fonction
munmap implémentée, la tâche Sieve ne devrait plus causer de pénurie mémoire et
toutes les tâches devraient pouvoir s’exécuter complètement.
ok (current()).

### Question 2
Il est possible que malgré une exécution complète, la tâche “Adversary” indique
un echec. En lisant le code de cette tâche dans task/adversary.c et en relisant
le code de votre fonction munmap, indiquez ce qui peut provoquer cet echec.
censee faire une faute de segmentation?

### Question 3
Corrigez le problème soulevé dans la ### Question 2 à l’aide d’une fonction définie
dans le fichier include/x86.h.

Control Register : (https://en.wikipedia.org/wiki/Control_register)

FLAGS(https://www.shsu.edu/~csc_tjm/fall2003/cs272/flags.html)
* RFLAGS : (https://fr.wikipedia.org/wiki/RFLAGS)
-    TF (bit 8) Trap Flag (Drapeau de trappe) : Lorsqu'il est armé, ce drapeau
    permet le débogage en mode pas à pas, c'est-à-dire instruction par
    instruction. Lorsqu'il est désarmé, le mode pas à pas est inopérant
    (fonctionnement normal).

-    IF (bit 9) Interrupt Flag (Drapeau d'interruption) : Ce drapeau contrôle
    la façon dont le processeur répond aux requêtes d'interruptions masquables
    (c'est-à-dire désactivables). Lorsqu'il est armé, le processeur peut
    répondre à toutes les interruptions, dans le cas contraire (drapeau IF
    désarmé), le processeur ne pourra répondre qu'aux interruptions non
    masquables.

-    IOPL (bits 12 et 13) Input / Output privilege level field (Champ de niveau
    de privilège d'entrée et de sortie) : Ce champ indique le niveau de
    privilège en entrée/sortie (E/S) du programme ou de la tâche courante. Le
    niveau de privilège courant du programme ou de la tâche en cours doit être
    égal ou inférieur au niveau de privilège d'E/S pour accéder à l'espace
    d'adressage. Ce champ ne peut être modifié qu'avec un niveau de privilège
    égal à 0 (niveau de privilège le plus haut). Ce concept de niveaux de
    privilèges est implémenté au travers des anneaux de protection.

-    NT (bit 14) Nested task Flag (Drapeau de tâche chaînée) : Ce drapeau
    contrôle l'enchaînement des tâches interrompues et appelées. Il indique
    ainsi, lorsqu'il est armé, si la tâche courante est liée à une tâche
    parent (la tâche qui s'exécutait auparavant) via l'instruction CALL ou par
    le biais d'une interruption. Lorsqu'il est désarmé, ce drapeau indique
    simplement que la tâche courante n'a pas de tâche parente.

-    RF (bit 16) Resume Flag (Drapeau de redémarrage) : Ce drapeau contrôle la
    réponse du processeur aux exceptions de débogage. Il assure notamment que
    le débogage en pas à pas (voir drapeau TF) n'intervient qu'une seule fois
    par instruction.

-    VM (bit 17) Virtual-8086 mode Flag (Drapeau de mode virtuel 8086) :
    Lorsque ce drapeau est armé le processeur est en mode virtuel 8086.
    Lorsqu'il est désarmé, le processeur revient en mode protégé.

-    AC (bit 18) Alignment Check Flag (Drapeau de vérification d'alignement) :
    Ce drapeau, lorsqu'il est armé, assure une vérification d'alignement des
    références mémoire. Lorsqu'il est désarmé, aucune vérification
    d'alignement n'est effectuée. Ce drapeau nécessite d'armer conjointement
    le bit AM du registre de contrôle CR0.

-    VIF (bit 19) Virtual Interrupt Flag (Drapeau d'interruption virtuelle) :
    Ce drapeau est une image virtuelle du drapeau IF. Il est utilisé en
    conjonction avec le drapeau VIP (bit 20).

-    VIP (bit 20) Virtual Interrupt Pending Flag (Drapeau d'interruption
    virtuelle en attente) : Lorsqu'il est armé ce drapeau indique qu'une
    interruption est en attente. Lorsqu'il est désarmé ce drapeau indique
    qu'aucune interruption n'est en attente. Seuls les programmes peuvent
    armer ou désarmer ce drapeau, le processeur ne fait que le lire. À
    utiliser conjointement avec le drapeau VIF (bit 19).

-    ID (bit 21) Identification Flag (Drapeau d'indentification) : Si un
    programme a la possibilité d'armer ou de désarmer ce drapeau, cela indique
    que le processeur supporte l'utilisation de l'instruction CPUID.

``` c

objdump -s :

adversary.elf:     file format elf64-x86-64

Contents of section .header:
 2000000000 ffdec04b a6ad10ff 00000000 20000000  ...K........ ...
 2000000010 00100000 20000000 00300000 20000000  .... ....0.. ...
 2000000020 00000000 20000000 30000000 20000000  .... ...0... ...
Contents of section .text:
 2000000030 49bb3801 00000000 00005348 8d1deeff  I.8.......SH....
 2000000040 ffff31ff 48b898ff ffffffff ffff4c01  ..1.H.........L.
 2000000050 db488d34 03cd8048 b80030ff ff1f0000  .H.4...H..0.....
 2000000060 0048ba00 40ffff1f 0000000f 1f440000  .H..@........D..
 2000000070 88004883 c0014839 d075f548 be0030ff  ..H...H9.u.H..0.
 2000000080 ff1f0000 00bf0300 0000cd80 48b8b0ff  ............H...
 2000000090 ffffffff ffff4889 f24531c0 41b90500  ......H..E1.A...
 20000000a0 000048b9 0040ffff 1f000000 4c8d1403  ..H..@......L...
 20000000b0 803a0074 104c89c7 4c89d6cd 804c89cf  .:.t.L..L....L..
 20000000c0 4c89c6cd 804883c2 014839ca 75e231d2  L....H...H9.u.1.
 20000000d0 bf040000 004889d6 cd8048b8 d8ffffff  .....H....H.....
 20000000e0 ffffffff 4889d748 8d3403cd 80bf0500  ....H..H.4......
 20000000f0 00004889 d6cd805b c3                 ..H....[.
Contents of section .data:
 2000000100 20203d3d 3e204164 76657273 61727920    ==> Adversary
 2000000110 5461736b 0a000000 20202d2d 3e204164  Task....  --> Ad
 2000000120 76657273 61727920 72657375 6c743a20  versary result:
 2000000130 6661696c 7572650a 00000000 00000000  failure.........
 2000000140 20202d2d 3e204164 76657273 61727920    --> Adversary
 2000000150 72657375 6c743a20 73756363 6573730a  result: success.
 2000000160 00000000 00000000                    ........
Contents of section .got.plt:
 2000000168 00000000 00000000 00000000 00000000  ................
 2000000178 00000000 00000000                    ........
```
Avez-vous compris pourquoi la tâche Adversary ne fonctionne pas ?

En théorie, vous devriez observer que Adversary échoue car elle lit une
donnée présente sur une page qui s'est fait démappée, ce qui devrait
être impossible.

Ce problème est lié au TLB.



