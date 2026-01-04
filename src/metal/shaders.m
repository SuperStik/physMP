#include <assert.h>

#import <Metal/Metal.h>

#include "shaders.h"
#include "shaders/blinnphong.h"
#include "shaders/screen.h"
#include "shaders/unlit.h"

struct shaders *shdr_load(struct shaders *shdr, id device) {
	id<MTLLibrary> lib = [device newDefaultLibrary];
	assert(lib != nil);

	@autoreleasepool {
		MTLRenderPipelineDescriptor *desc = [
			MTLRenderPipelineDescriptor new];
		MTLVertexDescriptor *vertdesc = [MTLVertexDescriptor
			vertexDescriptor];

		shdr->blinnphong = shdr_blinnphong_new(device, lib, desc,
				vertdesc);

		[desc reset];
		[vertdesc reset];

		shdr->screen = shdr_screen_new(device, lib, desc, vertdesc);

		[desc reset];
		[vertdesc reset];

		shdr->unlit = shdr_unlit_new(device, lib, desc, vertdesc);

		[desc release];
	}

	[lib release];

	return shdr;
}

void shdr_release(struct shaders *shdr) {
	[shdr->blinnphong release];
	[shdr->screen release];
	[shdr->unlit release];
}
