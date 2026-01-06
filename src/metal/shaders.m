#include <assert.h>

#import <Metal/Metal.h>

#include "shaders.h"
#include "shaders/blinnphong.h"
#include "shaders/screen.h"
#include "shaders/unlit.h"

struct shaders *shdr_load(struct shaders *shdr, id device) {
	dispatch_group_t group = dispatch_group_create();

	@autoreleasepool {
		MTLRenderPipelineDescriptor *desc = [
			MTLRenderPipelineDescriptor new];
		MTLVertexDescriptor *vertdesc = [MTLVertexDescriptor
			vertexDescriptor];

		id<MTLLibrary> lib = [device newDefaultLibrary];
		assert(lib != nil);

		shdr_blinnphong_new(&shdr->blinnphong, group, device, lib, desc,
				vertdesc);

		[desc reset];
		[vertdesc reset];

		shdr_screen_new(&shdr->screen, group, device, lib, desc,
				vertdesc);

		[desc reset];
		[vertdesc reset];

		shdr_unlit_new(&(shdr->unlit), group, device, lib, desc,
				vertdesc);

		[desc release];

		[lib release];
	}

	dispatch_group_wait(group, DISPATCH_TIME_FOREVER);
	dispatch_release(group);

	return shdr;
}

void shdr_release(struct shaders *shdr) {
	[shdr->blinnphong release];
	[shdr->screen release];
	[shdr->unlit release];
}
