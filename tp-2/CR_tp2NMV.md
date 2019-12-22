# CR TP MEMOIRE


## Exercice 1

``` c

char *mem = allocate_and_touch_memory(MEMORY_SIZE);
start_timer();
for (i = 0; i < MEMORY_SIZE; i += PARAM)
mem[i] = 1;
stop_timer();

```

### Question 1
En vous rappellant du cours de la semaine précédente, et sachant qu’on cherche
à mesurer précisemment un temps d’exécution, quel est l’intérêt d’initialiser
la nouvelle zone mémoire ?

"alloue en mémoire le nombre d’octet indiqué et initialise la nouvelle zone à zéro"
L'allocation reelle d'une page n'est faite que la premiere fois que cette page
est atteinte, en effet sinon lors d'un mmap l'allocation d'un morceau de
memoire reste une operation "gratuite.
Pour etre sure que les pages sont bien allouees physiquement il faut atteindre
une premiere fois la page.
Desormais les temps d'acces ne compteront que les temps d'acces au cache.

### Question 2
La boucle décrite génère-t-elle des cache hit ? Des cache miss ?

Dependant de MEMORY_SIZE et de PARAM et i:

- Nous ne connaissons pas le type de i, ce type est important car un unique i
peut faire la taille d'un mot ou la taille d'une ligne de cache (1oct jusqu'a
8oct).  Le parametre i permettra d'avoir plus ou moins de donnees dans une meme
ligne de cache.
- PARAM va faire varier se deplacer l'index i, si PARAM est petit la boucle va profiter de la localite spatiale entre deux donnes i.
- MEMORY_SIZE est la taille de l'empreinte memoire du tableau.

dependant de ces

### Question 3
Comment varient les nombres de cache miss et de cache hit quand le paramètre
PARAM augmente. En particulier, comment varient-t-ils quand PARAM est très
inférieur, légèrement inférieur ou supérieur à la taille d’une ligne de cache ?
Quelle est approximativement la proportion de cache hit à ces trois étapes ?

### Question 4
D’après votre réponse à la ### Question 3, quel est le type d’accès déterminant
pour le temps d’exécution quand le paramètre PARAM est très inférieur,
légèrement inférieur ou supérieur à la taille d’une ligne de cache ?  Comment
devrait varier le temps d’exécuion mesuré à chacune de ces trois étapes ?


### Question 5
Le fichier cacheline.c contient un code similaire à celui fournit dans cet
exercice. La fonction uint64 t detect(char *mem) diffère néamoins sur deux
aspects : la boucle principale est exécutée plusieurs fois plutôt qu’une seule
et est précédée par un boucle de warmup. Expliquez l’intérêt de ces deux
modifications.

###Question 6
Exécutez la commande make cacheline. Elle a pour effet de compiler le fichier
cacheline.c avec plusieurs valeurs pour le paramètre PARAM, d’exécuter chacune
des versions et de représenter les temps d’exécution sous forme graphique. Ce
graphique indique en abscisee la valeur de PARAM et en ordonnée le temps
d’exécution moyen de la boucle principale en nanoseconde. Déduisez de ce
graphique la taille d’une ligne de cache sur votre machine.
