#include <xetypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <input/input.h>
#include <console/console.h>

#include <ppc/timebase.h>
#include <time/time.h>
#include <ppc/atomic.h>

#include <xenos/xe.h>
#include <xenos/xenos.h>
#include <xenos/edram.h>
#include <xenos/xenos.h>

#include <debug.h>

#include "Vec.h"
#include "video.h"
#include "input.h"
#include "gui/gui.h"
#include "snes9x.h"
#include "gfx.h"

struct ati_info {
	uint32_t unknown1[4];
	uint32_t base;
	uint32_t unknown2[8];
	uint32_t width;
	uint32_t height;
} __attribute__((__packed__));

#define MAX_SHADER 10

typedef unsigned int DWORD;

typedef void (*video_callback)(void*);

typedef struct {
	char name[30];
	struct XenosShader * ps;
	struct XenosShader * vs;
	video_callback callback;
} SnesShader;

typedef struct {
	float x, y, z, w;
	float u, v;
} SnesVerticeFormats;

typedef struct{
	float w;
	float h;
} float2;

typedef struct {
       float2 video_size;
       float2 texture_size;
       float2 output_size;
} _shaderParameters;

#include "shaders/5xBR-v3.7a.ps.h"
#include "shaders/5xBR-v3.7a.vs.h"

#include "shaders/2xBR-v3.5a.ps.h"
#include "shaders/2xBR-v3.5a.vs.h"

//#include "shaders/5xBR-v3.7b.ps.h"
//#include "shaders/5xBR-v3.7b.vs.h"
//
//#include "shaders/5xBR-v3.7c.ps.h"
//#include "shaders/5xBR-v3.7c.vs.h"

//#include "shaders/5xBR-v3.7c_crt.ps.h"
//#include "shaders/5xBR-v3.7c_crt.vs.h"

#include "shaders/scanline.ps.h"
#include "shaders/scanline.vs.h"

#include "shaders/simple.ps.h"
#include "shaders/simple.vs.h"

static matrix4x4 modelViewProj;
static struct XenosVertexBuffer *snes_vb = NULL;
// bitmap emulation
static struct XenosSurface * g_SnesSurface = NULL;
// fb copy of snes display used by gui
static struct XenosSurface * g_SnesSurfaceShadow = NULL;

static int selected_snes_shader = 0;
static int nb_snes_shaders = 0;
static SnesShader SnesShaders[MAX_SHADER];

static _shaderParameters shaderParameters;

static void default_callback(void*) {
	Xe_SetVertexShaderConstantF(g_pVideoDevice, 0, (float*) &modelViewProj, 4);
	Xe_SetVertexShaderConstantF(g_pVideoDevice, 4, (float*) &shaderParameters, 2);
	Xe_SetPixelShaderConstantF(g_pVideoDevice, 0, (float*) &shaderParameters, 2);
}

static void no_filtering_callback(void*) {
	// Disable filtering for xbr
	g_SnesSurface->use_filtering = 0;

	// Shader constant
	Xe_SetVertexShaderConstantF(g_pVideoDevice, 0, (float*) &modelViewProj, 4);
	Xe_SetVertexShaderConstantF(g_pVideoDevice, 4, (float*) &shaderParameters, 2);
	Xe_SetPixelShaderConstantF(g_pVideoDevice, 0, (float*) &shaderParameters, 2);
}

static void loadShader(char * name, const XenosVBFFormat * vbf, const void * ps_main, const void * vs_main, video_callback callback) {
	strcpy(SnesShaders[nb_snes_shaders].name, name);

	SnesShaders[nb_snes_shaders].ps = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) ps_main);
	Xe_InstantiateShader(g_pVideoDevice, SnesShaders[nb_snes_shaders].ps, 0);

	SnesShaders[nb_snes_shaders].vs = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) vs_main);
	Xe_InstantiateShader(g_pVideoDevice, SnesShaders[nb_snes_shaders].vs, 0);
	Xe_ShaderApplyVFetchPatches(g_pVideoDevice, SnesShaders[nb_snes_shaders].vs, 0, vbf);

	SnesShaders[nb_snes_shaders].callback = callback;

	nb_snes_shaders++;

	if (nb_snes_shaders >= MAX_SHADER) {
		printf("Too much shader created !!!\n");
		exit(0);
	}
}

