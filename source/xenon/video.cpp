/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 *
 * video.cpp
 * Video routines
 ***************************************************************************/

#include <xetypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <input/input.h>
#include <console/console.h>

#include <ppc/timebase.h>
#include <time/time.h>

#include <xenos/xe.h>
#include <xenos/xenos.h>
#include <xenos/edram.h>
#include <xenos/xenos.h>

#include "input.h"
#include "gui/gui.h"

#include <Vec.h>

#include <debug.h>

#include "snes9x.h"
#include "gfx.h"

int LoadTextureFromFile(char * pSrcFile, XenosSurface **ppTexture);

typedef unsigned int DWORD;
//#include "ps.h"
//#include "vs.h"
#include "shaders/vs.h"
#include "shaders/ps.t.h"
#include "shaders/ps.c.h"
//#include "shaders/ps.snes.h"

#include "shaders/xbr_5x_ps.h"
#include "shaders/xbr_5x_vs.h"

#define DEFAULT_FIFO_SIZE 256 * 1024

//static GXRModeObj *vmode; // Menu video mode
//static unsigned char gp_fifo[DEFAULT_FIFO_SIZE] ATTRIBUTE_ALIGN(32);
//static Mtx GXmodelView2D;
int screenheight;
int screenwidth;
u32 FrameTimer = 0;

#define MAX_VERTEX_COUNT 65536

static struct XenosDevice _xe;
static struct XenosVertexBuffer *vb = NULL;
static struct XenosVertexBuffer *snes_vb = NULL;
struct XenosDevice * g_pVideoDevice = NULL;
static struct XenosShader * g_pVertexShader = NULL;
static struct XenosShader * g_pPixelTexturedShader = NULL;
static struct XenosShader * g_pPixelColoredShader = NULL;
static struct XenosShader * g_pPixelSnesShader = NULL;
static struct XenosShader * g_pVertexSnesShader = NULL;

//XenosSurface * g_pTexture;

matrix4x4 modelView2D;
matrix4x4 projection;
//matrix4x4 WVP;
static int nb_vertices = 0;

static struct XenosSurface * g_SnesSurface = NULL;

typedef struct {
        float x, y, z, w; // 32
        unsigned int color; // 36
        unsigned int padding; // 40
        float u, v; // 48
} __attribute__((packed)) DrawVerticeFormats;

typedef struct {
        float x, y, z, w; // 32
        unsigned int color; // 36
        float u, v; // 48
} SnesVerticeFormats;

// Init Matrices

void InitMatrices() {
        matrix4x4 WVP;
        matrixLoadIdentity(&WVP);

        matrixLoadIdentity(&projection);
        matrixOrthoRH(&projection, 640, 480, 0, 300);

        matrixLoadIdentity(&modelView2D);
        matrixTranslation(&modelView2D, 0, 0, -50.f);
}

/****************************************************************************
 * ResetVideo_Menu
 *
 * Reset the video/rendering mode for the menu
 ****************************************************************************/
void
ResetVideo_Menu() {
        // Init Matrices
        InitMatrices();
}

void CreateVbText(float x, float y, float w, float h, uint32_t color, DrawVerticeFormats * Rect) {
        // bottom left
        Rect[0].x = x - w;
        Rect[0].y = y + h;
        Rect[0].u = 0;
        Rect[0].v = 1;
        Rect[0].color = color;

        // bottom right
        Rect[1].x = x + w;
        Rect[1].y = y + h;
        Rect[1].u = 1;
        Rect[1].v = 1;
        Rect[1].color = color;

        // top right
        Rect[2].x = x + w;
        Rect[2].y = y - h;
        Rect[2].u = 1;
        Rect[2].v = 0;
        Rect[2].color = color;

        // Top left
        Rect[3].x = x - w;
        Rect[3].y = y - h;
        Rect[3].u = 0;
        Rect[3].v = 0;
        Rect[3].color = color;

        int i = 0;
        for (i = 0; i < 4; i++) {
                Rect[i].z = 0.0;
                Rect[i].w = 1.0;
        }
}

