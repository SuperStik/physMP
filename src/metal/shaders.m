#include <assert.h>

#import <Metal/Metal.h>

#include "shaders.h"

struct shaders *shdr_load(struct shaders *shdr, id device) {
	id<MTLLibrary> lib = [device newDefaultLibrary];
	assert(lib != nil);

	id<MTLFunction> vertlevel = [lib newFunctionWithName:@"vertLevel"];
	id<MTLFunction> fraglevel = [lib newFunctionWithName:@"fragLevel"];

	id<MTLFunction> vertobject = [lib newFunctionWithName:@"vertObject"];
	id<MTLFunction> fragobject = [lib newFunctionWithName:@"fragObject"];
	
	[lib release];

	MTLRenderPipelineDescriptor *desc = [MTLRenderPipelineDescriptor new];
	desc.label = @"pipeline.level";
	desc.vertexFunction = vertlevel;
	[vertlevel release];
	desc.fragmentFunction = fraglevel;
	[fraglevel release];
	desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

	shdr->level = [device newRenderPipelineStateWithDescriptor:desc
							     error:nil];

	[desc reset];
	desc.label = @"pipeline.object";
	desc.vertexFunction = vertobject;
	[vertobject release];
	desc.fragmentFunction = fragobject;
	[fragobject release];
	desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

	shdr->object = [device newRenderPipelineStateWithDescriptor:desc
							      error:nil];

	[desc release];

	return shdr;
}

void shdr_release(struct shaders *shdr) {
	[shdr->level release];
	[shdr->object release];
}
