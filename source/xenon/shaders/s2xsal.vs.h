#if 0
//
// Generated by 2.0.20675.0
//
//   fxc /Fh s2xsal.vs.h /Tvs_3_0 filters\super2xSal.cg /Emain_vertex
//    /VnVS2xSal
//
//
// Parameters:
//
//   struct
//   {
//       float2 video_size;
//       float2 texture_size;
//       float2 output_size;
//
//   } $IN;
//   
//   float4x4 $modelViewProj;
//
//
// Registers:
//
//   Name           Reg   Size
//   -------------- ----- ----
//   $modelViewProj c0       4
//   $IN            c4       2
//

// Shader type: vertex 

xvs_3_0
defconst $IN.video_size, float, vector, [1, 2], c4
defconst $IN.texture_size, float, vector, [1, 2], c5
defconst $IN.output_size, float, vector, [1, 2], cX
defconst $modelViewProj, float, matrix_columns, [4, 4], c0-3
config AutoSerialize=false
config AutoResource=false
config VsMaxReg=3
config VsResource=1
// VsExportCount=8

dcl_index r0.x
dcl_texcoord o0.xy
dcl_texcoord1 o1
dcl_texcoord2 o2
dcl_texcoord3 o3
dcl_texcoord4 o4
dcl_texcoord5 o5
dcl_texcoord6 o6
dcl_texcoord7 o7
dcl_texcoord8 o8

def c252, 0, 0, 0, 0
def c253, 0, 0, 0, 0
def c254, 0, 0, 0, 0
def c255, 2, 0, 0, 0


    exec
    vfetch r1.wxyz, r0.x, position
    vfetch r0.__xy, r0.x, texcoord
    alloc position
    exec
    mul r2, r1.x, c3.xwzy
  + rcp r1.x, c5.x
    mad r2, r1.w, c2.xwzy, r2
    mad r2, r1.z, c1.xzyw, r2.xzwy
    mad oPos, r1.y, c0, r2.xzyw
    alloc interpolators
    exec
    add r2.x, r1.x, r0.z
    add r2.w, -r1.x, r0.z
    mov r2.yz, r0.zzw
  + rcp r1.y, c5.y
    mad r3.xy, r1.yx, c255.x, r0.wz
    movs o4.x, r3.y
    add o2.x, r1.x, r0.z
    exec
    add o6.y, r1.y, r0.w
    add r0.xy, r1.xy, r0.zw
    add r3.zw, -r1.xxxy, r0.zzzw
  + movs r0._, r1.y
    mov o0.xy, r0.zw
    mov o8.xy00, r3.yx
    mov o4.yz, r2.zzw
  + adds_prev o4.w, r0.w
    exec
    mov o7.xz, r2.yx
    mov o7.yw, r3.x
    mov o2.yzw, r3.wwyw
    mov o6.xzw, r3.yzzx
    mov o3, r2.wzxz
    mov o5, r0.zyxy
    exece
    add o1.xyz, -r1.xyx, r0.zwz
  + movs o1.w, r0.w

// PDB hint 00000000-00000000-00000000

#endif

// This microcode is in native DWORD byte order.

const DWORD VS2xSal[] =
{
    0x102a1101, 0x000001b4, 0x000001a8, 0x00000000, 0x00000024, 0x00000104, 
    0x0000012c, 0x00000000, 0x00000000, 0x000000dc, 0x0000001c, 0x000000cf, 
    0xfffe0300, 0x00000002, 0x0000001c, 0x00000000, 0x000000c8, 0x00000044, 
    0x00020004, 0x00020000, 0x00000098, 0x00000000, 0x000000a8, 0x00020000, 
    0x00040000, 0x000000b8, 0x00000000, 0x24494e00, 0x76696465, 0x6f5f7369, 
    0x7a6500ab, 0x00010003, 0x00010002, 0x00010000, 0x00000000, 0x74657874, 
    0x7572655f, 0x73697a65, 0x006f7574, 0x7075745f, 0x73697a65, 0x00ababab, 
    0x00000048, 0x00000054, 0x00000064, 0x00000054, 0x00000071, 0x00000054, 
    0x00050000, 0x00010006, 0x00010003, 0x00000080, 0x246d6f64, 0x656c5669, 
    0x65775072, 0x6f6a00ab, 0x00030003, 0x00040004, 0x00010000, 0x00000000, 
    0x76735f33, 0x5f300032, 0x2e302e32, 0x30363735, 0x2e3000ab, 0x00000000, 
    0x00000001, 0x00000000, 0x00000000, 0x00000014, 0x00fc0010, 0x00000000, 
    0x00000000, 0x00000000, 0x00000000, 0x00000040, 0x00000168, 0x00810003, 
    0x00000000, 0x00000000, 0x00008929, 0x00000001, 0x00000002, 0x0000000d, 
    0x00000290, 0x00100004, 0x00205005, 0x00003050, 0x0001f151, 0x0002f252, 
    0x0004f353, 0x0005f454, 0x0007f555, 0x0008f656, 0x000af757, 0x000cf858, 
    0x00001013, 0x0000101c, 0x0000000f, 0x00001018, 0x0000101a, 0x0000000e, 
    0x00001015, 0x0000101b, 0x00000010, 0x00001019, 0x00000016, 0x00001017, 
    0x00001014, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
    0x00000000, 0x40000000, 0x00000000, 0x00000000, 0x00000000, 0x30052004, 
    0x00001200, 0xc2000000, 0x00004006, 0x00001200, 0xc4000000, 0x0000600a, 
    0x60101200, 0x12000000, 0x00006016, 0x101c1200, 0x22000000, 0x05f81000, 
    0x00000443, 0x00000000, 0x05f80000, 0x0000023f, 0x00000000, 0x4c1f0102, 
    0x006c886c, 0x81010305, 0xc80f0002, 0x001b8800, 0xab010202, 0xc80f0002, 
    0x00c63494, 0xab010102, 0xc80f803e, 0x00b10034, 0xab010002, 0xc8010002, 
    0x006cc600, 0xe0010000, 0xc8080002, 0x046cc600, 0xe0010000, 0x4c260102, 
    0x001616b1, 0xc2000005, 0xc8030003, 0x006d6cc7, 0xab01ff00, 0x14108004, 
    0x000000b1, 0xe2000003, 0xc8018002, 0x006cc600, 0xe0010000, 0xc8028006, 
    0x00b11b00, 0xe0010000, 0xc8030000, 0x00b01a00, 0xe0010000, 0x140c0003, 
    0x04ac06b1, 0xe0010001, 0xc8038000, 0x001a1a00, 0xe2000000, 0xc803c008, 
    0x006d6d00, 0xe2030300, 0x04868004, 0x0016161b, 0xe2020200, 0xc8058007, 
    0x006d6d00, 0xe2020200, 0xc80a8007, 0x006c6c00, 0xe2030300, 0xc80e8002, 
    0x003b3b00, 0xe2030300, 0xc80d8006, 0x00454500, 0xe2030300, 0xc80f8003, 
    0x00e7e700, 0xe2020200, 0xc80f8005, 0x00a2a200, 0xe2000000, 0x14878001, 
    0x0460ca1b, 0xe0010000, 0x00000000, 0x00000000, 0x00000000
};