void CreateVb(float x, float y, float w, float h, uint32_t color, DrawVerticeFormats * Rect) {
        // bottom left
        Rect[0].x = x - w;
        Rect[0].y = y - h;
        Rect[0].u = 0;
        Rect[0].v = 0;
        Rect[0].color = color;

        // bottom right
        Rect[1].x = x + w;
        Rect[1].y = y - h;
        Rect[1].u = 1;
        Rect[1].v = 0;
        Rect[1].color = color;

        // top right
        Rect[2].x = x + w;
        Rect[2].y = y + h;
        Rect[2].u = 1;
        Rect[2].v = 1;
        Rect[2].color = color;

        // Top left
        Rect[3].x = x - w;
        Rect[3].y = y + h;
        Rect[3].u = 0;
        Rect[3].v = 1;
        Rect[3].color = color;

        int i = 0;
        for (i = 0; i < 4; i++) {
                Rect[i].z = 0.0;
                Rect[i].w = 1.0;
        }
}

void CreateVbQuad(float width, float height, uint32_t color, DrawVerticeFormats * Rect) {
        // bottom left
        Rect[0].x = -width;
        Rect[0].y = -height;
        Rect[0].u = 0;
        Rect[0].v = 0;
        Rect[0].color = color;

        // bottom right
        Rect[1].x = width;
        Rect[1].y = -height;
        Rect[1].u = 1;
        Rect[1].v = 0;
        Rect[1].color = color;

        // top right
        Rect[2].x = width;
        Rect[2].y = height;
        Rect[2].u = 1;
        Rect[2].v = 1;
        Rect[2].color = color;

        // Top left
        Rect[3].x = -width;
        Rect[3].y = height;
        Rect[3].u = 0;
        Rect[3].v = 1;
        Rect[3].color = color;

        int i = 0;
        for (i = 0; i < 4; i++) {
                Rect[i].z = 0.0;
                Rect[i].w = 1.0;
        }
}

void CreateVbSnes(float x, float y, float w, float h, uint32_t color, SnesVerticeFormats * Rect) {
        // bottom left
        Rect[0].x = x - w;
        Rect[0].y = y - h;
        Rect[0].u = 0;
        Rect[0].v = 0;
        Rect[0].color = color;

        // bottom right
        Rect[1].x = x + w;
        Rect[1].y = y - h;
        Rect[1].u = 1;
        Rect[1].v = 0;
        Rect[1].color = color;

        // top right
        Rect[2].x = x + w;
        Rect[2].y = y + h;
        Rect[2].u = 1;
        Rect[2].v = 1;
        Rect[2].color = color;

        // Top left
        Rect[3].x = x - w;
        Rect[3].y = y + h;
        Rect[3].u = 0;
        Rect[3].v = 1;
        Rect[3].color = color;

        int i = 0;
        for (i = 0; i < 4; i++) {
                Rect[i].z = 0.0;
                Rect[i].w = 1.0;
        }
}

void oCreateVbQuad(float width, float height, uint32_t color, DrawVerticeFormats * Rect) {
        // bottom left
        Rect[0].x = -width;
        Rect[0].y = -height;
        Rect[0].u = 0;
        Rect[0].v = 0;
        Rect[0].color = color;

        // bottom right
        Rect[1].x = width;
        Rect[1].y = -height;
        Rect[1].u = 1;
        Rect[1].v = 0;
        Rect[1].color = color;

        // top right
        Rect[2].x = width;
        Rect[2].y = height;
        Rect[2].u = 1;
        Rect[2].v = 1;
        Rect[2].color = color;

        // Top left
        Rect[3].x = -width;
        Rect[3].y = height;
        Rect[3].u = 0;
        Rect[3].v = 1;
        Rect[3].color = color;

        int i = 0;
        for (i = 0; i < 4; i++) {
                Rect[i].z = 0.0;
                Rect[i].w = 1.0;
        }
}

extern "C" struct XenosDevice * GetVideoDevice() {
        return g_pVideoDevice;
}

/****************************************************************************
 * InitVideo
 *
 * This function MUST be called at startup.
 * - also sets up menu video mode
 ***************************************************************************/