void initSnesVideo() {
	static const struct XenosVBFFormat vbf = {
		2,
		{
			{XE_USAGE_POSITION, 0, XE_TYPE_FLOAT4},
			{XE_USAGE_TEXCOORD, 0, XE_TYPE_FLOAT2},
		}
	};

	loadShader("Normal", &vbf, PSSimple, VSSimple, default_callback);
	//loadShader("Xbr 2x 3.5a", &vbf, PS2xBRa, VS2xBRa, no_filtering_callback); /// too slow ?
	loadShader("Xbr 5x 3.7a", &vbf, PS5xBRa, VS5xBRa, no_filtering_callback);
	loadShader("Scanlines", &vbf, PSScanline, VSScanline, no_filtering_callback);

	snes_vb = Xe_CreateVertexBuffer(g_pVideoDevice, 4096);

	// Create surfaces
	g_SnesSurface = Xe_CreateTexture(g_pVideoDevice, MAX_SNES_WIDTH, MAX_SNES_HEIGHT, 1, XE_FMT_565 | XE_FMT_16BE, 0);
	g_SnesSurfaceShadow = Xe_CreateTexture(g_pVideoDevice, screenwidth, screenheight, 0, XE_FMT_8888 | XE_FMT_ARGB, 0);

	g_SnesSurface->u_addressing = XE_TEXADDR_WRAP;
	g_SnesSurface->v_addressing = XE_TEXADDR_WRAP;

	GFX.Screen = (uint16*) g_SnesSurface->base;
	GFX.Pitch = g_SnesSurface->wpitch;

	memset(g_SnesSurface->base, 0, g_SnesSurface->wpitch * g_SnesSurface->hpitch);
	memset(g_SnesSurfaceShadow->base, 0, g_SnesSurfaceShadow->wpitch * g_SnesSurfaceShadow->hpitch);

	// init fake matrices
	matrixLoadIdentity(&modelViewProj);

}

static int detect_changes(int w, int h) {
	static int old_width = -2000;
	static int old_height = -2000;
	static int widescreen = -2000;
	static int xshift = -2000;
	static int yshift = -2000;
	static float zoomVert = -2000;
	static float zoomHor = -2000;

	int changed = 0;

	if (w != old_width) {
		changed = 1;
		goto end;
	}
	if (h != old_height) {
		changed = 1;
		goto end;
	}

	if (widescreen != GCSettings.widescreen) {
		changed = 1;
		goto end;
	}

	if (xshift != GCSettings.xshift) {
		changed = 1;
		goto end;
	}

	if (yshift != GCSettings.yshift) {
		changed = 1;
		goto end;
	}

	if (zoomVert != GCSettings.zoomVert) {
		changed = 1;
		goto end;
	}

	if (zoomHor != GCSettings.zoomHor) {
		changed = 1;
		goto end;
	}

	// save values for next loop        
end:
	old_width = w;
	old_height = h;
	widescreen = GCSettings.widescreen;
	xshift = GCSettings.xshift;
	yshift = GCSettings.yshift;
	zoomHor = GCSettings.zoomHor;
	zoomVert = GCSettings.zoomVert;

	return changed;
}

