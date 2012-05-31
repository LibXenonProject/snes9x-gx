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

#include "video.h"
#include "input.h"
#include "gui/gui.h"
#include "snes9x.h"
#include "gfx.h"

#define MAX_SHADER 10

typedef unsigned int DWORD;

typedef void (*video_callback)(void*);

typedef struct {
        char name[10];
        struct XenosShader * ps;
        struct XenosShader * vs;
        video_callback callback;
} SnesShader;

typedef struct {
        float x, y, z, w; // 32
        unsigned int color; // 36
        float u, v; // 48
} SnesVerticeFormats;

#include "shaders/xbr_5x_ps.h"
#include "shaders/xbr_5x_vs.h"

static struct XenosVertexBuffer *snes_vb = NULL;
static struct XenosSurface * g_SnesSurface = NULL;
static struct XenosSurface * g_SnesSurfaceShadow = NULL;

static int selected_snes_shader = 0;
static int nb_snes_shaders = 0;
static SnesShader SnesShaders[MAX_SHADER];

static int unsigned int video_lock = 0;


static void video_thread();

static void CreateVbSnes(float x, float y, float w, float h, uint32_t color, SnesVerticeFormats * Rect) {
        Rect[0].x = x;
        Rect[0].y = y + h;
        Rect[0].u = 0.f;
        Rect[0].v = 0.f;
        Rect[0].color = color;

        // bottom left
        Rect[1].x = x;
        Rect[1].y = y;
        Rect[1].u = 0.f;
        Rect[1].v = 1.f;
        Rect[1].color = color;

        // top right
        Rect[2].x = x + w;
        Rect[2].y = y + h;
        Rect[2].u = 1.f;
        Rect[2].v = 0.f;
        Rect[2].color = color;

        for (int i = 0; i < 3; i++) {
                Rect[i].z = 0.0;
                Rect[i].w = 1.0;
        }
}

static void xbr_callback(void*) {
        // Disable filtering for xbr
        g_SnesSurface->use_filtering = 0;

        // Shader constant
        float settings_texture_size[2] = {g_SnesSurface->width, g_SnesSurface->height};
        Xe_SetVertexShaderConstantF(g_pVideoDevice, 0, settings_texture_size, 1);
        Xe_SetPixelShaderConstantF(g_pVideoDevice, 0, settings_texture_size, 1);
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
        static const struct XenosVBFFormat xbr_vbf = {
                3,
                {
                        {XE_USAGE_POSITION, 0, XE_TYPE_FLOAT4},
                        {XE_USAGE_COLOR, 0, XE_TYPE_UBYTE4},
                        {XE_USAGE_TEXCOORD, 0, XE_TYPE_FLOAT2},
                }
        };

        loadShader("Xbr 5x 3.7a", &xbr_vbf, g_xps_xbr5x_ps_main, g_xvs_xbr5x_vs_main, xbr_callback);

        snes_vb = Xe_CreateVertexBuffer(g_pVideoDevice, 4096);

        // Create snes surface
        g_SnesSurface = Xe_CreateTexture(g_pVideoDevice, MAX_SNES_WIDTH, MAX_SNES_HEIGHT, 1, XE_FMT_565 | XE_FMT_16BE, 0);
        g_SnesSurfaceShadow = Xe_CreateTexture(g_pVideoDevice, MAX_SNES_WIDTH, MAX_SNES_HEIGHT, 1, XE_FMT_565 | XE_FMT_16BE, 0);

        g_SnesSurface->u_addressing = XE_TEXADDR_WRAP;
        g_SnesSurface->v_addressing = XE_TEXADDR_WRAP;

        GFX.Screen = (uint16*) g_SnesSurfaceShadow->base;
        GFX.Pitch = g_SnesSurface->wpitch;

        memset(g_SnesSurface->base, 0, g_SnesSurface->wpitch * g_SnesSurface->hpitch);
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
        zoomHor= GCSettings.zoomHor;
        zoomVert= GCSettings.zoomVert;

        return changed;
}

static void DrawSnes(XenosSurface * data) {
        if (data == NULL)
                return;

        // detect if something changed
        if(detect_changes(g_SnesSurface->width, g_SnesSurface->height)){
                // work on vb
                float x, y, w, h;
                float scale = 1.f;

                if (GCSettings.widescreen) {
                        scale = 3.f / 4.f;
                }

                w = (scale * 2.f ) * GCSettings.zoomHor;
                h = 2.f * GCSettings.zoomVert;

                x = ( GCSettings.xshift / (float) screenwidth) - 1.f;
                y = (- GCSettings.yshift / (float) screenheight) - 1.f;

                // Update Vb
                SnesVerticeFormats* Rect = (SnesVerticeFormats*) Xe_VB_Lock(g_pVideoDevice, snes_vb, 0, 4096, XE_LOCK_WRITE);
                {
                        CreateVbSnes(w, h, x, y, 0xFFFFFFFF, Rect);
                }
                Xe_VB_Unlock(g_pVideoDevice, snes_vb);
        }

        // Begin draw
        Xe_InvalidateState(g_pVideoDevice);

        Xe_SetCullMode(g_pVideoDevice, XE_CULL_NONE);
        Xe_SetClearColor(g_pVideoDevice, 0);

        Xe_SetFillMode(g_pVideoDevice, XE_FILL_WIREFRAME, XE_FILL_WIREFRAME);

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
        Xe_Sync(g_pVideoDevice);
}

XenosSurface * get_snes_surface() {
        return g_SnesSurface;
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

static void video_thread(){
        while(){                
                lock(&video_lock);
                if(frame == 1){
                        // copy
                        memcpy(g_SnesSurface->base, g_SnesSurfaceShadow->base, g_SnesSurface->wpitch * g_SnesSurface->hpitch);
                }
                frame = 0;
                unlock(&video_lock);
        }
}

void update_video(int width, int height) {
        //lock(&video_lock);
        
        g_SnesSurface->width = width;
        g_SnesSurface->height = height;
        
        //unlock(&video_lock);
        
        //  Menu_DrawImg(0, 0, screenwidth, screenheight, g_SnesSurface, 0, 1, 1, 0xFF);

        // move it to callback
        if (GCSettings.FilterMethod == 1)
                g_SnesSurface->use_filtering = 1;
        else
                g_SnesSurface->use_filtering = 0;

        DrawSnes(g_SnesSurface);

        // Display Menu ?
        if (ScreenshotRequested) {
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