void
InitVideo() {
        xenos_init(VIDEO_MODE_AUTO);
        //console_init();

        g_pVideoDevice = &_xe;

        Xe_Init(g_pVideoDevice);

        XenosSurface * fb = Xe_GetFramebufferSurface(g_pVideoDevice);

        screenheight = ((float) fb->height)*(720.f / (float) fb->height);
        screenwidth = ((float) fb->width)*(1280.f / (float) fb->width);

        //        screenheight = ((float) fb->height)*(480.f / (float) fb->height);
        //        screenwidth = ((float) fb->width)*(640.f / (float) fb->width);

        //    screenheight = 480;
        //    screenwidth = 640;

        Xe_Init(g_pVideoDevice);

        Xe_SetRenderTarget(g_pVideoDevice, Xe_GetFramebufferSurface(g_pVideoDevice));

        static const struct XenosVBFFormat vbf = {
                4,
                {
                        {XE_USAGE_POSITION, 0, XE_TYPE_FLOAT4},
                        {XE_USAGE_COLOR, 0, XE_TYPE_UBYTE4},
                        {XE_USAGE_COLOR, 1, XE_TYPE_UBYTE4}, //padding
                        {XE_USAGE_TEXCOORD, 0, XE_TYPE_FLOAT2},
                }
        };

        static const struct XenosVBFFormat xbr_vbf = {
                3,
                {
                        {XE_USAGE_POSITION, 0, XE_TYPE_FLOAT4},
                        {XE_USAGE_COLOR, 0, XE_TYPE_UBYTE4},
                        {XE_USAGE_TEXCOORD, 0, XE_TYPE_FLOAT2},
                }
        };

        g_pPixelTexturedShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) g_xps_psT);
        Xe_InstantiateShader(g_pVideoDevice, g_pPixelTexturedShader, 0);

        g_pPixelColoredShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) g_xps_psC);
        Xe_InstantiateShader(g_pVideoDevice, g_pPixelColoredShader, 0);

        g_pVertexShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) g_xvs_VSmain);
        Xe_InstantiateShader(g_pVideoDevice, g_pVertexShader, 0);

        Xe_ShaderApplyVFetchPatches(g_pVideoDevice, g_pVertexShader, 0, &vbf);


        g_pPixelSnesShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) g_xps_xbr5x_ps_main);
        Xe_InstantiateShader(g_pVideoDevice, g_pPixelSnesShader, 0);
        g_pVertexSnesShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) g_xvs_xbr5x_vs_main);
        Xe_InstantiateShader(g_pVideoDevice, g_pVertexSnesShader, 0);
        Xe_ShaderApplyVFetchPatches(g_pVideoDevice, g_pVertexSnesShader, 0, &xbr_vbf);
        
//        g_pPixelSnesShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) g_xps_PS);
//        Xe_InstantiateShader(g_pVideoDevice, g_pPixelSnesShader, 0);
//        g_pVertexSnesShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) g_xvs_VS);
//        Xe_InstantiateShader(g_pVideoDevice, g_pVertexSnesShader, 0);
//        Xe_ShaderApplyVFetchPatches(g_pVideoDevice, g_pVertexSnesShader, 0, &xbr_vbf);

        edram_init(g_pVideoDevice);

        vb = Xe_CreateVertexBuffer(g_pVideoDevice, MAX_VERTEX_COUNT * sizeof (DrawVerticeFormats));
        snes_vb = Xe_CreateVertexBuffer(g_pVideoDevice, 4096);

        Xe_SetClearColor(g_pVideoDevice, 0xFFFFFFFF);

        //    LoadTextureFromFile("uda:/1.png", &g_pTexture);

        Xe_InvalidateState(g_pVideoDevice);

        // Create snes surface
        g_SnesSurface = Xe_CreateTexture(g_pVideoDevice, MAX_SNES_WIDTH, MAX_SNES_HEIGHT, 1, XE_FMT_565 | XE_FMT_16BE, 0);

        g_SnesSurface->u_addressing = XE_TEXADDR_WRAP;
        g_SnesSurface->v_addressing = XE_TEXADDR_WRAP;

        GFX.Screen = (uint16*) g_SnesSurface->base;
        GFX.Pitch = g_SnesSurface->wpitch;

        memset(g_SnesSurface->base, 0, g_SnesSurface->wpitch * g_SnesSurface->hpitch);

        ResetVideo_Menu();
}

/****************************************************************************
 * StopGX
 *
 * Stops GX (when exiting)
 ***************************************************************************/
void StopGX() {
        //    GX_AbortFrame();
        //    GX_Flush();
        //
        //    VIDEO_SetBlack(TRUE);
        //    VIDEO_Flush();
}

