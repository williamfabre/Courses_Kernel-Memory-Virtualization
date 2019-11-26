#ifndef _GC_H_
#define _GC_H_

// Ce fichier contient l'interface de votre ramasse-miettes

// La fonction gcmalloc doit renvoyer un nouvel objet.
// Vous vous servirez de la fonction pre_malloc qui se trouve dans
// les fichiers alloc.h et alloc.c.
// Si vous commencez à manquer de place,
// vous devez déclancher une demande de collection ici.
// Vous devrez modifier l'entête des objets pour ajouter les champs que vous jugerez
// nécessaires (object_header qui se trouve dans alloc.h)
void *gcmalloc(unsigned int size);

// la fonction writeBarrier doit être appelée à chaque écriture.
// Vous devez vous assurez que l'invariant tri-couleurs de Steel est respecté.
// Pensez que plusieurs threads peuvent appeler writeBarrier en parallèle!
void  _writeBarrier(void *dst, void *src);
#define doWriteRef(dst, src) ({ _writeBarrier(&(dst), src); dst = src; })

// la fonction handShake est appelée régulièrement pas un mutateur. Si une collection
// est demandée suite à un manque de place en mémoire, cette fonction doit s'occuper
// d'ajouter les racines de la pile à une liste pour que le collecteur puisse commencer une collection.
// Cette fonction doit libérer le mutateur pendant une collection.
// Pour augmenter la concurrence, vous utiliserez une variable locale (thread local storage, TLS, indiquée par __thread)
// au thread pour stocker vos racines.
void  handShake();

// Cette fonction attache un nouveau thread.
// Elle permet d'initialiser le TLS du thread et
// d'enregistrer le nouveau thread auprès du mutateur.
void attach_thread();

// Cette fonction détache un thread.
void detach_thread();

// cette fonction est appelée à l'initialisation du programme.
// Vous devrez créer un thread collecteur ici.
// Ce thread attend qu'une collection soit demandée à l'aide d'une variable condition.
void initialise_gc();

#endif
