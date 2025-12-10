#include <err.h>
#include <pthread.h>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_metal.h>
#include <SDL3/SDL_video.h>
#import <QuartzCore/CAMetalLayer.h>

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#include "../events.h"
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

struct model {
	gvec(float,4) model[4];
	gvec(float,4) normal[3];
};

struct lightdata {
	float position[3];
	float ambient[3];
	float diffuse[3];
	float specular[3];
};

extern char done;

static struct matrices matrices = {MAT_IDENTITY_INITIALIZER,
	MAT_IDENTITY_INITIALIZER};

static pthread_mutex_t depthmut = PTHREAD_MUTEX_INITIALIZER;
static id<MTLTexture> depthtex;

static pthread_mutex_t occlmut = PTHREAD_MUTEX_INITIALIZER;

static void *render(void *l);

static void rebuildprojs(struct matrices *, float w, float h);

static void rebuilddepth(id<MTLDevice>, int32_t width, int32_t height);

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

	int pixw, pixh;
	SDL_GetWindowSizeInPixels(window, &pixw, &pixh);

	rebuilddepth(device, pixw, pixh);

	rebuildprojs(&matrices, (float)WIDTH, (float)HEIGHT);
	SDL_AddEventWatch(windowresize, device);
	
	localplayer.transform = matrices.view;

	pthread_t rthread;
	pthread_create(&rthread, NULL, render, layer);

	char occluded = 0;
	SDL_Event ev;
	while (!done && SDL_WaitEvent(&ev)) {
		switch (ev.type) {
			case SDL_EVENT_KEY_DOWN:
				ev_key_down(window, &ev.key);
				break;
			case SDL_EVENT_KEY_UP:
				ev_key_up(window, &ev.key);
				break;
			case SDL_EVENT_MOUSE_MOTION:
				player_turn(&localplayer, ev.motion.xrel, ev.motion.yrel);
				break;
			case SDL_EVENT_QUIT:
				done = 1;
				break;
			case SDL_EVENT_WINDOW_EXPOSED:
				if (occluded) {
					pthread_mutex_unlock(&occlmut);
					occluded = 0;
				}
				break;
			case SDL_EVENT_WINDOW_OCCLUDED:
				if (!occluded) {
					pthread_mutex_lock(&occlmut);
					occluded = 1;
				}
				break;
		}
	}

	if (occluded)
		pthread_mutex_unlock(&occlmut);

	pthread_join(rthread, NULL);

	[depthtex release];
	[device release];

	SDL_Metal_DestroyView(view);
	SDL_DestroyWindow(window);
}