extern "C" void doScreenCapture();

/****************************************************************************
 * Menu_Render
 *
 * Renders everything current sent to GX, and flushes video
 ***************************************************************************/
void Menu_Render() {

        FrameTimer++;

        // update the hole vb
        Xe_VB_Lock(g_pVideoDevice, vb, 0, MAX_VERTEX_COUNT * sizeof (DrawVerticeFormats), XE_LOCK_WRITE);
        Xe_VB_Unlock(g_pVideoDevice, vb);

        Xe_InvalidateState(g_pVideoDevice);

        Xe_Resolve(g_pVideoDevice);

        while (!Xe_IsVBlank(g_pVideoDevice));

        Xe_Sync(g_pVideoDevice);

        nb_vertices = 4096;
}

void SetRS() {
        Xe_SetBlendOp(g_pVideoDevice, XE_BLENDOP_ADD);
        Xe_SetSrcBlend(g_pVideoDevice, XE_BLEND_SRCALPHA);
        Xe_SetDestBlend(g_pVideoDevice, XE_BLEND_INVSRCALPHA);
        Xe_SetAlphaTestEnable(g_pVideoDevice, 1);

        Xe_SetCullMode(g_pVideoDevice, XE_CULL_NONE);
        Xe_SetStreamSource(g_pVideoDevice, 0, vb, nb_vertices, sizeof (DrawVerticeFormats));
}

void Draw() {
        SetRS();

        Xe_DrawPrimitive(g_pVideoDevice, XE_PRIMTYPE_RECTLIST, 0, 1);
        //nb_vertices += 4 * sizeof (DrawVerticeFormats);
        nb_vertices += 512; // fixe aligement
}

void UpdatesMatrices(f32 xpos, f32 ypos, f32 width, f32 height, f32 degrees, f32 scaleX, f32 scaleY) {
#define DegToRad(a)   ( (a) *  0.01745329252f )

        matrix4x4 m;
        matrix4x4 rotation;
        matrix4x4 scale;
        matrix4x4 translation;
        matrix4x4 WVP;

        matrixLoadIdentity(&WVP);
        matrixLoadIdentity(&m);
        matrixLoadIdentity(&rotation);
        matrixLoadIdentity(&scale);
        matrixLoadIdentity(&translation);

        matrixRotationZ(&rotation, DegToRad(degrees));
        matrixTranslation(&translation, xpos + width, ypos + height, 0);
        matrixScaling(&scale, scaleX, scaleY, 1.0f);

        //    // scale => rotate => translate
        matrixMultiply(&m, &scale, &rotation);
        matrixMultiply(&WVP, &m, &translation);

        Xe_SetVertexShaderConstantF(g_pVideoDevice, 0, (float*) &WVP, 4);
}

/****************************************************************************
 * Menu_DrawImg
 *
 * Draws the specified image on screen using GX
 ***************************************************************************/
void Menu_DrawImg(f32 xpos, f32 ypos, u16 width, u16 height, XenosSurface * data,
        f32 degrees, f32 scaleX, f32 scaleY, u8 alpha) {

        if (data == NULL)
                return;

        float x, y, w, h;

        x = (float) xpos;
        y = (float) ypos;
        w = (float) width;
        h = (float) height;

        x = (x / ((float) screenwidth / 2.f)) - 1.f; // 1280/2
        y = (y / ((float) screenheight / 2.f)) - 1.f; // 720/2

        w = (float) w / ((float) screenwidth);
        h = (float) h / ((float) screenheight);

        XeColor color;

        color.a = alpha;
        color.r = 0xFF;
        color.g = 0xFF;
        color.b = 0xFF;

        DrawVerticeFormats* Rect = (DrawVerticeFormats*) Xe_VB_Lock(g_pVideoDevice, vb, nb_vertices, 4 * sizeof (DrawVerticeFormats), XE_LOCK_WRITE);
        {
                // CreateVb(x,y,w*scaleX,h*scaleY,color.lcol,Rect);
                CreateVbQuad(w, h, color.lcol, Rect);
        }
        Xe_VB_Unlock(g_pVideoDevice, vb);

        Xe_SetTexture(g_pVideoDevice, 0, data);

        UpdatesMatrices(x, y, w, h, degrees, scaleX, scaleY);

        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_PIXEL, g_pPixelTexturedShader, 0);
        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_VERTEX, g_pVertexShader, 0);

        Draw();
}

