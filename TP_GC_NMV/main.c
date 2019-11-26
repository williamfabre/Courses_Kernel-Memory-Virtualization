#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include "debug.h"
#include "gc.h"
#include "alloc.h"

//#define gcmalloc malloc

// modifiez ce nombre si les tests sont trop longs.
#define FACTOR 64
// 4  -> 4Mo   de mémoire allouée
// 8  -> 11Mo s de mémoire allouée
// 16 -> 47Mo  de mémoire allouée
// 32 -> 191Mo de mémoire allouée
// 64 -> 764Mo de mémoire allouée

struct string_buf {
    char *buf;
    int  cursor;
    int  top;
};

void string_buf_append(struct string_buf *buf, int val) {
    if((buf->cursor + 64) > buf->top) {
        if(buf->top) {
            char *tmp = (char *)gcmalloc(buf->top << 1);
            memcpy(tmp, buf->buf, buf->cursor);
            doWriteRef(buf->buf, tmp);
            buf->top <<= 1;
        } else {
            doWriteRef(buf->buf, (char *)gcmalloc(64));
            buf->top = 64;
        }
    }
    buf->cursor += sprintf(buf->buf + buf->cursor, "%d ", val);
    handShake();
}

struct hash_elmt {
    struct hash_elmt *next;
    int               value;
};

struct hashmap {
    struct hash_elmt **nodes;
    int                mod;
    int                nb_elment;
};

struct hashmap *new_hashmap() {
    struct hashmap *res = gcmalloc(sizeof(struct hashmap));
    res->mod = 3;
    doWriteRef(res->nodes, (struct hash_elmt **)gcmalloc(res->mod * sizeof(struct hash_elmt **)));
    return res;
}

typedef void (*hashmap_apply_t)(void *parm, struct hash_elmt *elmt, void *other);

void hashmap_apply(struct hashmap *hm, hashmap_apply_t func, void *parm, void *other) {
    // Ce parcours n'est absolument pas optimal, mais il présente l'avantage
    // de supprimer des noeuds pour les déplacer ailleurs
    void recurse_apply(struct hash_elmt **elmt) {
        if(*elmt) {
            recurse_apply(&(*elmt)->next);
            func(parm, *elmt, other);
        }
    }
    
    for(int i=0; i<hm->mod; i++)
        recurse_apply(&(hm)->nodes[i]);
}

char *hashmap_to_string(struct hashmap *hm) {
    struct string_buf *buf = (struct string_buf *)gcmalloc(sizeof(struct string_buf));
    void print_elmt(struct string_buf *buf, struct hash_elmt *elmt) {
        string_buf_append(buf, elmt->value);
    }
    
    hashmap_apply(hm, (hashmap_apply_t)print_elmt, buf, 0);
    return buf->buf;
}

int has_elmt(struct hashmap *hm, int value) {
    struct hash_elmt **cur = &hm->nodes[value % hm->mod];
    
    for(; *cur; cur=&(*cur)->next) {
        handShake();
        if((*cur)->value == value)
            return 1;
    }
    
    return 0;
}

int hash(struct hashmap *hm, struct hash_elmt *elmt, struct hashmap *helper)
{
    struct hash_elmt **cur = &hm->nodes[elmt->value % hm->mod];
    
    for(; *cur; cur=&(*cur)->next) {
        handShake();
        if((*cur)->value == elmt->value)
            return 1;
    }
    
    elmt->next = 0;
    doWriteRef(*cur, elmt);
    
    // cette partie là devrait tester l'incrémental
    if(++hm->nb_elment > hm->mod<<1) {
        //        printf("Rehash...: %d\n", elmt->value);
        if(helper == hm)
            error("ne devrait pas arriver");
        
        doWriteRef(helper->nodes, hm->nodes);
        helper->nb_elment = hm->nb_elment;
        helper->mod = hm->mod;
        
        hm->nb_elment = 0;
        hm->mod <<= 1;
        doWriteRef(hm->nodes, (struct hash_elmt **)gcmalloc(hm->mod * sizeof(struct hash_elmt **)));
        
        hashmap_apply(helper, (hashmap_apply_t)hash, hm, 0);
    }
    
    handShake();
    
    return 0;
}

void *compute(void *arg)
{
    uint64_t num = 1+(uint64_t)arg;
    uint64_t nb_round = FACTOR * num;
    uint64_t nb_elmt = (FACTOR*2048)/num;
    
    attach_thread(__builtin_frame_address(0));
    
    for(int k=0; k<nb_round;) {
        struct hashmap *helper = new_hashmap();
        struct hashmap *hm = new_hashmap();
        
        for(int i=0; i<nb_elmt; i++) {
            struct hash_elmt *elmt = (struct hash_elmt *)gcmalloc(sizeof(struct hash_elmt));
            elmt->value = i;
            hash(hm, elmt, helper);
        }
        
        // check
        for(int i=0; i<nb_elmt; i++)
            if(!has_elmt(hm, i)) {
                printf("%s\n", hashmap_to_string(hm));
                error("votre hashmap est corrompue: l'élement %d manque", i);
            }
        
        if(!(++k%(FACTOR/2)) && (k!=nb_round))
            printf("Thread %ld executé à %2.0f%%\n", num, 100.*(float)k/(float)nb_round);
    }
    
    detach_thread();
    
    return 0;
}

int main(int argc, char **argv)
{
    uint64_t nb_threads = 0;
    
    initialise_gc();
    
    srand(time(NULL));
    
    pthread_t tid[nb_threads];
    for(uint64_t i=0; i<nb_threads; i++)
        pthread_create(&tid[i], 0, compute, (void *)i);
    
    compute((void *)nb_threads);
    
    for(int i=0; i<nb_threads; i++)
        pthread_join(tid[i], NULL);
    
    print_stats();
    
    return 0;
}