static void *render(void *l) {
	CAMetalLayer *layer = (__bridge CAMetalLayer *)l;
	id<MTLDevice> device = layer.device;

	id<MTLCommandQueue> cmdq = [device newCommandQueue];

	pthread_setname_np("physMP.render-thread.Metal");

	MTLRenderPassDescriptor *rpd = [MTLRenderPassDescriptor
		renderPassDescriptor];

	MTLRenderPassColorAttachmentDescriptor *color = rpd.colorAttachments[0];
	color.loadAction = MTLLoadActionClear;
	color.storeAction = MTLStoreActionDontCare;
	color.clearColor = MTLClearColorMake(0.5, 0.4, 0.1, 1.0);

	MTLRenderPassDepthAttachmentDescriptor *depth = rpd.depthAttachment;
	depth.loadAction = MTLLoadActionClear;
	depth.storeAction = MTLStoreActionDontCare;

	MTLDepthStencilDescriptor *d_desc = [MTLDepthStencilDescriptor new];
	d_desc.depthCompareFunction = MTLCompareFunctionLessEqual;
	d_desc.depthWriteEnabled = true;
	d_desc.label = @"depth.state.lew";
	id<MTLDepthStencilState> d_state = [device
		newDepthStencilStateWithDescriptor:d_desc];
	[d_desc release];

	struct shaders shdr;
	shdr_load(&shdr, device);

	const float verts[] = {
		128.0f, -4.0f, 128.0f,
		128.0f, -4.0f, -128.0f,
		-128.0f, -4.0f, 128.0f,
		-128.0f, -4.0f, -128.0f
	};

	const struct object_vertdata cube[] = {
		{{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
		{{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},
		{{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
		{{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}},

		{{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},

		{{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},
		{{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},
		{{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},
		{{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}},

		{{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		{{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
		{{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},

		{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
		{{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
		{{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
		{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},

		{{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
		{{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
		{{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	};

	const uint16_t cubeinds[] = {
		0, 1, 2,
		2, 1, 3,

		8, 9, 10,
		10, 9, 11,

		16, 17, 18,
		18, 17, 19,

		4, 5, 6,
		6, 5, 7,

		12, 13, 14,
		14, 13, 15,

		20, 21, 22,
		22, 21, 23
	};

	id<MTLBuffer> cubeinds_buf = [device
		newBufferWithBytes:cubeinds
			    length:sizeof(cubeinds)
			   options:MTLResourceCPUCacheModeWriteCombined];

	id<MTLBuffer> cube_buf = [device
		newBufferWithBytes:cube
			    length:sizeof(cube)
			   options:MTLResourceCPUCacheModeWriteCombined];

	id<MTLTexture> curdepthtex = nil;

	while (!done) {
		ARP_PUSH();

		id<CAMetalDrawable> drawable = [layer nextDrawable];
		color.texture = drawable.texture;

		if (__builtin_expect(curdepthtex != depthtex, 0)) {
			pthread_mutex_lock(&depthmut);
			curdepthtex = depthtex;
			depth.texture = curdepthtex;
			pthread_mutex_unlock(&depthmut);
		}

		id<MTLCommandBuffer> cmdb = [cmdq commandBuffer];

		id<MTLRenderCommandEncoder> enc = [cmdb
			renderCommandEncoderWithDescriptor:rpd];

		[enc setCullMode:MTLCullModeBack];

		[enc setDepthStencilState:d_state];

		[enc setVertexBytes:&matrices
			     length:sizeof(matrices)
			    atIndex:0];

		[enc setRenderPipelineState:shdr.object];

		struct model model;
		memcpy(model.model, modelobj, sizeof(float) * 16);
		gvec(float,4) modelinv[4];
		mat_inverse_t(model.model, modelinv);
		memcpy(model.normal, modelinv, sizeof(float) * 12);
		[enc setVertexBytes:&model
			     length:sizeof(model)
			    atIndex:1];
		[enc setVertexBuffer:cube_buf offset:0 atIndex:15];

		const struct lightdata light = {
			{0.0f, 0.0f, 0.0f},
			{0.2f, 0.2f, 0.2f},
			{0.5f, 0.5f, 0.5f},
			{1.0f, 1.0f, 1.0f}
		};
		[enc setFragmentBytes:&light length:sizeof(light) atIndex:0];

		[enc setFragmentBytes:localplayer.eyepos
			       length:sizeof(float) * 3
			      atIndex:1];

		[enc drawIndexedPrimitives:MTLPrimitiveTypeTriangle
				indexCount:36
				 indexType:MTLIndexTypeUInt16
			       indexBuffer:cubeinds_buf
			 indexBufferOffset:0];

		[enc setRenderPipelineState:shdr.level];
		[enc setVertexBytes:verts length:sizeof(verts) atIndex:15];

		[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip
			vertexStart:0
			vertexCount:4];

		[enc endEncoding];

		[cmdb presentDrawable:drawable];
		[cmdb commit];

		ARP_POP();

		/* pause render thread when window is occluded */
		pthread_mutex_lock(&occlmut);
		pthread_mutex_unlock(&occlmut);
	}

	shdr_release(&shdr);
	[d_state release];
	[cube_buf release];
	[cubeinds_buf release];
	[cmdq release];

	return NULL;
}

static void rebuildprojs(struct matrices *mats, float w, float h) {
	GUTL_perspectivef((float *)&(mats->persp), 90.0f, w / h, 0.1f, 256.0f);
}

static void rebuilddepth(id<MTLDevice> device, int32_t width, int32_t height) {
	ARP_PUSH();

	MTLTextureDescriptor *depthdesc =[MTLTextureDescriptor
		texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
					     width:width
					    height:height
					 mipmapped:false];
	depthdesc.storageMode = MTLStorageModePrivate;
	depthdesc.usage = MTLTextureUsageRenderTarget;

	pthread_mutex_lock(&depthmut);
	[depthtex release];
	depthtex = [device newTextureWithDescriptor:depthdesc];
	pthread_mutex_unlock(&depthmut);

	ARP_POP();

	depthtex.label = @"depth.texture";
}

static bool windowresize(void *udata, SDL_Event *event) {
	id<MTLDevice> device = (__bridge id<MTLDevice>)udata;

	switch (event->type) {
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			rebuilddepth(device, event->window.data1,
					event->window.data2);
			break;
		case SDL_EVENT_WINDOW_RESIZED:
			rebuildprojs(&matrices, (float)event->window.data1,
					(float)event->window.data2);
			break;
	}

	return true;
}
