#ifndef _ALLOC_H_
#define _ALLOC_H_

struct object_header {
    unsigned int object_size; // gives the exact objet size
    // A FAIRE : EXO 1 - Q1
};

struct object_header *pre_malloc(unsigned int size);
void                  pre_free(void *ptr);
struct object_header *toHeader(void *ptr);
void *                toObject(struct object_header *header);
void                  print_stats();
void init_bank_desc();

#endif
