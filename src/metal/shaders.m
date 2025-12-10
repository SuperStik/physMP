#include <assert.h>

#import <Metal/Metal.h>

#include "objc_macros.h"
#include "shaders.h"

struct shaders *shdr_load(struct shaders *shdr, id device) {
	id<MTLLibrary> lib = [device newDefaultLibrary];
	assert(lib != nil);

	id<MTLFunction> vertlevel = [lib newFunctionWithName:@"vertLevel"];
	id<MTLFunction> fraglevel = [lib newFunctionWithName:@"fragLevel"];

	id<MTLFunction> vertobject = [lib newFunctionWithName:@"vertObject"];
	id<MTLFunction> fragobject = [lib newFunctionWithName:@"fragObject"];
	
	[lib release];

	ARP_PUSH();
	MTLPipelineBufferDescriptorArray *bufs;
	MTLVertexAttributeDescriptorArray *attrs;
	MTLVertexAttributeDescriptor *attr;

	/* Level Pipeline */
	MTLRenderPipelineDescriptor *desc = [MTLRenderPipelineDescriptor new];
	desc.label = @"pipeline.level";
	desc.vertexFunction = vertlevel;
	[vertlevel release];
	desc.fragmentFunction = fraglevel;
	[fraglevel release];
	desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
	desc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

	bufs = desc.vertexBuffers;
	bufs[0].mutability = MTLMutabilityImmutable;
	bufs[15].mutability = MTLMutabilityImmutable;

	MTLVertexDescriptor *vertdesc = [MTLVertexDescriptor vertexDescriptor];

	attr = vertdesc.attributes[0];
	attr.format = MTLVertexFormatFloat3;
	attr.offset = 0;
	attr.bufferIndex = 15;

	vertdesc.layouts[15].stride = sizeof(float) * 3;

	desc.vertexDescriptor = vertdesc;

	shdr->level = [device newRenderPipelineStateWithDescriptor:desc
							     error:nil];

	/* Object Pipeline */
	[desc reset];
	desc.label = @"pipeline.object";
	desc.vertexFunction = vertobject;
	[vertobject release];
	desc.fragmentFunction = fragobject;
	[fragobject release];
	desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
	desc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

	bufs = desc.vertexBuffers;
	bufs[0].mutability = MTLMutabilityImmutable;
	bufs[1].mutability = MTLMutabilityImmutable;
	bufs[15].mutability = MTLMutabilityImmutable;

	desc.fragmentBuffers[0].mutability = MTLMutabilityImmutable;

	[vertdesc reset];

	attrs = vertdesc.attributes;

	attr = attrs[0];
	attr.format = MTLVertexFormatFloat3;
	attr.offset = offsetof(struct object_vertdata, pos);
	attr.bufferIndex = 15;

	attr = attrs[1];
	attr.format = MTLVertexFormatFloat3;
	attr.offset = offsetof(struct object_vertdata, normal);
	attr.bufferIndex = 15;

	vertdesc.layouts[15].stride = sizeof(struct object_vertdata);

	desc.vertexDescriptor = vertdesc;

	shdr->object = [device newRenderPipelineStateWithDescriptor:desc
							      error:nil];

	[desc release];
	ARP_POP();

	return shdr;
}

void shdr_release(struct shaders *shdr) {
	[shdr->level release];
	[shdr->object release];
}
