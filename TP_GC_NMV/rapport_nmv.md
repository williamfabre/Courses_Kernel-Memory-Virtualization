Rapport de NMV pour le tp GC

# EXERCICE 1

``` c
n |= 1 << x			// set bit
n &= ~(1 <<x)		// reset bit
n ^= 1 << x			// inverser le bit
if ( n & (1 << x))		// est-ce aue le bit vaut 1
```

## static :
https://stackoverflow.com/questions/572547/what-does-static-mean-in-c
###Variable locale
Utiliser une variable dans le scope d'une fonction qui entre les invocations
a besoin de preserver sa valeur. Permet de ne pas declarer une variable
globale. La fonction est non thread safe.
###Pour une fonction
son scope restera dans le fichier sert de controle d'acces, pour
encapsuler
ATTENTION : static ne peut etre utiliser dans une structure, il faut allouer et
desallouer une structure de maniere contigue, DONC il est impossible de mettre
une morceau de la structure en static.

##extern : valeur de base des fonctions (accessible a l'include)
https://www.tutorialspoint.com/extern-keyword-in-c
fonctions / variables definies en dehors de ce scope mais qui sont
globales.
Etendre la visibilite d'une variable. declare sans definir une variable.
declarer cree une reference d'allocation c'est different d'une allocation, qui
correspond a la definition d'une variable. Si c'est un objet simple on alloue
en meme temps que la declaration.
La definition dans les compilateurs se fera au linkage.

##inline : (MACRO non deterministe)
https://www.geeksforgeeks.org/inline-function-in-c/
permet a une fonction d'etre substituee au choix du compilateur (l'appel de
fonction n'existera pas post compilation).
Si on fait appel a une fonction inline dans une autre fonction, alors ce code
aura un probleme de linkage.
a la difference de #define, possede un type, peut optimiser des branchements a
la compilation, confort de code non negligeable.
les define sont de maniere deterministe dans le code.
__attribute__(always_inline) -> inline forcement un attribut dans une fonction.

##reeantrant :
https://en.wikipedia.org/wiki/Reentrancy_(computing)
Se dit d'un programme qui est thread safe (peut etre lancer plusieurs fois)
de maniere concurrente

##ftrace / strace

##__thread :
###Thread local storage : Methode de programmation qui utilise les static et la
https://en.wikipedia.org/wiki/Thread-local_storage
memoire globale locale a des threads.
Ressemble a une variable globale mais utilisable par chaque thread. Eviter
les mutex pour une variable globale partagee (isolation).
se declare avec __thread
exemplce utiliser errno pour le retour d'erreur d'un thread par thread.

##__builtin_frame_adress(unsigned int level) : (voir pour la valeur 0)
https://gcc.gnu.org/onlinedocs/gcc/Return-Address.html
###Renvoie l'adresse de retour de la fonction ou une de ses adresses appelantes.
level :
nombre de frames a scan dans la pile des appels. si on utilise 0 alors
ce sera l'adresse de retour de la fonction si c'est 1 alors ce sera
l'adresse de la fonction appelante etc. ( sert a remonter la stack des
fonctions appelantes)
ATTENTION il faut utiliser noiline comme attribut de fonction pour que
###le compilateur n'inline pas les fonctions.
Parfois on ne peut determiner (a cause de l'architecture) ou quand on a
atteint le bout de la stack alors la fonction renvoie 0 ou une valeur
random. (peut etre utiliser pour savoir si on a fini la stack).

utiliser -Wframe-address pour diagnostiquer un probleme avec cette
fonction qui a des effets non predictibles. (only debug).


#EXERCICE 2


## gcc -M :
output une regle et pas le resultat du pre-processing.

## 1ULL :
1 Unsigned Long Long

## char* avoir void*
https://www.cs.auckland.ac.nz/references/unix/digital/AQTLTBTE/DOCU_045.HTM
les char seront opimise a la compilation et pas les void* (en tout cas tq ils
ne sont pas cast).
mais c'est pas bien c'est pas portable, qui nous dit que char sera exactement
un pointeur.
A void pointer is a pointer without a specified data type to describe the object
to which it points. In effect, it is a generic pointer. (Before the ANSI C
standard, char * was used to define generic pointers; this practice is now
discouraged by the ANSI standard because it is less portable.)



































