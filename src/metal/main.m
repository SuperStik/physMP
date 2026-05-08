#include <err.h>
#include <pthread.h>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_metal.h>
#include <SDL3/SDL_video.h>
#import <QuartzCore/CAMetalLayer.h>

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>

#include <events.h>
#include <gutl.h>
#include "main.h"
#include <math/angle.h>
#include <math/matrix.h>
#include <math/vector.h>
#include "shaders.h"
#include <shared.h>

#define WIDTH 640
#define HEIGHT 480

struct matrices {
	gvec(float,4) view[4];
	gvec(float,4) persp[4];
};

struct model {
	gvec(float,4) model[4];
	gvec(float,4) normal[3];
	float viewpos[3];
};

struct lightdata {
	float position[3];
	_Float16 ambient[3];
	_Float16 diffuse[3];
	_Float16 specular[3];
};

extern char done;

static struct matrices matrices = {MTX_IDENTITY_INITIALIZER,
	MTX_IDENTITY_INITIALIZER};

static pthread_mutex_t depthmut = PTHREAD_MUTEX_INITIALIZER;
static id<MTLTexture> depthtex;
static id<MTLTexture> geometrybuf;

static pthread_mutex_t occlmut = PTHREAD_MUTEX_INITIALIZER;
static char occluded = 0;

static void *render(void *l);

static void rebuildprojs(struct matrices *, float w, float h);

static void rebuilddepth(id<MTLDevice>, int32_t width, int32_t height);

static bool windowresize(void *userdata, SDL_Event *);

