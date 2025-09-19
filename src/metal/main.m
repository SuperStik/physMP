#include <err.h>
#include <pthread.h>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_metal.h>
#include <SDL3/SDL_video.h>
#import <QuartzCore/CAMetalLayer.h>

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#include "main.h"
#include "objc_macros.h"

#define WIDTH 640
#define HEIGHT 480

static id<MTLBuffer> matbuf;
static char done = 0;

static void *render(void *c);

static void updatemats(float *matrices, float width, float height);

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

	bool unified = [device hasUnifiedMemory];

	const MTLResourceOptions matbufops =
		MTLResourceCPUCacheModeWriteCombined;
	matbuf = [device newBufferWithLength:(sizeof(float) * (16 * 1))
				     options:matbufops];
	float *matrices = (float *)[matbuf contents];

	updatemats(matrices, (float)WIDTH, (float)HEIGHT);
	const NSRange matrange = NSMakeRange(0, sizeof(float) * (16 * 1));
	if (!unified)
		[matbuf didModifyRange:matrange];

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

	[matbuf release];
	[device release];

	SDL_Metal_DestroyView(view);
	SDL_DestroyWindow(window);
}

static void *render(void *c) {
	return NULL;
}

static void updatemats(float *matrices, float width, float height) {
}