/****************************************************************************
 * Menu_DrawRectangle
 *
 * Draws a rectangle at the specified coordinates using GX
 ***************************************************************************/
void Menu_DrawRectangle(f32 x, f32 y, f32 width, f32 height, GXColor color, u8 filled) {

        float w, h;

        x = (float) x;
        y = (float) y;
        w = (float) width;
        h = (float) height;

        x = (x / ((float) screenwidth / 2.f)) - 1.f; // 1280/2
        y = (y / ((float) screenheight / 2.f)) - 1.f; // 720/2

        w = (float) w / ((float) screenwidth);
        h = (float) h / ((float) screenheight);


        XeColor _color;

        _color.a = color.a;
        _color.r = color.r;
        _color.g = color.g;
        _color.b = color.b;

        DrawVerticeFormats* Rect = (DrawVerticeFormats*) Xe_VB_Lock(g_pVideoDevice, vb, nb_vertices, 4 * sizeof (DrawVerticeFormats), XE_LOCK_WRITE);
        {
                CreateVbQuad(w, h, _color.lcol, Rect);
                //CreateVb(x,y,w,h,_color.lcol,Rect);
        }
        Xe_VB_Unlock(g_pVideoDevice, vb);

        Xe_SetTexture(g_pVideoDevice, 0, NULL);

        UpdatesMatrices(x, y, w, h, 0, 1, 1);

        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_PIXEL, g_pPixelColoredShader, 0);
        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_VERTEX, g_pVertexShader, 0);

        Draw();
}

void Menu_T(XenosSurface * surf, f32 texWidth, f32 texHeight, int16_t screenX, int16_t screenY, GXColor color) {
        //return;
        float x, y, w, h;
        if (surf == NULL)
                return;

        x = (float) screenX;
        y = (float) screenY;
        w = (float) texWidth;
        h = (float) texHeight;

        x = (x / ((float) screenwidth / 2.f)) - 1.f; // 1280/2
        y = (y / ((float) screenheight / 2.f)) - 1.f; // 720/2

        w = (float) w / ((float) screenwidth);
        h = (float) h / ((float) screenheight);

        // Correct aspect ratio
        //    h = h * ((float) screenwidth/(float) screenheight);

        //    w = w/2;
        //    w = h/2;

        DrawVerticeFormats* Rect = (DrawVerticeFormats*) Xe_VB_Lock(g_pVideoDevice, vb, nb_vertices, 4 * sizeof (DrawVerticeFormats), XE_LOCK_WRITE);
        {
                XeColor _color;

                _color.a = color.a;
                _color.r = color.r;
                _color.g = color.g;
                _color.b = color.b;

                CreateVbText(0, 0, w, h, _color.lcol, Rect);
        }
        Xe_VB_Unlock(g_pVideoDevice, vb);

        Xe_SetTexture(g_pVideoDevice, 0, surf);

        UpdatesMatrices(x, y, w, h, 0, 1, 1);

        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_PIXEL, g_pPixelTexturedShader, 0);
        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_VERTEX, g_pVertexShader, 0);

        Draw();
}

/****************************************************************************
 * Update Video
 ***************************************************************************/
uint32 prevRenderedFrameCount = 0;
int fscale = 1;

enum {
        UvBottom = 0,
        UvTop,
        UvLeft,
        UvRight
};
/*
float bottom = 0.0f;
float top = 1.0f;
float left = 1.0f;
float right = 0.0f;
 */
float PsxScreenUv[4] = {0.f, 1.0f, 1.0f, 0.f};

float psxRealW = 1024.f;
float psxRealH = 512.f;

/****************************************************************************
 * Menu_DrawImg
 *
 * Draws the specified image on screen using GX
 ***************************************************************************/
