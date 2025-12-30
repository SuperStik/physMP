#include <assert.h>

#import <Metal/Metal.h>

#include "objc_macros.h"
#include "shaders.h"

struct shaders *shdr_load(struct shaders *shdr, id device) {
	id<MTLLibrary> lib = [device newDefaultLibrary];
	assert(lib != nil);

	id<MTLFunction> vertunlit = [lib newFunctionWithName:@"vertUnlit"];
	id<MTLFunction> fragunlit = [lib newFunctionWithName:@"fragUnlit"];

	id<MTLFunction> vertblinnphong = [lib
		newFunctionWithName:@"vertBlinnPhong"];
	id<MTLFunction> fragblinnphong = [lib
		newFunctionWithName:@"fragBlinnPhong"];
	
	[lib release];

	ARP_PUSH();
	MTLPipelineBufferDescriptorArray *bufs;
	MTLVertexAttributeDescriptorArray *attrs;
	MTLVertexAttributeDescriptor *attr;

	/* Blinn-Phong Pipeline */
	MTLRenderPipelineDescriptor *desc = [MTLRenderPipelineDescriptor new];
	desc.label = @"pipeline.blinnphong";
	desc.vertexFunction = vertblinnphong;
	[vertblinnphong release];
	desc.fragmentFunction = fragblinnphong;
	[fragblinnphong release];
	desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
	desc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

	bufs = desc.vertexBuffers;
	bufs[0].mutability = MTLMutabilityImmutable;
	bufs[1].mutability = MTLMutabilityImmutable;
	bufs[15].mutability = MTLMutabilityImmutable;

	desc.fragmentBuffers[0].mutability = MTLMutabilityImmutable;

	MTLVertexDescriptor *vertdesc = [MTLVertexDescriptor vertexDescriptor];

	attrs = vertdesc.attributes;

	attr = attrs[0];
	attr.format = MTLVertexFormatFloat3;
	attr.offset = offsetof(struct object_vertdata, pos);
	attr.bufferIndex = 15;

	attr = attrs[1];
	attr.format = MTLVertexFormatHalf3;
	attr.offset = offsetof(struct object_vertdata, normal);
	attr.bufferIndex = 15;

	vertdesc.layouts[15].stride = sizeof(struct object_vertdata);

	desc.vertexDescriptor = vertdesc;

	shdr->blinnphong = [device newRenderPipelineStateWithDescriptor:desc
								  error:nil];

	/* Unlit Pipeline */
	[desc reset];
	desc.label = @"pipeline.unlit";
	desc.vertexFunction = vertunlit;
	[vertunlit release];
	desc.fragmentFunction = fragunlit;
	[fragunlit release];
	desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
	desc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

	bufs = desc.vertexBuffers;
	bufs[0].mutability = MTLMutabilityImmutable;
	bufs[15].mutability = MTLMutabilityImmutable;

	[vertdesc reset];

	attr = vertdesc.attributes[0];
	attr.format = MTLVertexFormatFloat3;
	attr.offset = 0;
	attr.bufferIndex = 15;

	vertdesc.layouts[15].stride = sizeof(float) * 3;

	desc.vertexDescriptor = vertdesc;

	shdr->unlit = [device newRenderPipelineStateWithDescriptor:desc
							     error:nil];

	[desc release];
	ARP_POP();

	return shdr;
}

void shdr_release(struct shaders *shdr) {
	[shdr->blinnphong release];
	[shdr->unlit release];
}
