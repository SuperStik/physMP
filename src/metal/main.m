#include <err.h>
#include <pthread.h>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_metal.h>
#include <SDL3/SDL_video.h>
#import <QuartzCore/CAMetalLayer.h>

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#include "../gutl.h"
#include "../math/matrix.h"
#include "../math/vector.h"
#include "main.h"
#include "objc_macros.h"

#define WIDTH 640
#define HEIGHT 480

struct matrices {
	gvec(float,4) view[4];
	gvec(float,4) persp[4];
};

static char done = 0;
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

	char occluded = 0;
	SDL_Event ev;
	while (!done && SDL_WaitEvent(&ev)) {
		switch (ev.type) {
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

	id<MTLRenderPipelineState> levelrps;

	ARP_PUSH();
	NSBundle *bundle = [NSBundle mainBundle];
	NSURL *url = [bundle URLForResource:@"resources/shaders/default"
			      withExtension:@"metallib"];

	id<MTLLibrary> lib = [device newLibraryWithURL:url error:nil];

	id<MTLFunction> levelvert = [lib newFunctionWithName:@"vertLevel"];
	id<MTLFunction> levelfrag = [lib newFunctionWithName:@"fragLevel"];

	[lib release];

	MTLRenderPipelineDescriptor *desc = [MTLRenderPipelineDescriptor new];
	desc.label = @"pipeline.level";
	desc.vertexFunction = levelvert;
	desc.fragmentFunction = levelfrag;
	desc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;

	[levelvert release];
	[levelfrag release];

	levelrps = [device newRenderPipelineStateWithDescriptor:desc error:nil];

	[desc release];
	ARP_POP();

	float verts[] = {
		0.0f, 1.0f, 4.0f,
		1.0f, -1.0f, 4.0f,
		-1.0f, -1.0f, 4.0f
	};

	while (!done) {
		ARP_PUSH();

		id<CAMetalDrawable> drawable = [layer nextDrawable];
		color.texture = drawable.texture;

		id<MTLCommandBuffer> cmdb = [cmdq commandBuffer];

		id<MTLRenderCommandEncoder> enc = [cmdb
			renderCommandEncoderWithDescriptor:rpd];

		[enc setRenderPipelineState:levelrps];
		[enc setVertexBytes:&matrices
			     length:sizeof(matrices)
			    atIndex:0];
		[enc setVertexBytes:verts length:sizeof(verts) atIndex:1];

		[enc drawPrimitives:MTLPrimitiveTypeTriangle
			vertexStart:0
			vertexCount:3];

		[enc endEncoding];

		[cmdb presentDrawable:drawable];
		[cmdb commit];

		ARP_POP();
	}

	[levelrps release];
	[cmdq release];

	return NULL;
}

static void rebuildprojs(struct matrices *mats, float w, float h) {
	GUTL_perspectivef((float *)&(mats->persp), 90.0f, w / h, 0.1f, 128.0f);
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
