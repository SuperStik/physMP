#ifndef SHADER_BLINNPHONG
#define SHADER_BLINNPHONG 1

#include <dispatch/dispatch.h>
#include <objc/objc.h>

void shdr_blinnphong_new(id *state, dispatch_group_t, id device, id library,
		void *pipeline_descriptor, void *vertex_descriptor);

#endif /* SHADER_BLINNPHONG */
