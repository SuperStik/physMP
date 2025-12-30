#import <Metal/Metal.h>

#include "../shaders.h"
#include "blinnphong.h"

id shdr_blinnphong_new(id d, id l, void *p, void *v) {
	id<MTLDevice> device = d;
	id<MTLLibrary> lib = l;
	MTLRenderPipelineDescriptor *desc = p;
	MTLVertexDescriptor *vertdesc = v;

	MTLPipelineBufferDescriptorArray *bufs;
	MTLVertexAttributeDescriptorArray *attrs;
	MTLVertexAttributeDescriptor *attr;

	id<MTLFunction> vertblinnphong = [lib
		newFunctionWithName:@"vertBlinnPhong"];
	id<MTLFunction> fragblinnphong = [lib
		newFunctionWithName:@"fragBlinnPhong"];

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

	return [device newRenderPipelineStateWithDescriptor:desc error:nil];
}
