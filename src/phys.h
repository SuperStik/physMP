#ifndef PHYS_H
#define PHYS_H 1

#include <pthread.h>
#include <cppheader.h>

C_BEGIN;

pthread_t phys_begin(void);
void phys_end(pthread_t);

C_END;

#endif /* PHYS_H */
