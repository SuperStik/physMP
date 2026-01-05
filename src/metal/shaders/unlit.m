#import <Metal/Metal.h>

#include "unlit.h"

id shdr_unlit_new(id d, id l, void *p, void *v) {
	id<MTLDevice> device = d;
	id<MTLLibrary> lib = l;
	MTLRenderPipelineDescriptor *desc = p;
	MTLVertexDescriptor *vertdesc = v;

	MTLPipelineBufferDescriptorArray *bufs;
	MTLVertexAttributeDescriptorArray *attrs;
	MTLVertexAttributeDescriptor *attr;

	id<MTLFunction> vertunlit = [lib newFunctionWithName:@"vertUnlit"];
	id<MTLFunction> fragunlit = [lib newFunctionWithName:@"fragUnlit"];

	desc.label = @"pipeline.unlit";
	desc.vertexFunction = vertunlit;
	[vertunlit release];
	desc.fragmentFunction = fragunlit;
	[fragunlit release];
	MTLRenderPipelineColorAttachmentDescriptorArray *colors =
		desc.colorAttachments;
	colors[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
	colors[1].pixelFormat = MTLPixelFormatBGRA8Unorm;
	desc.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

	bufs = desc.vertexBuffers;
	bufs[0].mutability = MTLMutabilityImmutable;
	bufs[15].mutability = MTLMutabilityImmutable;

	attr = vertdesc.attributes[0];
	attr.format = MTLVertexFormatFloat3;
	attr.offset = 0;
	attr.bufferIndex = 15;

	vertdesc.layouts[15].stride = sizeof(float) * 3;

	desc.vertexDescriptor = vertdesc;

	return [device newRenderPipelineStateWithDescriptor:desc error:nil];
}
