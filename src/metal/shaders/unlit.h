#ifndef SHADER_UNLIT
#define SHADER_UNLIT 1

#include <dispatch/dispatch.h>
#include <objc/objc.h>

void shdr_unlit_new(id *state, dispatch_group_t, id device, id library, void *
		pipeline_descriptor, void *vertex_descriptor);

#endif /* SHADER_UNLIT */
