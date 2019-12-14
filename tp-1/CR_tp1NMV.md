# CR TP MMU

## Commandes utile
info registers Affiche l’état les principaux registres de la machine.
xp/8g addr Affiche le contenu de la mémoire à l’adresse virtuelle addr. 

## Definitions :
- adresse physique :

- adresse physique d’un niveau de la table des pages : un niveau intermédiaire de la table des pages est mappé
par une identité (il est accessible par une adresse virtuelle égale à son adresse physique)

- hauteur d'un niveau :


## Exercice 1
Ce premier exercice vous permet de vous familiariser avec la structure d’une table des pages. On vous
propose d’implémenter une fonction qui affiche la structure de la table des pages courante. Cette fonction a
le prototype : void print pgt(paddr t pml, uint8 t lvl). Cette fonction recursive prend comme
paramètre l’adresse physique d’un niveau de la table des pages et la hauteur de ce niveau.


### Question 1
Dans le cadre de ce TP on supposera toujours qu’un niveau intermédiaire de la table des pages est mappé
par une identité (il est accessible par une adresse virtuelle égale à son adresse physique). Pourquoi cette
supposition est-elle importante ?
- Cette supposition permet de ne pas avoir a acceder a la mmu pour traduire une adresse qui sert
a acceder a la table de la mmu (beaucoup plus difficile)


### Question 2
Combien y a-t-il d’entrée par niveau de table des pages pour une architecture x86 64 bits ? Comment la
MMU détermine-t-elle si une entrée d’un niveau de table des pages est valide ? Si elle est terminale ?

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
Dans kernel/main.c, programmez et testez la nouvelle fonction print pgt. Vous testerez votre nou-
velle fonction en l’appellant depuis la fonction main multiboot2 située dans le fichier kernel/main.c
juste avant le chargement des tâches utilisateur. Pour cela vous pouvez utiliser la fonction uint64_t
store cr3(void) qui retourne le contenu du registre CR3.

* movq %rax, %rbx :
		moving a quadword from %rax to %rbx results in the instruction

* movl <source>, <dest>.......transfert
		Met le contenu de <source> dans <dest>. Équivalent C: <dest>=<source>.
		Ex: movl %esp, %ebp sauvegarde le pointeur de pile %esp dans le registre %ebp.
		Ex: movl %eax, 12(%ebp) stocke le contenu de %eax dans les quatre octets commençant à %ebp+12.


## Exercice 2

Dans cet exercice on implémente la fonction qui permet de mapper une adresse virtuelle sur une adresse
physique. Par soucis de simplicité, on fera les suppositions suivantes :
— Les niveaux intermédiaires de la table des pages sont tous mappés par une identité (l’adresse virtuelle
est égale à l’adresse physique).
— Tous les nouveaux mappings seront faits avec les droits utilisateurs (bit 2) et en écriture (bit 1).
— On ne cherche pas à gérer les huge pages.
— Si un mapping est demandé pour une adresse virtuelle α, l’adresse virtuelle α n’est pas déjà mappée.
Vous avez à votre disposition la fonction paddr t alloc page() qui alloue une nouvelle page déjà mappée
par une identité. Le contenu de cette page est indéfini.

###Question 1
Étant donné une adresse virtuelle vaddr à mapper et la hauteur courante dans la table des pages lvl
(avec lvl = 4 pour le niveau indiqué dans le CR3), donnez la formule qui indique l’index de l’entrée
correspondante dans le niveau courant.

```c

#define INDEX(vaddr, lvl)    (((((vaddr<<16)>>28)>>(lvl-1))<<54)>>54)

```
