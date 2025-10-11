#include <err.h>
#include <pthread.h>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_metal.h>
#include <SDL3/SDL_video.h>
#import <QuartzCore/CAMetalLayer.h>

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#include "../gutl.h"
#include "../math/angle.h"
#include "../math/matrix.h"
#include "../math/vector.h"
#include "../shared.h"
#include "main.h"
#include "objc_macros.h"
#include "shaders.h"

#define WIDTH 640
#define HEIGHT 480

struct matrices {
	gvec(float,4) view[4];
	gvec(float,4) persp[4];
};

extern char done;
static struct matrices matrices = {MAT_IDENTITY_INITIALIZER,
	MAT_IDENTITY_INITIALIZER};

static void *render(void *l);

static void rebuildprojs(struct matrices *, float w, float h);

static bool windowresize(void *userdata, SDL_Event *);

void MTL_main(void) {
	SDL_Window *window = SDL_CreateWindow("unbloCked", WIDTH, HEIGHT,
			SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY |
			SDL_WINDOW_METAL);
	if (window == NULL)
		errx(1, "%s", SDL_GetError());

	if (!SDL_SetWindowMinimumSize(window, WIDTH, HEIGHT))
		warnx("%s", SDL_GetError());

	SDL_SetWindowRelativeMouseMode(window, true);

	SDL_MetalView view = SDL_Metal_CreateView(window);
	void *l = SDL_Metal_GetLayer(view);
	CAMetalLayer *layer = (__bridge CAMetalLayer *)l;

	id<MTLDevice> device = nil;

	ARP_PUSH();
	NSProcessInfo *pinfo = NSProcessInfo.processInfo;
	bool lowpower = pinfo.lowPowerModeEnabled;

	NSArray<id<MTLDevice>> *devices = MTLCopyAllDevices();
	NSEnumerator<id<MTLDevice>> *devenum = [devices objectEnumerator];
	id<MTLDevice> curdevice = nil;
	while(curdevice = [devenum nextObject]) {
		if (curdevice.lowPower == lowpower) {
			device = curdevice;
			[device retain];
			break;
		}
	}

	[devices release];
	ARP_POP();

	if (device == nil) {
		device = layer.preferredDevice;
		[device retain];
	}

	if (device == nil)
		device = MTLCreateSystemDefaultDevice();

	if (__builtin_expect(device == nil, 0))
		err(1, "Failed to get device!");

	layer.device = device;
	layer.pixelFormat = MTLPixelFormatBGRA8Unorm;

	/* just to make sure Cocoa is in multithreaded mode */
	if (__builtin_expect(![NSThread isMultiThreaded], false)) {
		NSThread *dummy = [NSThread new];
		[dummy start];
		[dummy cancel];
		[dummy release];
	}

	rebuildprojs(&matrices, (float)WIDTH, (float)HEIGHT);
	SDL_AddEventWatch(windowresize, &matrices);

	pthread_t rthread;
	pthread_create(&rthread, NULL, render, layer);

	float pitch = 0.0f;
	float yaw = 0.0f;
	char occluded = 0;
	SDL_Event ev;
	while (!done && SDL_WaitEvent(&ev)) {
		switch (ev.type) {
			case SDL_EVENT_MOUSE_MOTION:
				pitch -= ev.motion.yrel * 0.1f;
				yaw -= ev.motion.xrel * 0.1f;
				if (fabsf(pitch) > 90.0f)
					pitch = copysignf(90.0f, pitch);

				gvec(float,4) rot = ang_eulnoroll2quat(pitch,
						yaw);
				mat_getrotate(matrices.view, rot);
				break;
			case SDL_EVENT_QUIT:
				done = 1;
				break;
		}
	}

	pthread_join(rthread, NULL);

	[device release];

	SDL_Metal_DestroyView(view);
	SDL_DestroyWindow(window);
}

static void *render(void *l) {
	CAMetalLayer *layer = (__bridge CAMetalLayer *)l;
	id<MTLDevice> device = layer.device;

	id<MTLCommandQueue> cmdq = [device newCommandQueue];

	MTLRenderPassDescriptor *rpd = [MTLRenderPassDescriptor
		renderPassDescriptor];
	MTLRenderPassColorAttachmentDescriptor *color = rpd.colorAttachments[0];
	color.loadAction = MTLLoadActionClear;
	color.storeAction = MTLStoreActionDontCare;
	color.clearColor = MTLClearColorMake(0.5, 0.4, 0.1, 1.0);

	struct shaders shdr;
	shdr_load(&shdr, device);

	const float verts[] = {
		128.0f, -16.0f, 128.0f,
		128.0f, -16.0f, -128.0f,
		-128.0f, -16.0f, 128.0f,
		-128.0f, -16.0f, -128.0f
	};

	const float cube[] = {
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, 0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, 0.5f,
	};

	const uint16_t cubeinds[] = {
		0, 1, 2,
		2, 1, 3,

		0, 4, 1,
		1, 4, 5,

		0, 2, 4,
		4, 2, 6,

		4, 6, 5,
		5, 6, 7,

		2, 3, 6,
		6, 3, 7,

		1, 5, 3,
		3, 5, 7
	};

	id<MTLBuffer> cubeinds_buf = [device
		newBufferWithBytes:cubeinds
			    length:sizeof(cubeinds)
			   options:MTLResourceCPUCacheModeWriteCombined];

	while (!done) {
		ARP_PUSH();

		id<CAMetalDrawable> drawable = [layer nextDrawable];
		color.texture = drawable.texture;

		id<MTLCommandBuffer> cmdb = [cmdq commandBuffer];

		id<MTLRenderCommandEncoder> enc = [cmdb
			renderCommandEncoderWithDescriptor:rpd];

		[enc setCullMode:MTLCullModeBack];

		[enc setRenderPipelineState:shdr.level];
		[enc setVertexBytes:&matrices
			     length:sizeof(matrices)
			    atIndex:0];

		[enc setVertexBytes:verts length:sizeof(verts) atIndex:1];

		[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip
			vertexStart:0
			vertexCount:4];

		[enc setRenderPipelineState:shdr.object];
		[enc setVertexBytes:modelobj
			     length:sizeof(float) * 16
			    atIndex:1];
		[enc setVertexBytes:cube length:sizeof(cube) atIndex:15];

		[enc drawIndexedPrimitives:MTLPrimitiveTypeTriangle
				indexCount:36
				 indexType:MTLIndexTypeUInt16
			       indexBuffer:cubeinds_buf
			 indexBufferOffset:0];

		[enc endEncoding];

		[cmdb presentDrawable:drawable];
		[cmdb commit];

		ARP_POP();
	}

	shdr_release(&shdr);
	[cubeinds_buf release];
	[cmdq release];

	return NULL;
}

static void rebuildprojs(struct matrices *mats, float w, float h) {
	GUTL_perspectivef((float *)&(mats->persp), 90.0f, w / h, 0.1f, 256.0f);
}

static bool windowresize(void *udata, SDL_Event *event) {
	struct matrices *mats = udata;

	switch (event->type) {
		case SDL_EVENT_WINDOW_RESIZED:
			rebuildprojs(mats, (float)event->window.data1,
					(float)event->window.data2);
			break;
	}

	return true;
}