void MTL_main(void) {
	SDL_Window *window = SDL_CreateWindow("physMP", WIDTH, HEIGHT,
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

	@autoreleasepool {
		NSProcessInfo *pinfo = NSProcessInfo.processInfo;
		bool lowpower = pinfo.lowPowerModeEnabled;

		NSArray<id<MTLDevice>> *devices = MTLCopyAllDevices();
		NSEnumerator<id<MTLDevice>> *devenum = [devices
			objectEnumerator];
		id<MTLDevice> curdevice = nil;
		while(curdevice = [devenum nextObject]) {
			if (curdevice.lowPower == lowpower) {
				device = curdevice;
				[device retain];
				break;
			}
		}

		[devices release];
	}

	if (device == nil) {
		device = layer.preferredDevice;
		[device retain];
	}

	if (device == nil)
		device = MTLCreateSystemDefaultDevice();

	if (__builtin_expect(device == nil, 0))
		err(1, "Failed to get device!");

	layer.device = device;
	layer.pixelFormat = MTLPixelFormatBGR10A2Unorm;

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
				player_turn(&localplayer, ev.motion.xrel,
						ev.motion.yrel);
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
	[geometrybuf release];
	[device release];

	SDL_Metal_DestroyView(view);
	SDL_DestroyWindow(window);
}

static void *render(void *l) {
	CAMetalLayer *layer = (__bridge CAMetalLayer *)l;
	id<MTLDevice> device = layer.device;

	id<MTLCommandQueue> cmdq = [device newCommandQueue];

	pthread_setname_np("physMP.render-thread.Metal");

	MTLRenderPassDescriptor *scrrpd = [MTLRenderPassDescriptor
		renderPassDescriptor];

	MTLRenderPassColorAttachmentDescriptor *color =
		scrrpd.colorAttachments[0];
	/*color.loadAction = MTLLoadActionDontCare;*/
	color.storeAction = MTLStoreActionDontCare;

	MTLRenderPassDescriptor *geomrpd = [MTLRenderPassDescriptor
		renderPassDescriptor];
	MTLRenderPassColorAttachmentDescriptorArray *geombufs =
		geomrpd.colorAttachments;

	MTLRenderPassColorAttachmentDescriptor *albedo_specular = geombufs[0];
	albedo_specular.loadAction = MTLLoadActionClear;
	albedo_specular.storeAction = MTLStoreActionStore;
	albedo_specular.clearColor = MTLClearColorMake(0.5, 0.4, 0.1, 1.0);

	MTLRenderPassColorAttachmentDescriptor *normal_shadow = geombufs[1];
	normal_shadow.slice = 1;
	normal_shadow.loadAction = MTLLoadActionClear;
	normal_shadow.storeAction = MTLStoreActionStore;
	normal_shadow.clearColor = MTLClearColorMake(0.5, 0.5, 1.0, 1.0);

	MTLRenderPassDepthAttachmentDescriptor *depth = geomrpd.depthAttachment;
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

	const _Float16 screenrect[] = {
		1.0f16, 1.0f16,
		1.0f16, -1.0f16,
		-1.0f16, 1.0f16,
		-1.0f16, -1.0f16
	};

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

	const MTLResourceOptions opts = MTLResourceCPUCacheModeWriteCombined |
		MTLResourceHazardTrackingModeUntracked;

	id<MTLBuffer> buf_cubeind = [device newBufferWithBytes:cubeinds
							length:sizeof(cubeinds)
						       options:opts];
	buf_cubeind.label = @"buffer.cube.indices";

	id<MTLBuffer> buf_cube = [device newBufferWithBytes:cube
						     length:sizeof(cube)
						    options:opts];
	buf_cube.label = @"buffer.cube.vertex_attributes";

	id<MTLFence> deferfence = [device newFence];

	while (!done) {
		/* pause render thread if window is occluded */
		if (__builtin_expect(occluded, 0)) {
			pthread_mutex_lock(&occlmut);
			pthread_mutex_unlock(&occlmut);
		}

		@autoreleasepool {
			id<MTLCommandBuffer> cmdb = [cmdq commandBuffer];

			id<CAMetalDrawable> drawable = [layer nextDrawable];
			color.texture = drawable.texture;

			id<MTLRenderCommandEncoder> scr = [cmdb
				renderCommandEncoderWithDescriptor:scrrpd];
			[scr setRenderPipelineState:shdr.screen];

			[scr setVertexBytes:screenrect
				     length:sizeof(screenrect)
				    atIndex:15];

			[scr waitForFence:deferfence
			     beforeStages:MTLRenderStageFragment];

			[scr setFragmentTexture:albedo_specular.texture
					atIndex:0];

			[scr drawPrimitives:MTLPrimitiveTypeTriangleStrip
				vertexStart:0
				vertexCount:4];

			[scr endEncoding];

			if (__builtin_expect(depth.texture != depthtex, 0)) {
				pthread_mutex_lock(&depthmut);
				depth.texture = depthtex;
				albedo_specular.texture = geometrybuf;
				normal_shadow.texture = geometrybuf;
				pthread_mutex_unlock(&depthmut);
			}

			id<MTLRenderCommandEncoder> enc = [cmdb
				renderCommandEncoderWithDescriptor:geomrpd];

			[enc updateFence:deferfence
			     afterStages:MTLRenderStageFragment];

			[enc setCullMode:MTLCullModeBack];

			[enc setDepthStencilState:d_state];

			[enc setVertexBytes:&matrices
				     length:sizeof(matrices)
				    atIndex:0];

			[enc setRenderPipelineState:shdr.blinnphong];

			struct model model;
			memcpy(model.model, modelobj, sizeof(float) * 16);
			gvec(float,4) modelinv[4];
			mtx_inverse_t(model.model, modelinv);
			memcpy(model.normal, modelinv, sizeof(float) * 12);
			model.viewpos[0] = localplayer.eyepos[0];
			model.viewpos[1] = localplayer.eyepos[1];
			model.viewpos[2] = localplayer.eyepos[2];
			[enc setVertexBytes:&model
				     length:sizeof(model)
				    atIndex:1];
			[enc setVertexBuffer:buf_cube offset:0 atIndex:15];

			const struct lightdata light = {
				{0.0f, 0.0f, 0.0f},
				{0.2f16, 0.2f16, 0.2f16},
				{0.5f16, 0.5f16, 0.5f16},
				{1.0f16, 1.0f16, 1.0f16}
			};
			[enc setFragmentBytes:&light
				       length:sizeof(light)
				      atIndex:0];

			[enc drawIndexedPrimitives:MTLPrimitiveTypeTriangle
					indexCount:36
					 indexType:MTLIndexTypeUInt16
				       indexBuffer:buf_cubeind
				 indexBufferOffset:0];

			[enc setRenderPipelineState:shdr.unlit];
			[enc setVertexBytes:verts
				     length:sizeof(verts)
				    atIndex:15];

			[enc drawPrimitives:MTLPrimitiveTypeTriangleStrip
				vertexStart:0
				vertexCount:4];

			[enc endEncoding];

			[cmdb presentDrawable:drawable];
			[cmdb commit];
		}
	}

	shdr_release(&shdr);
	[d_state release];
	[deferfence release];
	[buf_cube release];
	[buf_cubeind release];
	[cmdq release];

	return NULL;
}

static void rebuildprojs(struct matrices *mats, float w, float h) {
	GUTL_perspectivef((float *)&(mats->persp), 90.0f, w / h, 0.1f, 256.0f);
}

static void rebuilddepth(id<MTLDevice> device, int32_t width, int32_t height) {
	const MTLPixelFormat depthformat = MTLPixelFormatDepth32Float;
	@autoreleasepool {
		MTLTextureDescriptor *desc = [MTLTextureDescriptor
			texture2DDescriptorWithPixelFormat:depthformat
						     width:width
						    height:height
						 mipmapped:false];
		desc.storageMode = MTLStorageModePrivate;
		desc.usage = MTLTextureUsageRenderTarget;

		pthread_mutex_lock(&depthmut);
		[depthtex release];
		[geometrybuf release];

		depthtex = [device newTextureWithDescriptor:desc];
		desc.textureType = MTLTextureType2DArray;
		desc.pixelFormat = MTLPixelFormatBGRA8Unorm;
		desc.arrayLength = 2;
		desc.usage = MTLTextureUsageShaderRead |
			MTLTextureUsageRenderTarget;
		geometrybuf = [device newTextureWithDescriptor:desc];
		pthread_mutex_unlock(&depthmut);
	}

	depthtex.label = @"framebuffer.depth";
	geometrybuf.label = @"framebuffer.geometrybuffer";
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

	return false;
}