static void DrawSnes(XenosSurface * data) {
	if (data == NULL)
		return;

	while (!Xe_IsVBlank(g_pVideoDevice));
	Xe_Sync(g_pVideoDevice);
	
	// detect if something changed
	if (detect_changes(g_SnesSurface->width, g_SnesSurface->height)) {
		// work on vb
		float x, y, w, h;
		float scale = 1.f;
		float xoffset = 0;

		if (GCSettings.widescreen) {
			scale = 3.f / 4.f;
			xoffset = 0.25f;
		}

		w = (scale * 2.f) * GCSettings.zoomHor;
		h = 2.f * GCSettings.zoomVert;

		x = xoffset + (GCSettings.xshift / (float) screenwidth) - 1.f;
		y = (-GCSettings.yshift / (float) screenheight) - 1.f;

		// Update Vb
		SnesVerticeFormats* Rect = (SnesVerticeFormats*) Xe_VB_Lock(g_pVideoDevice, snes_vb, 0, 4096, XE_LOCK_WRITE);
		{
			Rect[0].x = x;
			Rect[0].y = y + h;
			Rect[0].u = 0;
			Rect[0].v = 0;

			// bottom left
			Rect[1].x = x;
			Rect[1].y = y;
			Rect[1].u = 0;
			Rect[1].v = 1;

			// top right
			Rect[2].x = x + w;
			Rect[2].y = y + h;
			Rect[2].u = 1;
			Rect[2].v = 0;

			// top right
			Rect[3].x = x + w;
			Rect[3].y = y + h;
			Rect[3].u = 1;
			Rect[3].v = 0;

			// bottom left
			Rect[4].x = x;
			Rect[4].y = y;
			Rect[4].u = 0;
			Rect[4].v = 1;

			// bottom right
			Rect[5].x = x + w;
			Rect[5].y = y;
			Rect[5].u = 1;
			Rect[5].v = 1;

			int i = 0;
			for (i = 0; i < 6; i++) {
				Rect[i].z = 0.0;
				Rect[i].w = 1.0;
			}
		}
		Xe_VB_Unlock(g_pVideoDevice, snes_vb);
	}

	// Begin draw
	Xe_InvalidateState(g_pVideoDevice);

	Xe_SetCullMode(g_pVideoDevice, XE_CULL_NONE);
	Xe_SetClearColor(g_pVideoDevice, 0);

	// Refresh  texture cache
	Xe_Surface_LockRect(g_pVideoDevice, data, 0, 0, 0, 0, XE_LOCK_WRITE);
	Xe_Surface_Unlock(g_pVideoDevice, data);

	// Set Stream, shader, textures
	Xe_SetTexture(g_pVideoDevice, 0, data);
	Xe_SetShader(g_pVideoDevice, SHADER_TYPE_PIXEL, SnesShaders[selected_snes_shader].ps, 0);
	Xe_SetShader(g_pVideoDevice, SHADER_TYPE_VERTEX, SnesShaders[selected_snes_shader].vs, 0);
	Xe_SetStreamSource(g_pVideoDevice, 0, snes_vb, 0, sizeof (SnesVerticeFormats));

	// use the callback related to selected shader
	if (SnesShaders[selected_snes_shader].callback) {
		SnesShaders[selected_snes_shader].callback(NULL);
	}

	// Draw
	Xe_DrawPrimitive(g_pVideoDevice, XE_PRIMTYPE_RECTLIST, 0, 1);

	// Display
	Xe_Resolve(g_pVideoDevice);
	// while (!Xe_IsVBlank(g_pVideoDevice));
	//Xe_Sync(g_pVideoDevice);
	
	Xe_Execute(g_pVideoDevice);
}

XenosSurface * get_snes_surface() {
	return g_SnesSurfaceShadow;
}

static void ShowFPS(void) {
	static unsigned long lastTick = 0;
	static int frames = 0;
	unsigned long nowTick;
	frames++;
	nowTick = mftb() / (PPC_TIMEBASE_FREQ / 1000);
	if (lastTick + 1000 <= nowTick) {

		printf("%d fps\r\n", frames);

		frames = 0;
		lastTick = nowTick;
	}
}

static int frame = 0;

void update_video(int width, int height) {

	g_SnesSurface->width = width;
	g_SnesSurface->height = height;
	
	shaderParameters.texture_size.w = width;
	shaderParameters.texture_size.h = height;
	shaderParameters.video_size.w = width;
	shaderParameters.video_size.h = height;
	shaderParameters.output_size.w = width;
	shaderParameters.output_size.h = height;

	// move it to callback
	if (GCSettings.render == 1)
		g_SnesSurface->use_filtering = 1;
	else
		g_SnesSurface->use_filtering = 0;

	DrawSnes(g_SnesSurface);

	// Display Menu ?
	if (ScreenshotRequested) {
		// copy fb
		struct ati_info *ai = (struct ati_info*) 0xec806100ULL;

		int width = ai->width;
		int height = ai->height;

		uint32_t * dst = (uint32_t*) Xe_Surface_LockRect(g_pVideoDevice, g_SnesSurfaceShadow, 0, 0, 0, 0, XE_LOCK_WRITE);
		volatile uint32_t *screen = (uint32_t*) (long) (ai->base | 0x80000000);

		int y, x;
		for (y = 0; y < height; ++y) {
			for (x = 0; x < width; ++x) {
				unsigned int base = ((((y & ~31) * width) + (x & ~31)*32) +
						(((x & 3) + ((y & 1) << 2) + ((x & 28) << 1) + ((y & 30) << 5)) ^ ((y & 8) << 2)));
				dst[y * width + x] = 0xFF000000 | __builtin_bswap32(screen[base]);
			}
		}
		Xe_Surface_Unlock(g_pVideoDevice, g_SnesSurfaceShadow);

		if (GCSettings.render == 0) // we can't take a screenshot in Original mode
		{
			GCSettings.render = 2; // switch to unfiltered mode
		} else {
			ScreenshotRequested = 0;
			//TakeScreenshot();
			ConfigRequested = 1;
		}
	}

	ShowFPS();
}

const char* GetFilterName(int filterID) {
	if (filterID >= nb_snes_shaders) {
		return "Unknown";
	} else {
		return SnesShaders[filterID].name;
	}
}

void SelectFilterMethod() {
	selected_snes_shader = GCSettings.FilterMethod;
}

int GetFilterNumber() {
	return nb_snes_shaders;
}