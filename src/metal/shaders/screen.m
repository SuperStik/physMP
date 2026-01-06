#import <Metal/Metal.h>

#include "screen.h"

void shdr_screen_new(id *pipe, dispatch_group_t group, id d, id l, void *p,
		void *v) {
	id<MTLDevice> device = d;
	id<MTLLibrary> lib = l;
	MTLRenderPipelineDescriptor *desc = p;
	MTLVertexDescriptor *vertdesc = v;

	MTLVertexAttributeDescriptor *attr;

	id<MTLFunction> vertscreen = [lib newFunctionWithName:@"vertScreen"];
	id<MTLFunction> fragscreen = [lib newFunctionWithName:@"fragScreen"];

	desc.label = @"pipeline.screen";
	desc.vertexFunction = vertscreen;
	[vertscreen release];
	desc.fragmentFunction = fragscreen;
	[fragscreen release];
	desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGR10A2Unorm;

	desc.vertexBuffers[15].mutability = MTLMutabilityImmutable;

	attr = vertdesc.attributes[0];
	attr.format = MTLVertexFormatHalf2;
	attr.offset = 0;
	attr.bufferIndex = 15;

	/* stride has to be a multiple of 4 */
	vertdesc.layouts[15].stride = sizeof(_Float16) * 2;

	desc.vertexDescriptor = vertdesc;

	dispatch_group_enter(group);
	[device newRenderPipelineStateWithDescriptor:desc
				   completionHandler:^(
						   id<MTLRenderPipelineState>
						   state, NSError *e) {
					   [state retain];
					   *pipe = state;
					   dispatch_group_leave(group);
				   }];
}
