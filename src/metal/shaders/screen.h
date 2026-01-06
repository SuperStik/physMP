#ifndef SHDR_SCREEN_H
#define SHDR_SCREEN_H 1

#include <dispatch/dispatch.h>
#include <objc/objc.h>

void shdr_screen_new(id *state, dispatch_group_t, id device, id library, void *
		pipeline_descriptor, void *vertex_descriptor);

#endif /* SHDR_SCREEN_H */