static void DrawSnes(XenosSurface * data) {
        if (data == NULL)
                return;

        //        float x, y, w, h;
        //
        //        float scale = 1;
        //
        //        if (GCSettings.widescreen) {
        //                scale = 3.f / 4.f;
        //        }
        //
        //        w = (float) screenwidth * scale;
        //        h = (float) screenheight;
        //
        //        x = (((float) screenwidth - w) / 2.0f) + GCSettings.xshift;
        //        y = (float) -GCSettings.yshift;
        //
        //        x = (x / ((float) screenwidth / 2.f)) - 1.f; // 1280/2
        //        y = (y / ((float) screenheight / 2.f)) - 1.f; // 720/2
        //
        //        w = (float) w / ((float) screenwidth);
        //        h = (float) h / ((float) screenheight);
        //
        //        w *= GCSettings.zoomHor;
        //        h *= GCSettings.zoomVert;


        float x = -1.0f;
        float y = -1.0f;
        float w = 2.0f;
        float h = 2.0f;

        // Create Vb
        SnesVerticeFormats* Rect = (SnesVerticeFormats*) Xe_VB_Lock(g_pVideoDevice, snes_vb, 0, 4096, XE_LOCK_WRITE);
        {
                // CreateVb(x,y,w*scaleX,h*scaleY,color.lcol,Rect);
                //CreateVbSnes(w, h, x, y, 0xFFFFFFFF, Rect);
                // top left
                Rect[0].x = x;
                Rect[0].y = y + h;
                Rect[0].u = PsxScreenUv[UvBottom];
                Rect[0].v = PsxScreenUv[UvRight];
                Rect[0].color = 0;

                // bottom left
                Rect[1].x = x;
                Rect[1].y = y;
                Rect[1].u = PsxScreenUv[UvBottom];
                Rect[1].v = PsxScreenUv[UvLeft];
                Rect[1].color = 0;

                // top right
                Rect[2].x = x + w;
                Rect[2].y = y + h;
                Rect[2].u = PsxScreenUv[UvTop];
                Rect[2].v = PsxScreenUv[UvRight];
                Rect[2].color = 0;

                // top right
                Rect[3].x = x + w;
                Rect[3].y = y + h;
                Rect[3].u = PsxScreenUv[UvTop];
                ;
                Rect[3].v = PsxScreenUv[UvRight];
                Rect[3].color = 0;

                // bottom left
                Rect[4].x = x;
                Rect[4].y = y;
                Rect[4].u = PsxScreenUv[UvBottom];
                Rect[4].v = PsxScreenUv[UvLeft];
                Rect[4].color = 0;

                // bottom right
                Rect[5].x = x + w;
                Rect[5].y = y;
                Rect[5].u = PsxScreenUv[UvTop];
                Rect[5].v = PsxScreenUv[UvLeft];
                Rect[5].color = 0;

                int i = 0;
                for (i = 0; i < 6; i++) {
                        Rect[i].z = 0.0;
                        Rect[i].w = 1.0;
                }

        }
        Xe_VB_Unlock(g_pVideoDevice, snes_vb);

        // Begin draw
        Xe_InvalidateState(g_pVideoDevice);
        
        Xe_SetCullMode(g_pVideoDevice, XE_CULL_NONE);
        Xe_SetClearColor(g_pVideoDevice, 0);
        
        Xe_SetFillMode(g_pVideoDevice,XE_FILL_WIREFRAME,XE_FILL_WIREFRAME);

        // Refresh  texture cache
        Xe_Surface_LockRect(g_pVideoDevice, data, 0, 0, 0, 0, XE_LOCK_WRITE);
        Xe_Surface_Unlock(g_pVideoDevice, data);

        // Set Stream, shader, textures
        Xe_SetTexture(g_pVideoDevice, 0, data);
        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_PIXEL, g_pPixelSnesShader, 0);
        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_VERTEX, g_pVertexSnesShader, 0);
        Xe_SetStreamSource(g_pVideoDevice, 0, snes_vb, 0, sizeof (SnesVerticeFormats));

        // set texture size
        float settings_texture_size[2] = {data->width, data->height};
        Xe_SetVertexShaderConstantF(g_pVideoDevice, 0, settings_texture_size, 1);
        Xe_SetPixelShaderConstantF(g_pVideoDevice, 0, settings_texture_size, 1);

        // Draw
        Xe_DrawPrimitive(g_pVideoDevice, XE_PRIMTYPE_RECTLIST, 0, 1);

        // Dispaly
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

void update_video(int width, int height) {

        g_SnesSurface->width = width;
        g_SnesSurface->height = height;
        //  Menu_DrawImg(0, 0, screenwidth, screenheight, g_SnesSurface, 0, 1, 1, 0xFF);

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