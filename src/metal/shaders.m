#include <assert.h>

#import <Metal/Metal.h>

#include "objc_macros.h"
#include "shaders.h"
#include "shaders/blinnphong.h"
#include "shaders/unlit.h"

struct shaders *shdr_load(struct shaders *shdr, id device) {
	id<MTLLibrary> lib = [device newDefaultLibrary];
	assert(lib != nil);

	ARP_PUSH();
	MTLRenderPipelineDescriptor *desc = [MTLRenderPipelineDescriptor new];
	MTLVertexDescriptor *vertdesc = [MTLVertexDescriptor vertexDescriptor];

	shdr->blinnphong = shdr_blinnphong_new(device, lib, desc, vertdesc);

	[desc reset];
	[vertdesc reset];

	shdr->unlit = shdr_unlit_new(device, lib, desc, vertdesc);

	[desc release];
	ARP_POP();

	[lib release];

	return shdr;
}

void shdr_release(struct shaders *shdr) {
	[shdr->blinnphong release];
	[shdr->unlit release];
}
