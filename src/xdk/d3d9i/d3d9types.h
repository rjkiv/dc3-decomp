#pragma once
#include "xdk/win_types.h"
#include "vectorintrinsics.h"

// Enums and very small struct definitions go here.

#pragma region Enums

typedef enum _D3DFORMAT {
    D3DFMT_DXT1 = 0x1a200152,
    D3DFMT_LIN_DXT1 = 0x1a200052,
    D3DFMT_DXT2 = 0x1a200153,
    D3DFMT_LIN_DXT2 = 0x1a200053,
    D3DFMT_DXT3 = 0x1a200153,
    D3DFMT_LIN_DXT3 = 0x1a200053,
    D3DFMT_DXT3A = 0x1a20017a,
    D3DFMT_LIN_DXT3A = 0x1a20007a,
    D3DFMT_DXT3A_1111 = 0x1a20017d,
    D3DFMT_LIN_DXT3A_1111 = 0x1a20007d,
    D3DFMT_DXT4 = 0x1a200154,
    D3DFMT_LIN_DXT4 = 0x1a200054,
    D3DFMT_DXT5 = 0x1a200154,
    D3DFMT_LIN_DXT5 = 0x1a200054,
    D3DFMT_DXT5A = 0x1a20017b,
    D3DFMT_LIN_DXT5A = 0x1a20007b,
    D3DFMT_DXN = 0x1a200171,
    D3DFMT_LIN_DXN = 0x1a200071,
    D3DFMT_CTX1 = 0x1a20017c,
    D3DFMT_LIN_CTX1 = 0x1a20007c,
    D3DFMT_A8 = 0x04900102,
    D3DFMT_LIN_A8 = 0x04900002,
    D3DFMT_L8 = 0x28000102,
    D3DFMT_LIN_L8 = 0x28000002,
    D3DFMT_R5G6B5 = 0x28280144,
    D3DFMT_LIN_R5G6B5 = 0x28280044,
    D3DFMT_R6G5B5 = 0x28280145,
    D3DFMT_LIN_R6G5B5 = 0x28280045,
    D3DFMT_L6V5U5 = 0x2a200b45,
    D3DFMT_LIN_L6V5U5 = 0x2a200a45,
    D3DFMT_X1R5G5B5 = 0x28280143,
    D3DFMT_LIN_X1R5G5B5 = 0x28280043,
    D3DFMT_A1R5G5B5 = 0x18280143,
    D3DFMT_LIN_A1R5G5B5 = 0x18280043,
    D3DFMT_A4R4G4B4 = 0x1828014f,
    D3DFMT_LIN_A4R4G4B4 = 0x1828004f,
    D3DFMT_X4R4G4B4 = 0x2828014f,
    D3DFMT_LIN_X4R4G4B4 = 0x2828004f,
    D3DFMT_Q4W4V4U4 = 0x1a20ab4f,
    D3DFMT_LIN_Q4W4V4U4 = 0x1a20aa4f,
    D3DFMT_A8L8 = 0x0800014a,
    D3DFMT_LIN_A8L8 = 0x0800004a,
    D3DFMT_G8R8 = 0x2d20014a,
    D3DFMT_LIN_G8R8 = 0x2d20004a,
    D3DFMT_V8U8 = 0x2d20ab4a,
    D3DFMT_LIN_V8U8 = 0x2d20aa4a,
    D3DFMT_D16 = 0x1a220158,
    D3DFMT_LIN_D16 = 0x1a220058,
    D3DFMT_L16 = 0x28000158,
    D3DFMT_LIN_L16 = 0x28000058,
    D3DFMT_R16F = 0x2da2ab5e,
    D3DFMT_LIN_R16F = 0x2da2aa5e,
    D3DFMT_R16F_EXPAND = 0x2da2ab5b,
    D3DFMT_LIN_R16F_EXPAND = 0x2da2aa5b,
    D3DFMT_UYVY = 0x1a20014c,
    D3DFMT_LIN_UYVY = 0x1a20004c,
    D3DFMT_LE_UYVY = 0x1a20010c,
    D3DFMT_LE_LIN_UYVY = 0x1a20000c,
    D3DFMT_G8R8_G8B8 = 0x1828014c,
    D3DFMT_LIN_G8R8_G8B8 = 0x1828004c,
    D3DFMT_R8G8_B8G8 = 0x1828014b,
    D3DFMT_LIN_R8G8_B8G8 = 0x1828004b,
    D3DFMT_YUY2 = 0x1a20014b,
    D3DFMT_LIN_YUY2 = 0x1a20004b,
    D3DFMT_LE_YUY2 = 0x1a20010b,
    D3DFMT_LE_LIN_YUY2 = 0x1a20000b,
    D3DFMT_A8R8G8B8 = 0x18280186,
    D3DFMT_LIN_A8R8G8B8 = 0x18280086,
    D3DFMT_X8R8G8B8 = 0x28280186,
    D3DFMT_LIN_X8R8G8B8 = 0x28280086,
    D3DFMT_A8B8G8R8 = 0x1a200186,
    D3DFMT_LIN_A8B8G8R8 = 0x1a200086,
    D3DFMT_X8B8G8R8 = 0x2a200186,
    D3DFMT_LIN_X8B8G8R8 = 0x2a200086,
    D3DFMT_X8L8V8U8 = 0x2a200b86,
    D3DFMT_LIN_X8L8V8U8 = 0x2a200a86,
    D3DFMT_Q8W8V8U8 = 0x1a20ab86,
    D3DFMT_LIN_Q8W8V8U8 = 0x1a20aa86,
    D3DFMT_A2R10G10B10 = 0x182801b6,
    D3DFMT_LIN_A2R10G10B10 = 0x182800b6,
    D3DFMT_X2R10G10B10 = 0x282801b6,
    D3DFMT_LIN_X2R10G10B10 = 0x282800b6,
    D3DFMT_A2B10G10R10 = 0x1a2001b6,
    D3DFMT_LIN_A2B10G10R10 = 0x1a2000b6,
    D3DFMT_A2W10V10U10 = 0x1a202bb6,
    D3DFMT_LIN_A2W10V10U10 = 0x1a202ab6,
    D3DFMT_A16L16 = 0x08000199,
    D3DFMT_LIN_A16L16 = 0x08000099,
    D3DFMT_G16R16 = 0x2d200199,
    D3DFMT_LIN_G16R16 = 0x2d200099,
    D3DFMT_V16U16 = 0x2d20ab99,
    D3DFMT_LIN_V16U16 = 0x2d20aa99,
    D3DFMT_R10G11B11 = 0x282801b7,
    D3DFMT_LIN_R10G11B11 = 0x282800b7,
    D3DFMT_R11G11B10 = 0x282801b8,
    D3DFMT_LIN_R11G11B10 = 0x282800b8,
    D3DFMT_W10V11U11 = 0x2a20abb7,
    D3DFMT_LIN_W10V11U11 = 0x2a20aab7,
    D3DFMT_W11V11U10 = 0x2a20abb8,
    D3DFMT_LIN_W11V11U10 = 0x2a20aab8,
    D3DFMT_G16R16F = 0x2d22ab9f,
    D3DFMT_LIN_G16R16F = 0x2d22aa9f,
    D3DFMT_G16R16F_EXPAND = 0x2d22ab9c,
    D3DFMT_LIN_G16R16F_EXPAND = 0x2d22aa9c,
    D3DFMT_L32 = 0x280001a1,
    D3DFMT_LIN_L32 = 0x280000a1,
    D3DFMT_R32F = 0x2da2aba4,
    D3DFMT_LIN_R32F = 0x2da2aaa4,
    D3DFMT_D24S8 = 0x2d200196,
    D3DFMT_LIN_D24S8 = 0x2d200096,
    D3DFMT_D24X8 = 0x2da00196,
    D3DFMT_LIN_D24X8 = 0x2da00096,
    D3DFMT_D24FS8 = 0x1a220197,
    D3DFMT_LIN_D24FS8 = 0x1a220097,
    D3DFMT_D32 = 0x1a2201a1,
    D3DFMT_LIN_D32 = 0x1a2200a1,
    D3DFMT_A16B16G16R16 = 0x1a20015a,
    D3DFMT_LIN_A16B16G16R16 = 0x1a20005a,
    D3DFMT_Q16W16V16U16 = 0x1a20ab5a,
    D3DFMT_LIN_Q16W16V16U16 = 0x1a20aa5a,
    D3DFMT_A16B16G16R16F = 0x1a22ab60,
    D3DFMT_LIN_A16B16G16R16F = 0x1a22aa60,
    D3DFMT_A16B16G16R16F_EXPAND = 0x1a22ab5d,
    D3DFMT_LIN_A16B16G16R16F_EXPAND = 0x1a22aa5d,
    D3DFMT_A32L32 = 0x080001a2,
    D3DFMT_LIN_A32L32 = 0x080000a2,
    D3DFMT_G32R32 = 0x2d2001a2,
    D3DFMT_LIN_G32R32 = 0x2d2000a2,
    D3DFMT_V32U32 = 0x2d20aba2,
    D3DFMT_LIN_V32U32 = 0x2d20aaa2,
    D3DFMT_G32R32F = 0x2d22aba5,
    D3DFMT_LIN_G32R32F = 0x2d22aaa5,
    D3DFMT_A32B32G32R32 = 0x1a2001a3,
    D3DFMT_LIN_A32B32G32R32 = 0x1a2000a3,
    D3DFMT_Q32W32V32U32 = 0x1a20aba3,
    D3DFMT_LIN_Q32W32V32U32 = 0x1a20aaa3,
    D3DFMT_A32B32G32R32F = 0x1a22aba6,
    D3DFMT_LIN_A32B32G32R32F = 0x1a22aaa6,
    D3DFMT_A2B10G10R10F_EDRAM = 0x1a2201bf,
    D3DFMT_G16R16_EDRAM = 0x2d20ab8d,
    D3DFMT_A16B16G16R16_EDRAM = 0x1a20ab55,
    D3DFMT_LE_X8R8G8B8 = 0x28280106,
    D3DFMT_LE_A8R8G8B8 = 0x18280106,
    D3DFMT_LE_X2R10G10B10 = 0x28280136,
    D3DFMT_LE_A2R10G10B10 = 0x18280136,
    D3DFMT_INDEX16 = 0x0001,
    D3DFMT_INDEX32 = 0x0006,
    D3DFMT_LE_INDEX16 = 0x0000,
    D3DFMT_LE_INDEX32 = 0x0004,
    D3DFMT_VERTEXDATA = 0x0008,
    D3DFMT_UNKNOWN = 0xff,
    D3DFMT_FORCE_DWORD = 0x7fffffff,
} D3DFORMAT;

typedef enum _D3DCMPFUNC {
    D3DCMP_NEVER = 0x0000,
    D3DCMP_LESS = 0x0001,
    D3DCMP_EQUAL = 0x0002,
    D3DCMP_LESSEQUAL = 0x0003,
    D3DCMP_GREATER = 0x0004,
    D3DCMP_NOTEQUAL = 0x0005,
    D3DCMP_GREATEREQUAL = 0x0006,
    D3DCMP_ALWAYS = 0x0007,
    D3DCMP_FORCE_DWORD = 0x7fffffff,
} D3DCMPFUNC, *LPD3DCMPFUNC;

/// https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dpool
typedef enum D3DPOOL {
    D3DPOOL_DEFAULT = 0,
    D3DPOOL_MANAGED = 1,
    D3DPOOL_SYSTEMMEM = 2,
    D3DPOOL_SCRATCH = 3,
    D3DPOOL_FORCE_DWORD = 0x7fffffff
} D3DPOOL, *LPD3DPOOL;

typedef enum _D3DRESOURCETYPE {
    D3DRTYPE_NONE = 0x0000,
    D3DRTYPE_VERTEXBUFFER = 0x0001,
    D3DRTYPE_INDEXBUFFER = 0x0002,
    D3DRTYPE_TEXTURE = 0x0003,
    D3DRTYPE_SURFACE = 0x0004,
    D3DRTYPE_VERTEXDECLARATION = 0x0005,
    D3DRTYPE_VERTEXSHADER = 0x0006,
    D3DRTYPE_PIXELSHADER = 0x0007,
    D3DRTYPE_CONSTANTBUFFER = 0x0008,
    D3DRTYPE_COMMANDBUFFER = 0x0009,
    D3DRTYPE_VOLUME = 0x0010,
    D3DRTYPE_VOLUMETEXTURE = 0x0011,
    D3DRTYPE_CUBETEXTURE = 0x0012,
    D3DRTYPE_ARRAYTEXTURE = 0x0013,
    D3DRTYPE_LINETEXTURE = 0x0014,
    D3DRTYPE_FORCE_DWORD = 0x7fffffff,
} D3DRESOURCETYPE, *LPD3DRESOURCETYPE;

typedef enum _D3DMULTISAMPLE_TYPE {
    D3DMULTISAMPLE_NONE = 0x0000,
    D3DMULTISAMPLE_2_SAMPLES = 0x0001,
    D3DMULTISAMPLE_4_SAMPLES = 0x0002,
    D3DMULTISAMPLE_FORCE_DWORD = 0x7fffffff,
} D3DMULTISAMPLE_TYPE, *LPD3DMULTISAMPLE_TYPE;

#pragma endregion
#pragma region D3D Structs

struct _D3DTAGCOLLECTION { /* Size=0x28 */
    /* 0x0000 */ UINT64 m_Mask[5];
};

/// https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dindexbuffer-desc
typedef struct _D3DINDEXBUFFER_DESC {
    D3DFORMAT Format;
    D3DRESOURCETYPE Type;
    DWORD Usage;
    D3DPOOL Pool; // pdb says this is a UINT (and not an enum)
    UINT Size;
} D3DINDEXBUFFER_DESC, *LPD3DINDEXBUFFER_DESC;

/// https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dvertexbuffer-desc
typedef struct _D3DVERTEXBUFFER_DESC {
    D3DFORMAT Format;
    D3DRESOURCETYPE Type;
    DWORD Usage;
    D3DPOOL Pool; // pdb says this is a UINT (and not an enum)
    UINT Size;
    DWORD FVF;
} D3DVERTEXBUFFER_DESC, *LPD3DVERTEXBUFFER_DESC;

typedef struct _D3DSURFACE_DESC { /* Size=0x20 */
    /* 0x0000 */ _D3DFORMAT Format;
    /* 0x0004 */ _D3DRESOURCETYPE Type;
    /* 0x0008 */ DWORD Usage;
    /* 0x000c */ DWORD Pool;
    /* 0x0010 */ _D3DMULTISAMPLE_TYPE MultiSampleType;
    /* 0x0014 */ DWORD MultiSampleQuality;
    /* 0x0018 */ DWORD Width;
    /* 0x001c */ DWORD Height;
} D3DSURFACE_DESC, *LPD3DSURFACE_DESC;

typedef enum _D3DHIZFUNC {
    D3DHIZFUNC_DEFAULT = 0x0000,
    D3DHIZFUNC_LESS_EQUAL = 0x0001,
    D3DHIZFUNC_GREATER_EQUAL = 0x0002,
    D3DHIZFUNC_FORCE_DWORD = 0x7fffffff,
} D3DHIZFUNC;

typedef struct _D3DSURFACE_PARAMETERS { /* Size=0x10 */
    /* 0x0000 */ UINT Base;
    /* 0x0004 */ UINT HierarchicalZBase;
    /* 0x0008 */ INT ColorExpBias;
    /* 0x000c */ D3DHIZFUNC HiZFunc;
} D3DSURFACE_PARAMETERS;

typedef struct _D3DVOLUME_DESC { /* Size=0x1c */
    /* 0x0000 */ _D3DFORMAT Format;
    /* 0x0004 */ _D3DRESOURCETYPE Type;
    /* 0x0008 */ UINT Usage;
    /* 0x000c */ UINT Pool;
    /* 0x0010 */ UINT Width;
    /* 0x0014 */ UINT Height;
    /* 0x0018 */ UINT Depth;
} D3DVOLUME_DESC;

typedef struct _D3DLOCKED_RECT { /* Size=0x8 */
    /* 0x0000 */ INT Pitch;
    /* 0x0004 */ VOID *pBits;
} D3DLOCKED_RECT;

struct tagRECT { /* Size=0x10 */
    /* 0x0000 */ INT left;
    /* 0x0004 */ INT top;
    /* 0x0008 */ INT right;
    /* 0x000c */ INT bottom;
};

struct tagPOINT { /* Size=0x8 */
    /* 0x0000 */ INT x;
    /* 0x0004 */ INT y;
};

typedef struct _D3DLOCKED_BOX { /* Size=0xc */
    /* 0x0000 */ INT RowPitch;
    /* 0x0004 */ INT SlicePitch;
    /* 0x0008 */ VOID *pBits;
} D3DLOCKED_BOX;

typedef struct _D3DBOX { /* Size=0x18 */
    /* 0x0000 */ UINT Left;
    /* 0x0004 */ UINT Top;
    /* 0x0008 */ UINT Right;
    /* 0x000c */ UINT Bottom;
    /* 0x0010 */ UINT Front;
    /* 0x0014 */ UINT Back;
} D3DBOX;

// https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3drect
typedef struct _D3DRECT { /* Size=0x10 */
    /** (x1, y1) = upper left corner of rectangle */
    /* 0x0000 */ LONG x1;
    /* 0x0004 */ LONG y1;
    /** (x2, y2) = lower right corner of rectangle */
    /* 0x0008 */ LONG x2;
    /* 0x000c */ LONG y2;
} D3DRECT;

typedef struct _D3DRING_BUFFER_PARAMETERS { /* Size=0x18 */
    /* 0x0000 */ DWORD Flags;
    /* 0x0004 */ DWORD PrimarySize;
    /* 0x0008 */ void *pPrimary;
    /* 0x000c */ DWORD SecondarySize;
    /* 0x0010 */ void *pSecondary;
    /* 0x0014 */ DWORD SegmentCount;
} D3DRING_BUFFER_PARAMETERS;

typedef enum _D3DCOLORSPACE {
    D3DCOLORSPACE_RGB = 0x0000,
    D3DCOLORSPACE_YCbCr601 = 0x0001,
    D3DCOLORSPACE_YCbCr709 = 0x0002,
    D3DCOLORSPACE_FORCE_DWORD = 0x7fffffff,
} D3DCOLORSPACE;

typedef struct _D3DVIDEO_SCALER_PARAMETERS { /* Size=0x1c */
    /* 0x0000 */ _D3DRECT ScalerSourceRect;
    /* 0x0010 */ DWORD ScaledOutputWidth;
    /* 0x0014 */ DWORD ScaledOutputHeight;
    /* 0x0018 */ DWORD FilterProfile;
} D3DVIDEO_SCALER_PARAMETERS;

typedef enum _D3DSWAPEFFECT {
    D3DSWAPEFFECT_DISCARD = 0x0001,
    D3DSWAPEFFECT_FLIP = 0x0002,
    D3DSWAPEFFECT_COPY = 0x0003,
    D3DSWAPEFFECT_FORCE_DWORD = 0x7fffffff,
} D3DSWAPEFFECT;

struct HWND__ { /* Size=0x4 */
    /* 0x0000 */ INT unused;
};

typedef struct _D3DPRESENT_PARAMETERS_ { /* Size=0x7c */
    /* 0x0000 */ UINT BackBufferWidth;
    /* 0x0004 */ UINT BackBufferHeight;
    /* 0x0008 */ _D3DFORMAT BackBufferFormat;
    /* 0x000c */ UINT BackBufferCount;
    /* 0x0010 */ _D3DMULTISAMPLE_TYPE MultiSampleType;
    /* 0x0014 */ DWORD MultiSampleQuality;
    /* 0x0018 */ _D3DSWAPEFFECT SwapEffect;
    /* 0x001c */ HWND__ *hDeviceWindow;
    /* 0x0020 */ BOOL Windowed;
    /* 0x0024 */ BOOL EnableAutoDepthStencil;
    /* 0x0028 */ _D3DFORMAT AutoDepthStencilFormat;
    /* 0x002c */ DWORD Flags;
    /* 0x0030 */ UINT FullScreen_RefreshRateInHz;
    /* 0x0034 */ UINT PresentationInterval;
    /* 0x0038 */ BOOL DisableAutoBackBuffer;
    /* 0x003c */ BOOL DisableAutoFrontBuffer;
    /* 0x0040 */ _D3DFORMAT FrontBufferFormat;
    /* 0x0044 */ _D3DCOLORSPACE FrontBufferColorSpace;
    /* 0x0048 */ _D3DRING_BUFFER_PARAMETERS RingBufferParameters;
    /* 0x0060 */ _D3DVIDEO_SCALER_PARAMETERS VideoScalerParameters;
} D3DPRESENT_PARAMETERS;

typedef struct _D3DPERFCOUNTER_VALUES { /* Size=0x1e0 */
    /* 0x0000 */ ULARGE_INTEGER CP[1];
    /* 0x0008 */ ULARGE_INTEGER RBBM[2];
    /* 0x0018 */ ULARGE_INTEGER SQ[4];
    /* 0x0038 */ ULARGE_INTEGER VGT[4];
    /* 0x0058 */ ULARGE_INTEGER VC[4];
    /* 0x0078 */ ULARGE_INTEGER PA_SU[4];
    /* 0x0098 */ ULARGE_INTEGER PA_SC[4];
    /* 0x00b8 */ ULARGE_INTEGER HZ[2];
    /* 0x00c8 */ ULARGE_INTEGER TCR[2];
    /* 0x00d8 */ ULARGE_INTEGER TCM[2];
    /* 0x00e8 */ ULARGE_INTEGER TCF[12];
    /* 0x0148 */ ULARGE_INTEGER TP0[2];
    /* 0x0158 */ ULARGE_INTEGER TP1[2];
    /* 0x0168 */ ULARGE_INTEGER TP2[2];
    /* 0x0178 */ ULARGE_INTEGER TP3[2];
    /* 0x0188 */ ULARGE_INTEGER SX[1];
    /* 0x0190 */ ULARGE_INTEGER BC[4];
    /* 0x01b0 */ ULARGE_INTEGER MC0[1];
    /* 0x01b8 */ ULARGE_INTEGER MC1[1];
    /* 0x01c0 */ ULARGE_INTEGER MH[3];
    /* 0x01d8 */ ULARGE_INTEGER BIF[1];
} D3DPERFCOUNTER_VALUES;

typedef struct _D3DMIPTAIL_DESC { /* Size=0x24 */
    /* 0x0000 */ _D3DFORMAT Format;
    /* 0x0004 */ _D3DRESOURCETYPE Type;
    /* 0x0008 */ UINT Usage;
    /* 0x000c */ UINT Pool;
    /* 0x0010 */ UINT Width;
    /* 0x0014 */ UINT Height;
    /* 0x0018 */ UINT Depth;
    /* 0x001c */ UINT BaseLevel;
    /* 0x0020 */ UINT Size;
} D3DMIPTAIL_DESC;

typedef struct _D3DLOCKED_TAIL { /* Size=0xc */
    /* 0x0000 */ INT RowPitch;
    /* 0x0004 */ INT SlicePitch;
    /* 0x0008 */ VOID *pBits;
} D3DLOCKED_TAIL;

typedef enum _D3DCUBEMAP_FACES {
    D3DCUBEMAP_FACE_POSITIVE_X = 0x0000,
    D3DCUBEMAP_FACE_NEGATIVE_X = 0x0001,
    D3DCUBEMAP_FACE_POSITIVE_Y = 0x0002,
    D3DCUBEMAP_FACE_NEGATIVE_Y = 0x0003,
    D3DCUBEMAP_FACE_POSITIVE_Z = 0x0004,
    D3DCUBEMAP_FACE_NEGATIVE_Z = 0x0005,
    D3DCUBEMAP_FACE_FORCE_DWORD = 0x7fffffff,
} D3DCUBEMAP_FACES;

typedef enum _D3DQUERYTYPE {
    D3DQUERYTYPE_VCACHE = 0x0004,
    D3DQUERYTYPE_RESOURCEMANAGER = 0x0005,
    D3DQUERYTYPE_VERTEXSTATS = 0x0006,
    D3DQUERYTYPE_EVENT = 0x0008,
    D3DQUERYTYPE_OCCLUSION = 0x0009,
    D3DQUERYTYPE_SCREENEXTENT = 0x000a,
} D3DQUERYTYPE;

typedef enum _D3DDECLTYPE {
    D3DDECLTYPE_FLOAT1 = 0x002c83a4,
    D3DDECLTYPE_FLOAT2 = 0x002c23a5,
    D3DDECLTYPE_FLOAT3 = 0x002a23b9,
    D3DDECLTYPE_FLOAT4 = 0x001a23a6,
    D3DDECLTYPE_INT1 = 0x002c83a1,
    D3DDECLTYPE_INT2 = 0x002c23a2,
    D3DDECLTYPE_INT4 = 0x001a23a3,
    D3DDECLTYPE_UINT1 = 0x002c82a1,
    D3DDECLTYPE_UINT2 = 0x002c22a2,
    D3DDECLTYPE_UINT4 = 0x001a22a3,
    D3DDECLTYPE_INT1N = 0x002c81a1,
    D3DDECLTYPE_INT2N = 0x002c21a2,
    D3DDECLTYPE_INT4N = 0x001a21a3,
    D3DDECLTYPE_UINT1N = 0x002c80a1,
    D3DDECLTYPE_UINT2N = 0x002c20a2,
    D3DDECLTYPE_UINT4N = 0x001a20a3,
    D3DDECLTYPE_D3DCOLOR = 0x00182886,
    D3DDECLTYPE_UBYTE4 = 0x001a2286,
    D3DDECLTYPE_BYTE4 = 0x001a2386,
    D3DDECLTYPE_UBYTE4N = 0x001a2086,
    D3DDECLTYPE_BYTE4N = 0x001a2186,
    D3DDECLTYPE_SHORT2 = 0x002c2359,
    D3DDECLTYPE_SHORT4 = 0x001a235a,
    D3DDECLTYPE_USHORT2 = 0x002c2259,
    D3DDECLTYPE_USHORT4 = 0x001a225a,
    D3DDECLTYPE_SHORT2N = 0x002c2159,
    D3DDECLTYPE_SHORT4N = 0x001a215a,
    D3DDECLTYPE_USHORT2N = 0x002c2059,
    D3DDECLTYPE_USHORT4N = 0x001a205a,
    D3DDECLTYPE_UDEC3 = 0x002a2287,
    D3DDECLTYPE_DEC3 = 0x002a2387,
    D3DDECLTYPE_UDEC3N = 0x002a2087,
    D3DDECLTYPE_DEC3N = 0x002a2187,
    D3DDECLTYPE_UDEC4 = 0x001a2287,
    D3DDECLTYPE_DEC4 = 0x001a2387,
    D3DDECLTYPE_UDEC4N = 0x001a2087,
    D3DDECLTYPE_DEC4N = 0x001a2187,
    D3DDECLTYPE_UHEND3 = 0x002a2290,
    D3DDECLTYPE_HEND3 = 0x002a2390,
    D3DDECLTYPE_UHEND3N = 0x002a2090,
    D3DDECLTYPE_HEND3N = 0x002a2190,
    D3DDECLTYPE_UDHEN3 = 0x002a2291,
    D3DDECLTYPE_DHEN3 = 0x002a2391,
    D3DDECLTYPE_UDHEN3N = 0x002a2091,
    D3DDECLTYPE_DHEN3N = 0x002a2191,
    D3DDECLTYPE_FLOAT16_2 = 0x002c235f,
    D3DDECLTYPE_FLOAT16_4 = 0x001a2360,
    D3DDECLTYPE_UNUSED = 0xff,
} D3DDECLTYPE;

typedef enum _D3DDECLMETHOD {
    D3DDECLMETHOD_DEFAULT = 0x0000,
    D3DDECLMETHOD_PARTIALU = 0x0001,
    D3DDECLMETHOD_PARTIALV = 0x0002,
    D3DDECLMETHOD_CROSSUV = 0x0003,
    D3DDECLMETHOD_UV = 0x0004,
    D3DDECLMETHOD_LOOKUP = 0x0005,
    D3DDECLMETHOD_LOOKUPPRESAMPLED = 0x0006,
} D3DDECLMETHOD;

typedef enum _D3DDECLUSAGE {
    D3DDECLUSAGE_POSITION = 0x0000,
    D3DDECLUSAGE_BLENDWEIGHT = 0x0001,
    D3DDECLUSAGE_BLENDINDICES = 0x0002,
    D3DDECLUSAGE_NORMAL = 0x0003,
    D3DDECLUSAGE_PSIZE = 0x0004,
    D3DDECLUSAGE_TEXCOORD = 0x0005,
    D3DDECLUSAGE_TANGENT = 0x0006,
    D3DDECLUSAGE_BINORMAL = 0x0007,
    D3DDECLUSAGE_TESSFACTOR = 0x0008,
    D3DDECLUSAGE_COLOR = 0x000a,
    D3DDECLUSAGE_FOG = 0x000b,
    D3DDECLUSAGE_DEPTH = 0x000c,
    D3DDECLUSAGE_SAMPLE = 0x000d,
} D3DDECLUSAGE;

typedef struct _D3DVERTEXELEMENT9 { /* Size=0xc */
    /* 0x0000 */ WORD Stream;
    /* 0x0002 */ WORD Offset;
    /* 0x0004 */ DWORD Type; // defined by D3DDECLTYPE
    /* 0x0008 */ BYTE Method; // defined by D3DDECLMETHOD
    /* 0x0009 */ BYTE Usage; // defined by D3DDECLUSAGE
    /* 0x000a */ BYTE UsageIndex;
} D3DVERTEXELEMENT9;

typedef enum _D3DPRIMITIVETYPE {
    D3DPT_POINTLIST = 0x0001,
    D3DPT_LINELIST = 0x0002,
    D3DPT_LINESTRIP = 0x0003,
    D3DPT_TRIANGLELIST = 0x0004,
    D3DPT_TRIANGLEFAN = 0x0005,
    D3DPT_TRIANGLESTRIP = 0x0006,
    D3DPT_RECTLIST = 0x0008,
    D3DPT_QUADLIST = 0x000d,
    D3DPT_FORCE_DWORD = 0x7fffffff,
} D3DPRIMITIVETYPE;

// https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dviewport9
/** Defines the window dimensions of a render-target surface onto which a 3D volume
 * projects. */
typedef struct _D3DVIEWPORT9 { /* Size=0x18 */
    /* 0x0000 */ DWORD X;
    /* 0x0004 */ DWORD Y;
    /* 0x0008 */ DWORD Width;
    /* 0x000c */ DWORD Height;
    /* 0x0010 */ float MinZ;
    /* 0x0014 */ float MaxZ;
} D3DVIEWPORT9;

#define D3DDECL_END() { -1, 0, -1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 }

typedef struct _D3DSWAP_STATUS { /* Size=0xc */
    /* 0x0000 */ UINT Flags;
    /* 0x0004 */ UINT Swap;
    /* 0x0008 */ UINT EnqueuedCount;
} D3DSWAP_STATUS;

typedef struct _D3DGAMMARAMP { /* Size=0x600 */
    /* 0x0000 */ UINT16 red[256];
    /* 0x0200 */ UINT16 green[256];
    /* 0x0400 */ UINT16 blue[256];
} D3DGAMMARAMP;

typedef struct _D3DRESOLVE_PARAMETERS { /* Size=0xc */
    /* 0x0000 */ D3DFORMAT ColorFormat;
    /* 0x0004 */ INT ColorExpBias;
    /* 0x0008 */ D3DFORMAT DepthFormat;
} D3DRESOLVE_PARAMETERS;

typedef struct _D3DPOINT { /* Size=0x8 */
    /* 0x0000 */ INT x;
    /* 0x0004 */ INT y;
} D3DPOINT;

typedef struct _D3DMATRIX { /* Size=0x40 */
    union {
        struct {
            /* 0x0000 */ float _11;
            /* 0x0004 */ float _12;
            /* 0x0008 */ float _13;
            /* 0x000c */ float _14;
            /* 0x0010 */ float _21;
            /* 0x0014 */ float _22;
            /* 0x0018 */ float _23;
            /* 0x001c */ float _24;
            /* 0x0020 */ float _31;
            /* 0x0024 */ float _32;
            /* 0x0028 */ float _33;
            /* 0x002c */ float _34;
            /* 0x0030 */ float _41;
            /* 0x0034 */ float _42;
            /* 0x0038 */ float _43;
            /* 0x003c */ float _44;
        };
        /* 0x0000 */ float m[4][4];
    };
} D3DMATRIX;

#pragma endregion
#pragma region GPU Structs

union GPUVERTEX_FETCH_CONSTANT { /* Size=0x8 */
    struct {
        /* 0x0000 */ UINT Type : 2; /* BitPos=0 */
        /* 0x0000 */ UINT BaseAddress : 30; /* BitPos=2 */
        /* 0x0000 */ UINT : 0; /* BitPos=32 */
        /* 0x0004 */ UINT Endian : 2; /* BitPos=0 */
        /* 0x0004 */ UINT Size : 24; /* BitPos=2 */
        /* 0x0004 */ UINT AddressClamp : 1; /* BitPos=26 */
        /* 0x0004 */ UINT : 1; /* BitPos=27 */
        /* 0x0004 */ UINT RequestSize : 2; /* BitPos=28 */
        /* 0x0004 */ UINT ClampDisable : 2; /* BitPos=30 */
    };
    /* 0x0000 */ UINT dword[2];
};

struct GPUTEXTURESIZE_2D { /* Size=0x4 */
    /* 0x0000 */ DWORD Width : 13; /* BitPos=0 */
    /* 0x0000 */ DWORD Height : 13; /* BitPos=13 */
};

struct GPUTEXTURESIZE_1D { /* Size=0x4 */
    /* 0x0000 */ DWORD Width : 24; /* BitPos=0 */
};

struct GPUTEXTURESIZE_STACK { /* Size=0x4 */
    /* 0x0000 */ DWORD Width : 13; /* BitPos=0 */
    /* 0x0000 */ DWORD Height : 13; /* BitPos=13 */
    /* 0x0000 */ DWORD Depth : 6; /* BitPos=26 */
};

struct GPUTEXTURESIZE_3D { /* Size=0x4 */
    /* 0x0000 */ DWORD Width : 11; /* BitPos=0 */
    /* 0x0000 */ DWORD Height : 11; /* BitPos=11 */
    /* 0x0000 */ DWORD Depth : 10; /* BitPos=22 */
};

union GPUTEXTURE_FETCH_CONSTANT { /* Size=0x18 */
    struct {
        /* 0x0000 */ DWORD Type : 2; /* BitPos=0 */
        /* 0x0000 */ DWORD SignX : 2; /* BitPos=2 */
        /* 0x0000 */ DWORD SignY : 2; /* BitPos=4 */
        /* 0x0000 */ DWORD SignZ : 2; /* BitPos=6 */
        /* 0x0000 */ DWORD SignW : 2; /* BitPos=8 */
        /* 0x0000 */ DWORD ClampX : 3; /* BitPos=10 */
        /* 0x0000 */ DWORD ClampY : 3; /* BitPos=13 */
        /* 0x0000 */ DWORD ClampZ : 3; /* BitPos=16 */
        /* 0x0000 */ UINT : 3; /* BitPos=19 */
        /* 0x0000 */ DWORD Pitch : 9; /* BitPos=22 */
        /* 0x0000 */ DWORD Tiled : 1; /* BitPos=31 */
        /* 0x0000 */ UINT : 0; /* BitPos=32 */
        /* 0x0004 */ DWORD DataFormat : 6; /* BitPos=0 */
        /* 0x0004 */ DWORD Endian : 2; /* BitPos=6 */
        /* 0x0004 */ DWORD RequestSize : 2; /* BitPos=8 */
        /* 0x0004 */ DWORD Stacked : 1; /* BitPos=10 */
        /* 0x0004 */ DWORD ClampPolicy : 1; /* BitPos=11 */
        /* 0x0004 */ DWORD BaseAddress : 20; /* BitPos=12 */
        /* 0x0008 */ union { /* Size=0x4 */
            /* 0x0000 */ GPUTEXTURESIZE_1D OneD;
            /* 0x0000 */ GPUTEXTURESIZE_2D TwoD;
            /* 0x0000 */ GPUTEXTURESIZE_3D ThreeD;
            /* 0x0000 */ GPUTEXTURESIZE_STACK Stack;
        } Size;
        /* 0x000c */ DWORD NumFormat : 1; /* BitPos=0 */
        /* 0x000c */ DWORD SwizzleX : 3; /* BitPos=1 */
        /* 0x000c */ DWORD SwizzleY : 3; /* BitPos=4 */
        /* 0x000c */ DWORD SwizzleZ : 3; /* BitPos=7 */
        /* 0x000c */ DWORD SwizzleW : 3; /* BitPos=10 */
        /* 0x000c */ INT ExpAdjust : 6; /* BitPos=13 */
        /* 0x000c */ DWORD MagFilter : 2; /* BitPos=19 */
        /* 0x000c */ DWORD MinFilter : 2; /* BitPos=21 */
        /* 0x000c */ DWORD MipFilter : 2; /* BitPos=23 */
        /* 0x000c */ DWORD AnisoFilter : 3; /* BitPos=25 */
        /* 0x000c */ UINT : 3; /* BitPos=28 */
        /* 0x000c */ DWORD BorderSize : 1; /* BitPos=31 */
        /* 0x000c */ UINT : 0; /* BitPos=32 */
        /* 0x0010 */ DWORD VolMagFilter : 1; /* BitPos=0 */
        /* 0x0010 */ DWORD VolMinFilter : 1; /* BitPos=1 */
        /* 0x0010 */ DWORD MinMipLevel : 4; /* BitPos=2 */
        /* 0x0010 */ DWORD MaxMipLevel : 4; /* BitPos=6 */
        /* 0x0010 */ DWORD MagAnisoWalk : 1; /* BitPos=10 */
        /* 0x0010 */ DWORD MinAnisoWalk : 1; /* BitPos=11 */
        /* 0x0010 */ INT LODBias : 10; /* BitPos=12 */
        /* 0x0010 */ INT GradExpAdjustH : 5; /* BitPos=22 */
        /* 0x0010 */ INT GradExpAdjustV : 5; /* BitPos=27 */
        /* 0x0010 */ INT : 0; /* BitPos=32 */
        /* 0x0014 */ DWORD BorderColor : 2; /* BitPos=0 */
        /* 0x0014 */ DWORD ForceBCWToMax : 1; /* BitPos=2 */
        /* 0x0014 */ DWORD TriClamp : 2; /* BitPos=3 */
        /* 0x0014 */ INT AnisoBias : 4; /* BitPos=5 */
        /* 0x0014 */ DWORD Dimension : 2; /* BitPos=9 */
        /* 0x0014 */ DWORD PackedMips : 1; /* BitPos=11 */
        /* 0x0014 */ DWORD MipAddress : 20; /* BitPos=12 */
    };
    /* 0x0000 */ DWORD dword[6];
};

union GPUFETCH_CONSTANT { /* Size=0x18 */
    /* 0x0000 */ GPUTEXTURE_FETCH_CONSTANT Texture;
    /* 0x0000 */ GPUVERTEX_FETCH_CONSTANT Vertex[3];
};

union GPU_POINT { /* Size=0x4 */
    struct {
        /* 0x0000 */ INT X : 15; /* BitPos=0 */
        /* 0x0000 */ INT : 1; /* BitPos=15 */
        /* 0x0000 */ INT Y : 15; /* BitPos=16 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_COLORINFO { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT ColorBase : 12; /* BitPos=0 */
        /* 0x0000 */ UINT : 4; /* BitPos=12 */
        /* 0x0000 */ UINT ColorFormat : 4; /* BitPos=16 */
        /* 0x0000 */ UINT ColorExpBias : 6; /* BitPos=20 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_DEPTHINFO { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT DepthBase : 12; /* BitPos=0 */
        /* 0x0000 */ UINT : 4; /* BitPos=12 */
        /* 0x0000 */ UINT DepthFormat : 1; /* BitPos=16 */
        /* 0x0000 */ UINT DisableHZClamp : 1; /* BitPos=17 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_SURFACEINFO { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT SurfacePitch : 14; /* BitPos=0 */
        /* 0x0000 */ UINT : 2; /* BitPos=14 */
        /* 0x0000 */ UINT MsaaSamples : 2; /* BitPos=16 */
        /* 0x0000 */ UINT HiZPitch : 14; /* BitPos=18 */
    };
    /* 0x0000 */ UINT dword;
};

struct GPU_DESTINATIONPACKET { /* Size=0x40 */
    /* 0x0000 */ GPU_SURFACEINFO SurfaceInfo;
    /* 0x0004 */ GPU_COLORINFO Color0Info;
    /* 0x0008 */ GPU_DEPTHINFO DepthInfo;
    /* 0x000c */ GPU_COLORINFO Color1Info;
    /* 0x0010 */ GPU_COLORINFO Color2Info;
    /* 0x0014 */ GPU_COLORINFO Color3Info;
    /* 0x0018 */ UINT CoherDestBase0;
    /* 0x001c */ UINT CoherDestBase1;
    /* 0x0020 */ UINT CoherDestBase2;
    /* 0x0024 */ UINT CoherDestBase3;
    /* 0x0028 */ UINT CoherDestBase4;
    /* 0x002c */ UINT CoherDestBase5;
    /* 0x0030 */ UINT CoherDestBase6;
    /* 0x0034 */ UINT CoherDestBase7;
    /* 0x0038 */ GPU_POINT ScreenScissorTL;
    /* 0x003c */ GPU_POINT ScreenScissorBR;
};

struct GPU_WINDOWPACKET { /* Size=0xc */
    /* 0x0000 */ GPU_POINT WindowOffset;
    /* 0x0004 */ GPU_POINT WindowScissorTL;
    /* 0x0008 */ GPU_POINT WindowScissorBR;
};

union GPU_COLORMASK { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT Write0 : 4; /* BitPos=0 */
        /* 0x0000 */ UINT Write1 : 4; /* BitPos=4 */
        /* 0x0000 */ UINT Write2 : 4; /* BitPos=8 */
        /* 0x0000 */ UINT Write3 : 4; /* BitPos=12 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_STENCILREFMASK { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT Ref : 8; /* BitPos=0 */
        /* 0x0000 */ UINT Mask : 8; /* BitPos=8 */
        /* 0x0000 */ UINT WriteMask : 8; /* BitPos=16 */
    };
    /* 0x0000 */ UINT dword;
};

struct GPU_VALUESPACKET { /* Size=0x54 */
    /* 0x0000 */ UINT MaxVtxIndx;
    /* 0x0004 */ UINT MinVtxIndx;
    /* 0x0008 */ UINT IndxOffset;
    /* 0x000c */ UINT MultiPrimIbResetIndx;
    /* 0x0010 */ GPU_COLORMASK ColorMask;
    /* 0x0014 */ float BlendRed;
    /* 0x0018 */ float BlendGreen;
    /* 0x001c */ float BlendBlue;
    /* 0x0020 */ float BlendAlpha;
    /* 0x0024 */ UINT Unused[3];
    /* 0x0030 */ GPU_STENCILREFMASK StencilRefMaskBF;
    /* 0x0034 */ GPU_STENCILREFMASK StencilRefMask;
    /* 0x0038 */ float AlphaRef;
    /* 0x003c */ float VportXScale;
    /* 0x0040 */ float VportXOffset;
    /* 0x0044 */ float VportYScale;
    /* 0x0048 */ float VportYOffset;
    /* 0x004c */ float VportZScale;
    /* 0x0050 */ float VportZOffset;
};

union GPU_CONTEXTMISC { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT InstPredOptimize : 1; /* BitPos=0 */
        /* 0x0000 */ UINT OutputScreenXY : 1; /* BitPos=1 */
        /* 0x0000 */ UINT SampleControl : 2; /* BitPos=2 */
        /* 0x0000 */ UINT : 4; /* BitPos=4 */
        /* 0x0000 */ UINT ParamGenPos : 8; /* BitPos=8 */
        /* 0x0000 */ UINT PerfCounterRef : 1; /* BitPos=16 */
        /* 0x0000 */ UINT YieldOptimize : 1; /* BitPos=17 */
        /* 0x0000 */ UINT TxCacheSelect : 1; /* BitPos=18 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_PROGRAMCONTROL { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT VsMaxReg : 6; /* BitPos=0 */
        /* 0x0000 */ UINT : 2; /* BitPos=6 */
        /* 0x0000 */ UINT PsMaxReg : 6; /* BitPos=8 */
        /* 0x0000 */ UINT : 2; /* BitPos=14 */
        /* 0x0000 */ UINT VsResource : 1; /* BitPos=16 */
        /* 0x0000 */ UINT PsResource : 1; /* BitPos=17 */
        /* 0x0000 */ UINT ParamGen : 1; /* BitPos=18 */
        /* 0x0000 */ UINT GenIndexPix : 1; /* BitPos=19 */
        /* 0x0000 */ UINT VsExportCount : 4; /* BitPos=20 */
        /* 0x0000 */ UINT VsExportMode : 3; /* BitPos=24 */
        /* 0x0000 */ UINT PsExportZ : 1; /* BitPos=27 */
        /* 0x0000 */ UINT PsExportColorCount : 3; /* BitPos=28 */
        /* 0x0000 */ UINT GenIndexVtx : 1; /* BitPos=31 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_INTERPOLATORCONTROL { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT ParamShade : 16; /* BitPos=0 */
        /* 0x0000 */ UINT SamplingPattern : 16; /* BitPos=16 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_WRAPPING1 { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT Wrap0 : 4; /* BitPos=0 */
        /* 0x0000 */ UINT Wrap1 : 4; /* BitPos=4 */
        /* 0x0000 */ UINT Wrap2 : 4; /* BitPos=8 */
        /* 0x0000 */ UINT Wrap3 : 4; /* BitPos=12 */
        /* 0x0000 */ UINT Wrap4 : 4; /* BitPos=16 */
        /* 0x0000 */ UINT Wrap5 : 4; /* BitPos=20 */
        /* 0x0000 */ UINT Wrap6 : 4; /* BitPos=24 */
        /* 0x0000 */ UINT Wrap7 : 4; /* BitPos=28 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_WRAPPING0 { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT Wrap8 : 4; /* BitPos=0 */
        /* 0x0000 */ UINT Wrap9 : 4; /* BitPos=4 */
        /* 0x0000 */ UINT Wrap10 : 4; /* BitPos=8 */
        /* 0x0000 */ UINT Wrap11 : 4; /* BitPos=12 */
        /* 0x0000 */ UINT Wrap12 : 4; /* BitPos=16 */
        /* 0x0000 */ UINT Wrap13 : 4; /* BitPos=20 */
        /* 0x0000 */ UINT Wrap14 : 4; /* BitPos=24 */
        /* 0x0000 */ UINT Wrap15 : 4; /* BitPos=28 */
    };
    /* 0x0000 */ UINT dword;
};

struct GPU_PROGRAMPACKET { /* Size=0x14 */
    /* 0x0000 */ GPU_PROGRAMCONTROL ProgramControl;
    /* 0x0004 */ GPU_CONTEXTMISC ContextMisc;
    /* 0x0008 */ GPU_INTERPOLATORCONTROL InterpolatorControl;
    /* 0x000c */ GPU_WRAPPING0 Wrapping0;
    /* 0x0010 */ GPU_WRAPPING1 Wrapping1;
};

union GPU_MODECONTROL { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT CullMode : 3; /* BitPos=0 */
        /* 0x0000 */ UINT PolyMode : 2; /* BitPos=3 */
        /* 0x0000 */ UINT PolyModeFrontPType : 3; /* BitPos=5 */
        /* 0x0000 */ UINT PolyModeBackPType : 3; /* BitPos=8 */
        /* 0x0000 */ UINT PolyOffsetFrontEnable : 1; /* BitPos=11 */
        /* 0x0000 */ UINT PolyOffsetBackEnable : 1; /* BitPos=12 */
        /* 0x0000 */ UINT PolyOffsetParaEnable : 1; /* BitPos=13 */
        /* 0x0000 */ UINT : 1; /* BitPos=14 */
        /* 0x0000 */ UINT MsaaEnable : 1; /* BitPos=15 */
        /* 0x0000 */ UINT VtxWindowOffsetEnable : 1; /* BitPos=16 */
        /* 0x0000 */ UINT : 2; /* BitPos=17 */
        /* 0x0000 */ UINT ProvokingVtxLast : 1; /* BitPos=19 */
        /* 0x0000 */ UINT PerspCorrDisable : 1; /* BitPos=20 */
        /* 0x0000 */ UINT MultiPrimIbEnable : 1; /* BitPos=21 */
        /* 0x0000 */ UINT : 1; /* BitPos=22 */
        /* 0x0000 */ UINT QuadOrderEnable : 1; /* BitPos=23 */
        /* 0x0000 */ UINT ScOneQuadPerClock : 1; /* BitPos=24 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_COLORCONTROL { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT AlphaFunc : 3; /* BitPos=0 */
        /* 0x0000 */ UINT AlphaTestEnable : 1; /* BitPos=3 */
        /* 0x0000 */ UINT AlphaToMaskEnable : 1; /* BitPos=4 */
        /* 0x0000 */ UINT : 19; /* BitPos=5 */
        /* 0x0000 */ UINT AlphaToMaskOffset0 : 2; /* BitPos=24 */
        /* 0x0000 */ UINT AlphaToMaskOffset1 : 2; /* BitPos=26 */
        /* 0x0000 */ UINT AlphaToMaskOffset2 : 2; /* BitPos=28 */
        /* 0x0000 */ UINT AlphaToMaskOffset3 : 2; /* BitPos=30 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_VTECONTROL { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT VportXScaleEnable : 1; /* BitPos=0 */
        /* 0x0000 */ UINT VportXOffsetEnable : 1; /* BitPos=1 */
        /* 0x0000 */ UINT VportYScaleEnable : 1; /* BitPos=2 */
        /* 0x0000 */ UINT VportYOffsetEnable : 1; /* BitPos=3 */
        /* 0x0000 */ UINT VportZScaleEnable : 1; /* BitPos=4 */
        /* 0x0000 */ UINT VportZOffsetEnable : 1; /* BitPos=5 */
        /* 0x0000 */ UINT : 2; /* BitPos=6 */
        /* 0x0000 */ UINT VtxXyFmt : 1; /* BitPos=8 */
        /* 0x0000 */ UINT VtxZFmt : 1; /* BitPos=9 */
        /* 0x0000 */ UINT VtxW0Fmt : 1; /* BitPos=10 */
        /* 0x0000 */ UINT PerfCounterRef : 1; /* BitPos=11 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_BLENDCONTROL { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT ColorSrcBlend : 5; /* BitPos=0 */
        /* 0x0000 */ UINT ColorBlendOp : 3; /* BitPos=5 */
        /* 0x0000 */ UINT ColorDestBlend : 5; /* BitPos=8 */
        /* 0x0000 */ UINT : 3; /* BitPos=13 */
        /* 0x0000 */ UINT AlphaSrcBlend : 5; /* BitPos=16 */
        /* 0x0000 */ UINT AlphaBlendOp : 3; /* BitPos=21 */
        /* 0x0000 */ UINT AlphaDestBlend : 5; /* BitPos=24 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_DEPTHCONTROL { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT StencilEnable : 1; /* BitPos=0 */
        /* 0x0000 */ UINT ZEnable : 1; /* BitPos=1 */
        /* 0x0000 */ UINT ZWriteEnable : 1; /* BitPos=2 */
        /* 0x0000 */ UINT : 1; /* BitPos=3 */
        /* 0x0000 */ UINT ZFunc : 3; /* BitPos=4 */
        /* 0x0000 */ UINT BackFaceEnable : 1; /* BitPos=7 */
        /* 0x0000 */ UINT StencilFunc : 3; /* BitPos=8 */
        /* 0x0000 */ UINT StencilFail : 3; /* BitPos=11 */
        /* 0x0000 */ UINT StencilZPass : 3; /* BitPos=14 */
        /* 0x0000 */ UINT StencilZFail : 3; /* BitPos=17 */
        /* 0x0000 */ UINT StencilFuncBF : 3; /* BitPos=20 */
        /* 0x0000 */ UINT StencilFailBF : 3; /* BitPos=23 */
        /* 0x0000 */ UINT StencilZPassBF : 3; /* BitPos=26 */
        /* 0x0000 */ UINT StencilZFailBF : 3; /* BitPos=29 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_CLIPCONTROL { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT ClipPlaneEnable0 : 1; /* BitPos=0 */
        /* 0x0000 */ UINT ClipPlaneEnable1 : 1; /* BitPos=1 */
        /* 0x0000 */ UINT ClipPlaneEnable2 : 1; /* BitPos=2 */
        /* 0x0000 */ UINT ClipPlaneEnable3 : 1; /* BitPos=3 */
        /* 0x0000 */ UINT ClipPlaneEnable4 : 1; /* BitPos=4 */
        /* 0x0000 */ UINT ClipPlaneEnable5 : 1; /* BitPos=5 */
        /* 0x0000 */ UINT : 8; /* BitPos=6 */
        /* 0x0000 */ UINT ClipPlaneMode : 2; /* BitPos=14 */
        /* 0x0000 */ UINT ClipDisable : 1; /* BitPos=16 */
        /* 0x0000 */ UINT ClipPlaneCullOnlyEnable : 1; /* BitPos=17 */
        /* 0x0000 */ UINT BoundaryEdgeFlagEnable : 1; /* BitPos=18 */
        /* 0x0000 */ UINT DxClipSpaceDef : 1; /* BitPos=19 */
        /* 0x0000 */ UINT DisableClipErrDetect : 1; /* BitPos=20 */
        /* 0x0000 */ UINT VtxKillOr : 1; /* BitPos=21 */
        /* 0x0000 */ UINT XyNanRetain : 1; /* BitPos=22 */
        /* 0x0000 */ UINT ZNanRetain : 1; /* BitPos=23 */
        /* 0x0000 */ UINT WNanRetain : 1; /* BitPos=24 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_EDRAMMODECONTROL { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT EdramMode : 3; /* BitPos=0 */
        /* 0x0000 */ UINT ColorDepthMacro : 1; /* BitPos=3 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_HICONTROL { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT HiZWriteEnable : 1; /* BitPos=0 */
        /* 0x0000 */ UINT HiZEnable : 1; /* BitPos=1 */
        /* 0x0000 */ UINT HiStencilWriteEnable : 1; /* BitPos=2 */
        /* 0x0000 */ UINT HiStencilEnable : 1; /* BitPos=3 */
        /* 0x0000 */ UINT HiZFunc : 1; /* BitPos=4 */
        /* 0x0000 */ UINT HiStencilFunc : 1; /* BitPos=5 */
        /* 0x0000 */ UINT : 2; /* BitPos=6 */
        /* 0x0000 */ UINT HiStencilRef : 8; /* BitPos=8 */
        /* 0x0000 */ UINT : 1; /* BitPos=16 */
        /* 0x0000 */ UINT HiBaseAddr : 15; /* BitPos=17 */
    };
    /* 0x0000 */ UINT dword;
};

struct GPU_CONTROLPACKET { /* Size=0x30 */
    /* 0x0000 */ GPU_DEPTHCONTROL DepthControl;
    /* 0x0004 */ GPU_BLENDCONTROL BlendControl0;
    /* 0x0008 */ GPU_COLORCONTROL ColorControl;
    /* 0x000c */ GPU_HICONTROL HiControl;
    /* 0x0010 */ GPU_CLIPCONTROL ClipControl;
    /* 0x0014 */ GPU_MODECONTROL ModeControl;
    /* 0x0018 */ GPU_VTECONTROL VteControl;
    /* 0x001c */ UINT Unused;
    /* 0x0020 */ GPU_EDRAMMODECONTROL EdramModeControl;
    /* 0x0024 */ GPU_BLENDCONTROL BlendControl1;
    /* 0x0028 */ GPU_BLENDCONTROL BlendControl2;
    /* 0x002c */ GPU_BLENDCONTROL BlendControl3;
};

union GPU_LINECONTROL { /* Size=0x4 */
    /* 0x0000 */ UINT Width : 16; /* BitPos=0 */
    /* 0x0000 */ UINT dword;
};

union GPU_VIZQUERY { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT VizQueryEnable : 1; /* BitPos=0 */
        /* 0x0000 */ UINT VizQueryId : 6; /* BitPos=1 */
        /* 0x0000 */ UINT KillPixPostHiZ : 1; /* BitPos=7 */
        /* 0x0000 */ UINT KillPixPostDetailMask : 1; /* BitPos=8 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_ENHANCE { /* Size=0x4 */
    /* 0x0000 */ UINT Misc : 16; /* BitPos=0 */
    /* 0x0000 */ UINT dword;
};

union GPU_GROUPVECTCONTROL { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT CompXEnable : 1; /* BitPos=0 */
        /* 0x0000 */ UINT CompYEnable : 1; /* BitPos=1 */
        /* 0x0000 */ UINT CompZEnable : 1; /* BitPos=2 */
        /* 0x0000 */ UINT CompWEnable : 1; /* BitPos=3 */
        /* 0x0000 */ UINT : 4; /* BitPos=4 */
        /* 0x0000 */ UINT Stride : 8; /* BitPos=8 */
        /* 0x0000 */ UINT Shift : 8; /* BitPos=16 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_POINTSIZE { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT Height : 16; /* BitPos=0 */
        /* 0x0000 */ UINT Width : 16; /* BitPos=16 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_GROUPPRIMTYPE { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT GroupPrimType : 4; /* BitPos=0 */
        /* 0x0000 */ UINT : 10; /* BitPos=4 */
        /* 0x0000 */ UINT GroupRetainOrder : 1; /* BitPos=14 */
        /* 0x0000 */ UINT GroupRetainQuads : 1; /* BitPos=15 */
        /* 0x0000 */ UINT GroupPrimOrder : 3; /* BitPos=16 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_HOSCONTROL { /* Size=0x4 */
    /* 0x0000 */ UINT TessMode : 2; /* BitPos=0 */
    /* 0x0000 */ UINT dword;
};

union GPU_OUTPUTPATHCONTROL { /* Size=0x4 */
    /* 0x0000 */ UINT PathSelect : 2; /* BitPos=0 */
    /* 0x0000 */ UINT dword;
};

union GPU_POINTMINMAX { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT MinSize : 16; /* BitPos=0 */
        /* 0x0000 */ UINT MaxSize : 16; /* BitPos=16 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_GROUPVECTFMTCONTROL { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT XConv : 4; /* BitPos=0 */
        /* 0x0000 */ UINT XOffset : 4; /* BitPos=4 */
        /* 0x0000 */ UINT YConv : 4; /* BitPos=8 */
        /* 0x0000 */ UINT YOffset : 4; /* BitPos=12 */
        /* 0x0000 */ UINT ZConv : 4; /* BitPos=16 */
        /* 0x0000 */ UINT ZOffset : 4; /* BitPos=20 */
        /* 0x0000 */ UINT WConv : 4; /* BitPos=24 */
        /* 0x0000 */ UINT WOffset : 4; /* BitPos=28 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_MPASSPSCONTROL { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT MPassPixVecPerPass : 20; /* BitPos=0 */
        /* 0x0000 */ UINT : 11; /* BitPos=20 */
        /* 0x0000 */ UINT MPassPsEnable : 1; /* BitPos=31 */
    };
    /* 0x0000 */ UINT dword;
};

struct GPU_TESSELLATORPACKET { /* Size=0x54 */
    /* 0x0000 */ GPU_POINTSIZE PointSize;
    /* 0x0004 */ GPU_POINTMINMAX PointMinMax;
    /* 0x0008 */ GPU_LINECONTROL LineControl;
    /* 0x000c */ UINT Unused1;
    /* 0x0010 */ GPU_OUTPUTPATHCONTROL OutputPathControl;
    /* 0x0014 */ GPU_HOSCONTROL HosControl;
    /* 0x0018 */ float HosMaxTessLevel;
    /* 0x001c */ float HosMinTessLevel;
    /* 0x0020 */ UINT HosReuseDepth;
    /* 0x0024 */ GPU_GROUPPRIMTYPE GroupPrimType;
    /* 0x0028 */ UINT GroupFirstDecr;
    /* 0x002c */ UINT GroupDecr;
    /* 0x0030 */ GPU_GROUPVECTCONTROL GroupVect0Control;
    /* 0x0034 */ GPU_GROUPVECTCONTROL GroupVect1Control;
    /* 0x0038 */ GPU_GROUPVECTFMTCONTROL GroupVect0FmtControl;
    /* 0x003c */ GPU_GROUPVECTFMTCONTROL GroupVect1FmtControl;
    /* 0x0040 */ UINT Unused2[2];
    /* 0x0048 */ GPU_MPASSPSCONTROL MPassPsControl;
    /* 0x004c */ GPU_VIZQUERY VizQuery;
    /* 0x0050 */ GPU_ENHANCE Enhance;
};

union GPU_DEBUGMISC0 { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT DbProgOn : 1; /* BitPos=0 */
        /* 0x0000 */ UINT : 3; /* BitPos=1 */
        /* 0x0000 */ UINT DbProbBreak : 1; /* BitPos=4 */
        /* 0x0000 */ UINT : 3; /* BitPos=5 */
        /* 0x0000 */ UINT DbProbAddr : 11; /* BitPos=8 */
        /* 0x0000 */ UINT : 5; /* BitPos=19 */
        /* 0x0000 */ UINT DbProbCount : 8; /* BitPos=24 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_COPYFUNC { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT CopyFuncRed : 3; /* BitPos=0 */
        /* 0x0000 */ UINT : 1; /* BitPos=3 */
        /* 0x0000 */ UINT CopyFuncGreen : 3; /* BitPos=4 */
        /* 0x0000 */ UINT : 1; /* BitPos=7 */
        /* 0x0000 */ UINT CopyFuncBlue : 3; /* BitPos=8 */
        /* 0x0000 */ UINT : 1; /* BitPos=11 */
        /* 0x0000 */ UINT CopyFuncAlpha : 3; /* BitPos=12 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_COPYMASK { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT CopyMaskRed : 8; /* BitPos=0 */
        /* 0x0000 */ UINT CopyMaskGreen : 8; /* BitPos=8 */
        /* 0x0000 */ UINT CopyMaskBlue : 8; /* BitPos=16 */
        /* 0x0000 */ UINT CopyMaskAlpha : 8; /* BitPos=24 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_SCLINECONTROL { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT BresControl : 8; /* BitPos=0 */
        /* 0x0000 */ UINT UseBresControl : 1; /* BitPos=8 */
        /* 0x0000 */ UINT ExpandLineWidth : 1; /* BitPos=9 */
        /* 0x0000 */ UINT LastPixel : 1; /* BitPos=10 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_VTXCONTROL { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT PixCenter : 1; /* BitPos=0 */
        /* 0x0000 */ UINT RoundMode : 2; /* BitPos=1 */
        /* 0x0000 */ UINT QuantMode : 3; /* BitPos=3 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_AACONFIG { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT MsaaNumSamples : 3; /* BitPos=0 */
        /* 0x0000 */ UINT : 10; /* BitPos=3 */
        /* 0x0000 */ UINT MaxSampleDist : 4; /* BitPos=13 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_SAMPLECOUNTCONTROL { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT ResetSampleCount : 1; /* BitPos=0 */
        /* 0x0000 */ UINT CopySampleCount : 1; /* BitPos=1 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_COPYDESTINFO { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT CopyDestEndian : 3; /* BitPos=0 */
        /* 0x0000 */ UINT CopyDestArray : 1; /* BitPos=3 */
        /* 0x0000 */ UINT CopyDestSlice : 3; /* BitPos=4 */
        /* 0x0000 */ UINT CopyDestFormat : 6; /* BitPos=7 */
        /* 0x0000 */ UINT CopyDestNumber : 3; /* BitPos=13 */
        /* 0x0000 */ UINT CopyDestExpBias : 6; /* BitPos=16 */
        /* 0x0000 */ UINT : 2; /* BitPos=22 */
        /* 0x0000 */ UINT CopyDestSwap : 1; /* BitPos=24 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_COPYREF { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT CopyRefRed : 8; /* BitPos=0 */
        /* 0x0000 */ UINT CopyRefGreen : 8; /* BitPos=8 */
        /* 0x0000 */ UINT CopyRefBlue : 8; /* BitPos=16 */
        /* 0x0000 */ UINT CopyRefAlpha : 8; /* BitPos=24 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_COPYCONTROL { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT CopySrcSelect : 3; /* BitPos=0 */
        /* 0x0000 */ UINT : 1; /* BitPos=3 */
        /* 0x0000 */ UINT CopySampleSelect : 3; /* BitPos=4 */
        /* 0x0000 */ UINT : 1; /* BitPos=7 */
        /* 0x0000 */ UINT ColorClearEnable : 1; /* BitPos=8 */
        /* 0x0000 */ UINT DepthClearEnable : 1; /* BitPos=9 */
        /* 0x0000 */ UINT : 10; /* BitPos=10 */
        /* 0x0000 */ UINT CopyCommand : 2; /* BitPos=20 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_COPYDESTPITCH { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT CopyDestPitch : 14; /* BitPos=0 */
        /* 0x0000 */ UINT : 2; /* BitPos=14 */
        /* 0x0000 */ UINT CopyDestHeight : 14; /* BitPos=16 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_OUTDEALLOCCONTROL { /* Size=0x4 */
    /* 0x0000 */ UINT DeallocDist : 7; /* BitPos=0 */
    /* 0x0000 */ UINT dword;
};

union GPU_CONST { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT Base : 9; /* BitPos=0 */
        /* 0x0000 */ UINT : 3; /* BitPos=9 */
        /* 0x0000 */ UINT Size : 9; /* BitPos=12 */
    };
    /* 0x0000 */ UINT dword;
};

union GPU_VERTEXREUSEBLOCKCONTROL { /* Size=0x4 */
    /* 0x0000 */ UINT VtxReuseDepth : 8; /* BitPos=0 */
    /* 0x0000 */ UINT dword;
};

union GPU_DEBUGMISC1 { /* Size=0x4 */
    struct {
        /* 0x0000 */ UINT DbOnPix : 1; /* BitPos=0 */
        /* 0x0000 */ UINT DbOnVtx : 1; /* BitPos=1 */
        /* 0x0000 */ UINT : 6; /* BitPos=2 */
        /* 0x0000 */ UINT DbInstCount : 8; /* BitPos=8 */
        /* 0x0000 */ UINT DbBreakAddr : 11; /* BitPos=16 */
    };
    /* 0x0000 */ UINT dword;
};

struct GPU_MISCPACKET { /* Size=0x98 */
    /* 0x0000 */ GPU_SCLINECONTROL ScLineControl;
    /* 0x0004 */ GPU_AACONFIG AaConfig;
    /* 0x0008 */ GPU_VTXCONTROL VtxControl;
    /* 0x000c */ float GbVertClipAdj;
    /* 0x0010 */ float GbVertDiscAdj;
    /* 0x0014 */ float GbHorzClipAdj;
    /* 0x0018 */ float GbHorzDiscAdj;
    /* 0x001c */ GPU_CONST VsConst;
    /* 0x0020 */ GPU_CONST PsConst;
    /* 0x0024 */ GPU_DEBUGMISC0 DebugMisc0;
    /* 0x0028 */ GPU_DEBUGMISC1 DebugMisc1;
    /* 0x002c */ UINT Unused1[5];
    /* 0x0040 */ UINT Unused2[2];
    /* 0x0048 */ UINT AaMask;
    /* 0x004c */ UINT Unused3[3];
    /* 0x0058 */ GPU_VERTEXREUSEBLOCKCONTROL VertexReuseBlockControl;
    /* 0x005c */ GPU_OUTDEALLOCCONTROL OutDeallocControl;
    /* 0x0060 */ GPU_COPYCONTROL CopyControl;
    /* 0x0064 */ UINT CopyDestBase;
    /* 0x0068 */ GPU_COPYDESTPITCH CopyDestPitch;
    /* 0x006c */ GPU_COPYDESTINFO CopyDestInfo;
    /* 0x0070 */ UINT HiClear;
    /* 0x0074 */ UINT DepthClear;
    /* 0x0078 */ UINT ColorClear;
    /* 0x007c */ UINT ColorClearLo;
    /* 0x0080 */ GPU_COPYFUNC CopyFunc;
    /* 0x0084 */ GPU_COPYREF CopyRef;
    /* 0x0088 */ GPU_COPYMASK CopyMask;
    /* 0x008c */ UINT CopySurfaceSlice;
    /* 0x0090 */ GPU_SAMPLECOUNTCONTROL SampleCountControl;
    /* 0x0094 */ UINT SampleCountAddress;
};

struct GPU_POINTPACKET { /* Size=0x20 */
    /* 0x0000 */ float PolyOffsetFrontScale;
    /* 0x0004 */ float PolyOffsetFrontOffset;
    /* 0x0008 */ float PolyOffsetBackScale;
    /* 0x000c */ float PolyOffsetBackOffset;
    /* 0x0010 */ float PointXRad;
    /* 0x0014 */ float PointYRad;
    /* 0x0018 */ float PointConstantSize;
    /* 0x001c */ float PointCullRad;
};

enum GPUPERFEVENT_RBBM {
    GPUPE_RBBM_COUNT = 0x0000,
    GPUPE_RBBM_NRT_BUSY = 0x0001,
    GPUPE_RBBM_BC_CNTX0_BUSY = 0x0002,
    GPUPE_RBBM_BC_CNTX17_BUSY = 0x0003,
    GPUPE_RBBM_SQ_CNTX0_BUSY = 0x0004,
    GPUPE_RBBM_SQ_CNTX17_BUSY = 0x0005,
    GPUPE_RBBM_VGT_BUSY = 0x0006,
    GPUPE_RBBM_VGT_NODMA_BUSY = 0x0007,
    GPUPE_RBBM_PA_BUSY = 0x0008,
    GPUPE_RBBM_SC_CNTX0_BUSY = 0x0009,
    GPUPE_RBBM_SC_CNTX17_BUSY = 0x000a,
    GPUPE_RBBM_TPC_BUSY = 0x000b,
    GPUPE_RBBM_TC_BUSY = 0x000c,
    GPUPE_RBBM_SX_BUSY = 0x000d,
    GPUPE_RESERVED1 = 0x000e,
    GPUPE_RBBM_CP_COHER_BUSY = 0x000f,
    GPUPE_RBBM_CP_NRT_BUSY = 0x0010,
    GPUPE_RESERVED2 = 0x0011,
    GPUPE_RBBM_CP_DMA_BUSY = 0x0012,
    GPUPE_RESERVED3 = 0x0013,
    GPUPE_RESERVED4 = 0x0014,
    GPUPE_RESERVED5 = 0x0015,
    GPUPE_RBBM_DMA_IDLE_STALL = 0x0016,
    GPUPE_RESERVED6 = 0x0017,
    GPUPE_RESERVED7 = 0x0018,
    GPUPE_RBBM_GFX_IDLE_STALL = 0x0019,
    GPUPE_RBBM_GFX_IDLEC_STALL = 0x001a,
    GPUPE_RBBM_INTERRUPT = 0x001b,
};

enum GPUPERFEVENT_MC {
    GPUPE_RANK_BANK0_ACCESSES = 0x0000,
    GPUPE_RANK_BANK1_ACCESSES = 0x0001,
    GPUPE_RANK_BANK2_ACCESSES = 0x0002,
    GPUPE_RANK_BANK3_ACCESSES = 0x0003,
    GPUPE_RANK_BANK4_ACCESSES = 0x0004,
    GPUPE_RANK_BANK5_ACCESSES = 0x0005,
    GPUPE_RANK_BANK6_ACCESSES = 0x0006,
    GPUPE_RANK_BANK7_ACCESSES = 0x0007,
    GPUPE_RANK_BANK8_ACCESSES = 0x0008,
    GPUPE_RANK_BANK9_ACCESSES = 0x0009,
    GPUPE_RANK_BANK10_ACCESSES = 0x000a,
    GPUPE_RANK_BANK11_ACCESSES = 0x000b,
    GPUPE_RANK_BANK12_ACCESSES = 0x000c,
    GPUPE_RANK_BANK13_ACCESSES = 0x000d,
    GPUPE_RANK_BANK14_ACCESSES = 0x000e,
    GPUPE_RANK_BANK15_ACCESSES = 0x000f,
    GPUPE_READ_2_WRITE = 0x0010,
    GPUPE_WRITE_2_READ = 0x0011,
    GPUPE_NEW_PAGE_ACCESSES = 0x0012,
    GPUPE_TOTAL_ACCESSES = 0x0013,
    GPUPE_READ_ACCESSES = 0x0014,
    GPUPE_ACCESS_PRESENT_NO_ISSUE_CLKS = 0x0015,
    GPUPE_CMD_PRESENT_NO_XFER_CLKS = 0x0016,
    GPUPE_URGENT_DC_ACCESSES = 0x0017,
    GPUPE_URGENT_SB_ACCESSES = 0x0018,
    GPUPE_URGENT_BIU_ACCESSES = 0x0019,
    GPUPE_NEW_BIUF_ACCESSES = 0x001a,
    GPUPE_NEW_CP_ACCESSES = 0x001b,
    GPUPE_NEW_TC_ACCESSES = 0x001c,
    GPUPE_NEW_VC_ACCESSES = 0x001d,
    GPUPE_NEW_BC_CP_ACCESSES = 0x001e,
    GPUPE_NEW_BC_EX_ACCESSES = 0x001f,
    GPUPE_NEW_VGT_ACCESSES = 0x0020,
    GPUPE_NEW_DC_ACCESSES = 0x0021,
    GPUPE_NEW_SB_ACCESSES = 0x0022,
    GPUPE_NEW_BIUS_ACCESSES = 0x0023,
    GPUPE_BIUS_READ_ACCESSES = 0x0024,
    GPUPE_SB_READ_ACCESSES = 0x0025,
    GPUPE_CP_READ_ACCESSES = 0x0026,
    GPUPE_NTH_SMPG_ACCESS_IS_BIUF = 0x0027,
    GPUPE_NTH_SMPG_ACCESS_IS_CP = 0x0028,
    GPUPE_NTH_SMPG_ACCESS_IS_TC = 0x0029,
    GPUPE_NTH_SMPG_ACCESS_IS_VC = 0x002a,
    GPUPE_NTH_SMPG_ACCESS_IS_BC_CP = 0x002b,
    GPUPE_NTH_SMPG_ACCESS_IS_BC_EX = 0x002c,
    GPUPE_NTH_SMPG_ACCESS_IS_VGT = 0x002d,
    GPUPE_NTH_SMPG_ACCESS_IS_DC = 0x002e,
    GPUPE_NTH_SMPG_ACCESS_IS_SB = 0x002f,
    GPUPE_NTH_SMPG_ACCESS_IS_BIUS = 0x0030,
    GPUPE_N_VALID_ENTRY_IN_TEXTURE_CAM_CLKS = 0x0031,
    GPUPE_N_VALID_ENTRY_IN_VC_CAM_CLKS = 0x0032,
    GPUPE_N_VALID_ENTRY_IN_BIUS_CAM_CLKS = 0x0033,
    GPUPE_N_SB_BUF_USED_CLKS = 0x0034,
    GPUPE_N_TC_BUF_USED_CLKS = 0x0035,
    GPUPE_N_VC_BUF_USED_CLKS = 0x0036,
    GPUPE_N_DC_BUF_USED_CLKS = 0x0037,
    GPUPE_N_VGT_BUF_USED_CLK = 0x0038,
    GPUPE_N_CP_BUF_USED_CLKS = 0x0039,
};

enum GPUPERFEVENT_HZ {
    GPUPE_WRC_1VAL_QUADS = 0x0000,
    GPUPE_WRC_2VAL_QUADS = 0x0001,
    GPUPE_WRC_3VAL_QUADS = 0x0002,
    GPUPE_WRC_4VAL_QUADS = 0x0003,
    GPUPE_HZ_WR_BUSY = 0x0004,
    GPUPE_HZ_SPARE0 = 0x0005,
    GPUPE_RDC_TILE_HIT = 0x0006,
    GPUPE_RDC_STAGE3_STALL = 0x0007,
    GPUPE_HZ_SPARE1 = 0x0008,
    GPUPE_HZ_SPARE2 = 0x0009,
    GPUPE_QUADS_KEPT = 0x000a,
    GPUPE_QUADS_ZCULL = 0x000b,
    GPUPE_QUADS_SCULL = 0x000c,
    GPUPE_QUADS_SZCULL = 0x000d,
    GPUPE_HZ_RE_BUSY = 0x000e,
    GPUPE_BC_SCLK_COUNT = 0x000f,
    GPUPE_BC_HZ_VALID0 = 0x0010,
    GPUPE_BC_HZ_VALID1 = 0x0011,
    GPUPE_BC_HZ_VALID2 = 0x0012,
    GPUPE_BC_HZ_VALID3 = 0x0013,
    GPUPE_SC_SCLK_COUNT = 0x0014,
    GPUPE_SC_HZ_COARSE_SND = 0x0015,
    GPUPE_HZ_SC_COARSE_HLD = 0x0016,
    GPUPE_HZ_SC_HIER_SND = 0x0017,
    GPUPE_SC_HZ_HIER_HLD = 0x0018,
    GPUPE_HZ_BC_TILE_SND = 0x0019,
    GPUPE_BC_HZ_TILE_HLD = 0x001a,
    GPUPE_SC_STALL_HZ = 0x001b,
    GPUPE_BC_STALL_HZ = 0x001c,
    GPUPE_EVENT_STALL = 0x001d,
    GPUPE_SC_STARVE_HZ_BUSY = 0x001e,
    GPUPE_SC_STARVE_HZ_IDLE = 0x001f,
};

enum GPUPERFEVENT_TCR {
    GPUPE_DGMMPD_IPMUX0_STALL = 0x0000,
    GPUPE_DGMMPD_IPMUX1_STALL = 0x0001,
    GPUPE_DGMMPD_IPMUX2_STALL = 0x0002,
    GPUPE_DGMMPD_IPMUX3_STALL = 0x0003,
    GPUPE_DGMMPD_IPMUX_ALL_STALL = 0x0004,
    GPUPE_OPMUX0_L2_WRITES = 0x0005,
    GPUPE_OPMUX1_L2_WRITES = 0x0006,
    GPUPE_OPMUX2_L2_WRITES = 0x0007,
    GPUPE_OPMUX3_L2_WRITES = 0x0008,
};

enum GPUPERFEVENT_SQ {
    GPUPE_SQ_PIXEL_VECTORS_SUB = 0x0000,
    GPUPE_SQ_VERTEX_VECTORS_SUB = 0x0001,
    GPUPE_SQ_ALU0_ACTIVE_VTX_SIMD0 = 0x0002,
    GPUPE_SQ_ALU1_ACTIVE_VTX_SIMD0 = 0x0003,
    GPUPE_SQ_ALU0_ACTIVE_PIX_SIMD0 = 0x0004,
    GPUPE_SQ_ALU1_ACTIVE_PIX_SIMD0 = 0x0005,
    GPUPE_SQ_ALU0_ACTIVE_VTX_SIMD1 = 0x0006,
    GPUPE_SQ_ALU1_ACTIVE_VTX_SIMD1 = 0x0007,
    GPUPE_SQ_ALU0_ACTIVE_PIX_SIMD1 = 0x0008,
    GPUPE_SQ_ALU1_ACTIVE_PIX_SIMD1 = 0x0009,
    GPUPE_SQ_EXPORT_CYCLES = 0x000a,
    GPUPE_SQ_ALU_CST_WRITTEN = 0x000b,
    GPUPE_SQ_TEX_CST_WRITTEN = 0x000c,
    GPUPE_SQ_ALU_CST_STALL = 0x000d,
    GPUPE_SQ_ALU_TEX_STALL = 0x000e,
    GPUPE_SQ_INST_WRITTEN = 0x000f,
    GPUPE_SQ_BOOLEAN_WRITTEN = 0x0010,
    GPUPE_SQ_LOOPS_WRITTEN = 0x0011,
    GPUPE_SQ_PIXEL_SWAP_IN = 0x0012,
    GPUPE_SQ_PIXEL_SWAP_OUT = 0x0013,
    GPUPE_SQ_VERTEX_SWAP_IN = 0x0014,
    GPUPE_SQ_VERTEX_SWAP_OUT = 0x0015,
    GPUPE_SQ_ALU_VTX_INST_ISSUED = 0x0016,
    GPUPE_SQ_TEX_VTX_INST_ISSUED = 0x0017,
    GPUPE_SQ_VC_VTX_INST_ISSUED = 0x0018,
    GPUPE_SQ_CF_VTX_INST_ISSUED = 0x0019,
    GPUPE_SQ_ALU_PIX_INST_ISSUED = 0x001a,
    GPUPE_SQ_TEX_PIX_INST_ISSUED = 0x001b,
    GPUPE_SQ_VC_PIX_INST_ISSUED = 0x001c,
    GPUPE_SQ_CF_PIX_INST_ISSUED = 0x001d,
    GPUPE_SQ_ALU0_FIFO_EMPTY_SIMD0 = 0x001e,
    GPUPE_SQ_ALU1_FIFO_EMPTY_SIMD0 = 0x001f,
    GPUPE_SQ_ALU0_FIFO_EMPTY_SIMD1 = 0x0020,
    GPUPE_SQ_ALU1_FIFO_EMPTY_SIMD1 = 0x0021,
    GPUPE_SQ_ALU_NOPS = 0x0022,
    GPUPE_SQ_PRED_SKIP = 0x0023,
    GPUPE_SQ_SYNC_ALU_STALL_SIMD0_VTX = 0x0024,
    GPUPE_SQ_SYNC_ALU_STALL_SIMD1_VTX = 0x0025,
    GPUPE_SQ_SYNC_TEX_STALL_VTX = 0x0026,
    GPUPE_SQ_SYNC_VC_STALL_VTX = 0x0027,
    GPUPE_SQ_CONSTANTS_USED_SIMD0 = 0x0028,
    GPUPE_SQ_CONSTANTS_SENT_SP_SIMD0 = 0x0029,
    GPUPE_SQ_GPR_STALL_VTX = 0x002a,
    GPUPE_SQ_GPR_STALL_PIX = 0x002b,
    GPUPE_SQ_VTX_RS_STALL = 0x002c,
    GPUPE_SQ_PIX_RS_STALL = 0x002d,
    GPUPE_SQ_SX_PC_FULL = 0x002e,
    GPUPE_SQ_SX_EXP_BUFF_FULL = 0x002f,
    GPUPE_SQ_SX_POS_BUFF_FULL = 0x0030,
    GPUPE_SQ_INTERP_QUADS = 0x0031,
    GPUPE_SQ_INTERP_ACTIVE = 0x0032,
    GPUPE_SQ_IN_PIXEL_STALL = 0x0033,
    GPUPE_SQ_IN_VTX_STALL = 0x0034,
    GPUPE_SQ_VTX_CNT = 0x0035,
    GPUPE_SQ_VTX_16_VECTOR = 0x0036,
    GPUPE_SQ_VTX_32_VECTOR = 0x0037,
    GPUPE_SQ_VTX_48_VECTOR = 0x0038,
    GPUPE_SQ_PIXEL_16_VECTOR = 0x0039,
    GPUPE_SQ_PIXEL_32_VECTOR = 0x003a,
    GPUPE_SQ_PIXEL_48_VECTOR = 0x003b,
    GPUPE_SQ_CONSTANTS_USED_SIMD1 = 0x003c,
    GPUPE_SQ_CONSTANTS_SENT_SP_SIMD1 = 0x003d,
    GPUPE_SQ_SX_MEM_EXP_FULL = 0x003e,
    GPUPE_SQ_ALU0_ACTIVE_VTX_SIMD2 = 0x003f,
    GPUPE_SQ_ALU1_ACTIVE_VTX_SIMD2 = 0x0040,
    GPUPE_SQ_ALU0_ACTIVE_PIX_SIMD2 = 0x0041,
    GPUPE_SQ_ALU1_ACTIVE_PIX_SIMD2 = 0x0042,
    GPUPE_SQ_ALU0_ACTIVE_VTX_SIMD3 = 0x0043,
    GPUPE_SQ_ALU1_ACTIVE_VTX_SIMD3 = 0x0044,
    GPUPE_SQ_ALU0_ACTIVE_PIX_SIMD3 = 0x0045,
    GPUPE_SQ_ALU1_ACTIVE_PIX_SIMD3 = 0x0046,
    GPUPE_SQ_ALU0_FIFO_EMPTY_SIMD2 = 0x0047,
    GPUPE_SQ_ALU1_FIFO_EMPTY_SIMD2 = 0x0048,
    GPUPE_SQ_ALU0_FIFO_EMPTY_SIMD3 = 0x0049,
    GPUPE_SQ_ALU1_FIFO_EMPTY_SIMD3 = 0x004a,
    GPUPE_SQ_SYNC_ALU_STALL_SIMD2_VTX = 0x004b,
    GPUPE_SQ_SYNC_ALU_STALL_SIMD3_VTX = 0x004c,
    GPUPE_SQ_SYNC_ALU_STALL_SIMD0_PIX = 0x004d,
    GPUPE_SQ_SYNC_ALU_STALL_SIMD1_PIX = 0x004e,
    GPUPE_SQ_SYNC_ALU_STALL_SIMD2_PIX = 0x004f,
    GPUPE_SQ_SYNC_ALU_STALL_SIMD3_PIX = 0x0050,
    GPUPE_SQ_SYNC_TEX_STALL_PIX = 0x0051,
    GPUPE_SQ_SYNC_VC_STALL_PIX = 0x0052,
    GPUPE_SQ_CONSTANTS_USED_SIMD2 = 0x0053,
    GPUPE_SQ_CONSTANTS_SENT_SP_SIMD2 = 0x0054,
    GPUPE_SQ_CONSTANTS_USED_SIMD3 = 0x0055,
    GPUPE_SQ_CONSTANTS_SENT_SP_SIMD3 = 0x0056,
    GPUPE_SQ_ALU0_FIFO_FULL_SIMD0 = 0x0057,
    GPUPE_SQ_ALU1_FIFO_FULL_SIMD0 = 0x0058,
    GPUPE_SQ_ALU0_FIFO_FULL_SIMD1 = 0x0059,
    GPUPE_SQ_ALU1_FIFO_FULL_SIMD1 = 0x005a,
    GPUPE_SQ_ALU0_FIFO_FULL_SIMD2 = 0x005b,
    GPUPE_SQ_ALU1_FIFO_FULL_SIMD2 = 0x005c,
    GPUPE_SQ_ALU0_FIFO_FULL_SIMD3 = 0x005d,
    GPUPE_SQ_ALU1_FIFO_FULL_SIMD3 = 0x005e,
    GPUPE_VC_PERF_STATIC = 0x005f,
    GPUPE_VC_PERF_STALLED = 0x0060,
    GPUPE_VC_PERF_STARVED = 0x0061,
    GPUPE_VC_PERF_SEND = 0x0062,
    GPUPE_VC_PERF_ACTUAL_STARVED = 0x0063,
    GPUPE_PIXEL_THREAD_0_ACTIVE = 0x0064,
    GPUPE_VERTEX_THREAD_0_ACTIVE = 0x0065,
    GPUPE_PIXEL_THREAD_0_NUMBER = 0x0066,
    GPUPE_VERTEX_THREAD_0_NUMBER = 0x0067,
    GPUPE_VERTEX_EVENT_NUMBER = 0x0068,
    GPUPE_PIXEL_EVENT_NUMBER = 0x0069,
};

enum GPUPERFEVENT_PA_SU {
    GPUPE_PERF_PAPC_PASX_REQ = 0x0000,
    GPUPE_PERF_PAPC_PASX_DISABLE_PIPE = 0x0001,
    GPUPE_PERF_PAPC_PASX_FIRST_VECTOR = 0x0002,
    GPUPE_PERF_PAPC_PASX_SECOND_VECTOR = 0x0003,
    GPUPE_PERF_PAPC_PASX_FIRST_DEAD = 0x0004,
    GPUPE_PERF_PAPC_PASX_SECOND_DEAD = 0x0005,
    GPUPE_PERF_PAPC_PASX_VTX_KILL_DISCARD = 0x0006,
    GPUPE_PERF_PAPC_PASX_VTX_NAN_DISCARD = 0x0007,
    GPUPE_PERF_PAPC_PA_INPUT_PRIM = 0x0008,
    GPUPE_PERF_PAPC_PA_INPUT_NULL_PRIM = 0x0009,
    GPUPE_PERF_PAPC_PA_INPUT_EVENT_FLAG = 0x000a,
    GPUPE_PERF_PAPC_PA_INPUT_FIRST_PRIM_SLOT = 0x000b,
    GPUPE_PERF_PAPC_PA_INPUT_END_OF_PACKET = 0x000c,
    GPUPE_PERF_PAPC_CLPR_CULL_PRIM = 0x000d,
    GPUPE_PERF_PAPC_CLPR_VVUCP_CULL_PRIM = 0x000e,
    GPUPE_PERF_PAPC_CLPR_VV_CULL_PRIM = 0x000f,
    GPUPE_PERF_PAPC_CLPR_UCP_CULL_PRIM = 0x0010,
    GPUPE_PERF_PAPC_CLPR_VTX_KILL_CULL_PRIM = 0x0011,
    GPUPE_PERF_PAPC_CLPR_VTX_NAN_CULL_PRIM = 0x0012,
    GPUPE_PERF_PAPC_CLPR_CULL_TO_NULL_PRIM = 0x0013,
    GPUPE_PERF_PAPC_CLPR_VVUCP_CLIP_PRIM = 0x0014,
    GPUPE_PERF_PAPC_CLPR_VV_CLIP_PRIM = 0x0015,
    GPUPE_PERF_PAPC_CLPR_UCP_CLIP_PRIM = 0x0016,
    GPUPE_PERF_PAPC_CLPR_POINT_CLIP_CANDIDATE = 0x0017,
    GPUPE_PERF_PAPC_CLPR_CLIP_PLANE_CNT_1 = 0x0018,
    GPUPE_PERF_PAPC_CLPR_CLIP_PLANE_CNT_2 = 0x0019,
    GPUPE_PERF_PAPC_CLPR_CLIP_PLANE_CNT_3 = 0x001a,
    GPUPE_PERF_PAPC_CLPR_CLIP_PLANE_CNT_4 = 0x001b,
    GPUPE_PERF_PAPC_CLPR_CLIP_PLANE_CNT_5_8 = 0x001c,
    GPUPE_PERF_PAPC_CLPR_CLIP_PLANE_CNT_9_12 = 0x001d,
    GPUPE_PERF_PAPC_CLPR_CLIP_PLANE_NEAR = 0x001e,
    GPUPE_PERF_PAPC_CLPR_CLIP_PLANE_FAR = 0x001f,
    GPUPE_PERF_PAPC_CLPR_CLIP_PLANE_LEFT = 0x0020,
    GPUPE_PERF_PAPC_CLPR_CLIP_PLANE_RIGHT = 0x0021,
    GPUPE_PERF_PAPC_CLPR_CLIP_PLANE_TOP = 0x0022,
    GPUPE_PERF_PAPC_CLPR_CLIP_PLANE_BOTTOM = 0x0023,
    GPUPE_PERF_PAPC_CLSM_NULL_PRIM = 0x0024,
    GPUPE_PERF_PAPC_CLSM_TOTALLY_VISIBLE_PRIM = 0x0025,
    GPUPE_PERF_PAPC_CLSM_CLIP_PRIM = 0x0026,
    GPUPE_PERF_PAPC_CLSM_CULL_TO_NULL_PRIM = 0x0027,
    GPUPE_PERF_PAPC_CLSM_OUT_PRIM_CNT_1 = 0x0028,
    GPUPE_PERF_PAPC_CLSM_OUT_PRIM_CNT_2 = 0x0029,
    GPUPE_PERF_PAPC_CLSM_OUT_PRIM_CNT_3 = 0x002a,
    GPUPE_PERF_PAPC_CLSM_OUT_PRIM_CNT_4 = 0x002b,
    GPUPE_PERF_PAPC_CLSM_OUT_PRIM_CNT_5_8 = 0x002c,
    GPUPE_PERF_PAPC_CLSM_OUT_PRIM_CNT_9_13 = 0x002d,
    GPUPE_PERF_PAPC_CLSM_NON_TRIVIAL_CULL = 0x002e,
    GPUPE_PERF_PAPC_SU_INPUT_PRIM = 0x002f,
    GPUPE_PERF_PAPC_SU_INPUT_CLIP_PRIM = 0x0030,
    GPUPE_PERF_PAPC_SU_INPUT_NULL_PRIM = 0x0031,
    GPUPE_PERF_PAPC_SU_ZERO_AREA_CULL_PRIM = 0x0032,
    GPUPE_PERF_PAPC_SU_BACK_FACE_CULL_PRIM = 0x0033,
    GPUPE_PERF_PAPC_SU_FRONT_FACE_CULL_PRIM = 0x0034,
    GPUPE_PERF_PAPC_SU_POLYMODE_FACE_CULL = 0x0035,
    GPUPE_PERF_PAPC_SU_POLYMODE_BACK_CULL = 0x0036,
    GPUPE_PERF_PAPC_SU_POLYMODE_FRONT_CULL = 0x0037,
    GPUPE_PERF_PAPC_SU_POLYMODE_INVALID_FILL = 0x0038,
    GPUPE_PERF_PAPC_SU_OUTPUT_PRIM = 0x0039,
    GPUPE_PERF_PAPC_SU_OUTPUT_CLIP_PRIM = 0x003a,
    GPUPE_PERF_PAPC_SU_OUTPUT_NULL_PRIM = 0x003b,
    GPUPE_PERF_PAPC_SU_OUTPUT_EVENT_FLAG = 0x003c,
    GPUPE_PERF_PAPC_SU_OUTPUT_FIRST_PRIM_SLOT = 0x003d,
    GPUPE_PERF_PAPC_SU_OUTPUT_END_OF_PACKET = 0x003e,
    GPUPE_PERF_PAPC_SU_OUTPUT_POLYMODE_FACE = 0x003f,
    GPUPE_PERF_PAPC_SU_OUTPUT_POLYMODE_BACK = 0x0040,
    GPUPE_PERF_PAPC_SU_OUTPUT_POLYMODE_FRONT = 0x0041,
    GPUPE_PERF_PAPC_SU_OUT_CLIP_POLYMODE_FACE = 0x0042,
    GPUPE_PERF_PAPC_SU_OUT_CLIP_POLYMODE_BACK = 0x0043,
    GPUPE_PERF_PAPC_SU_OUT_CLIP_POLYMODE_FRONT = 0x0044,
    GPUPE_PERF_PAPC_PASX_REQ_IDLE = 0x0045,
    GPUPE_PERF_PAPC_PASX_REQ_BUSY = 0x0046,
    GPUPE_PERF_PAPC_PASX_REQ_STALLED = 0x0047,
    GPUPE_PERF_PAPC_PASX_REC_IDLE = 0x0048,
    GPUPE_PERF_PAPC_PASX_REC_BUSY = 0x0049,
    GPUPE_PERF_PAPC_PASX_REC_STARVED_SX = 0x004a,
    GPUPE_PERF_PAPC_PASX_REC_STALLED = 0x004b,
    GPUPE_PERF_PAPC_PASX_REC_STALLED_POS_MEM = 0x004c,
    GPUPE_PERF_PAPC_PASX_REC_STALLED_CCGSM_IN = 0x004d,
    GPUPE_PERF_PAPC_CCGSM_IDLE = 0x004e,
    GPUPE_PERF_PAPC_CCGSM_BUSY = 0x004f,
    GPUPE_PERF_PAPC_CCGSM_STALLED = 0x0050,
    GPUPE_PERF_PAPC_CLPRIM_IDLE = 0x0051,
    GPUPE_PERF_PAPC_CLPRIM_BUSY = 0x0052,
    GPUPE_PERF_PAPC_CLPRIM_STALLED = 0x0053,
    GPUPE_PERF_PAPC_CLPRIM_STARVED_CCGSM = 0x0054,
    GPUPE_PERF_PAPC_CLIPSM_IDLE = 0x0055,
    GPUPE_PERF_PAPC_CLIPSM_BUSY = 0x0056,
    GPUPE_PERF_PAPC_CLIPSM_WAIT_CLIP_VERT_ENGH = 0x0057,
    GPUPE_PERF_PAPC_CLIPSM_WAIT_HIGH_PRI_SEQ = 0x0058,
    GPUPE_PERF_PAPC_CLIPSM_WAIT_CLIPGA = 0x0059,
    GPUPE_PERF_PAPC_CLIPSM_WAIT_AVAIL_VTE_CLIP = 0x005a,
    GPUPE_PERF_PAPC_CLIPSM_WAIT_CLIP_OUTSM = 0x005b,
    GPUPE_PERF_PAPC_CLIPGA_IDLE = 0x005c,
    GPUPE_PERF_PAPC_CLIPGA_BUSY = 0x005d,
    GPUPE_PERF_PAPC_CLIPGA_STARVED_VTE_CLIP = 0x005e,
    GPUPE_PERF_PAPC_CLIPGA_STALLED = 0x005f,
    GPUPE_PERF_PAPC_CLIP_IDLE = 0x0060,
    GPUPE_PERF_PAPC_CLIP_BUSY = 0x0061,
    GPUPE_PERF_PAPC_SU_IDLE = 0x0062,
    GPUPE_PERF_PAPC_SU_BUSY = 0x0063,
    GPUPE_PERF_PAPC_SU_STARVED_CLIP = 0x0064,
    GPUPE_PERF_PAPC_SU_STALLED_SC = 0x0065,
};

enum GPUPERFEVENT_VC {
    GPUPE_RG_VERTICES = 0x0000,
    GPUPE_RG_CLAMPED = 0x0001,
    GPUPE_RG_L2_REQUEST = 0x0002,
    GPUPE_RG_L1_REQUEST = 0x0003,
    GPUPE_RG_MEGAFETCH = 0x0004,
    GPUPE_RG_END_OF_GROUP = 0x0005,
    GPUPE_RG_CONFLICT = 0x0006,
    GPUPE_RG_DWORDS_REQUESTED = 0x0007,
    GPUPE_RG_SPARE0 = 0x0008,
    GPUPE_RG_SPARE1 = 0x0009,
    GPUPE_CC_STALLS = 0x000a,
    GPUPE_CC_HITS = 0x000b,
    GPUPE_CC_MISSES = 0x000c,
    GPUPE_CC_SECTOR_MISSES = 0x000d,
    GPUPE_CC_L2B_STALLS = 0x000e,
    GPUPE_CC_MI_STALLS = 0x000f,
    GPUPE_CC_MULTICYCLE_STALLS = 0x0010,
    GPUPE_CC_EVEN_ALLOC_STALLS = 0x0011,
    GPUPE_CC_ODD_ALLOC_STALLS = 0x0012,
    GPUPE_CC_EVEN_BUSY_STALLS = 0x0013,
    GPUPE_CC_ODD_BUSY_STALLS = 0x0014,
    GPUPE_CC_IN_FIFO_EMPTY = 0x0015,
    GPUPE_CC_IN_FIFO_FULL = 0x0016,
    GPUPE_CC_FREEZE = 0x0017,
    GPUPE_CC_SPARE0 = 0x0018,
    GPUPE_CC_SPARE1 = 0x0019,
    GPUPE_CC_SPARE2 = 0x001a,
    GPUPE_CC_SPARE3 = 0x001b,
    GPUPE_CC_SPARE4 = 0x001c,
    GPUPE_CC_SPARE5 = 0x001d,
    GPUPE_MI_REQUESTS_TO_MH = 0x001e,
    GPUPE_MI_AGP_REQUESTS = 0x001f,
    GPUPE_MI_LATENCY_BITS_4_0 = 0x0020,
    GPUPE_MI_LATENCY_BITS_9_5 = 0x0021,
    GPUPE_MI_LATENCY_BITS_14_10 = 0x0022,
    GPUPE_MI_LATENCY_BITS_17_15 = 0x0023,
    GPUPE_MI_INPUT_FIFOS_FULL = 0x0024,
    GPUPE_MI_INPUT_FIFOS_0_FULL = 0x0025,
    GPUPE_MI_INPUT_FIFOS_1_FULL = 0x0026,
    GPUPE_MI_SPARE1 = 0x0027,
    GPUPE_RP_SP_DATA_VALID = 0x0028,
    GPUPE_RP_STALLED = 0x0029,
    GPUPE_RP_SPARE0 = 0x002a,
    GPUPE_RP_SPARE1 = 0x002b,
    GPUPE_RP_SPARE2 = 0x002c,
    GPUPE_RP_SPARE3 = 0x002d,
    GPUPE_RP_SPARE4 = 0x002e,
    GPUPE_RP_SPARE5 = 0x002f,
    GPUPE_RP_SPARE6 = 0x0030,
    GPUPE_RP_SPARE7 = 0x0031,
    GPUPE_DC_NUM_VALIDS = 0x0032,
    GPUPE_DC_SPARE0 = 0x0033,
    GPUPE_DC_SPARE1 = 0x0034,
    GPUPE_DC_SPARE2 = 0x0035,
    GPUPE_DC_SPARE3 = 0x0036,
    GPUPE_DC_SPARE4 = 0x0037,
    GPUPE_DC_SPARE5 = 0x0038,
    GPUPE_DC_SPARE6 = 0x0039,
    GPUPE_DC_SPARE7 = 0x003a,
    GPUPE_DC_SPARE8 = 0x003b,
    GPUPE_SQ_VC_SEND = 0x003c,
    GPUPE_VC_STARVED_IDLE = 0x003d,
    GPUPE_VC_BUSY = 0x003e,
    GPUPE_VC_IDLE = 0x003f,
    GPUPE_VC_SPARE0 = 0x0040,
    GPUPE_VC_SPARE1 = 0x0041,
    GPUPE_VC_SPARE2 = 0x0042,
    GPUPE_VC_SPARE3 = 0x0043,
    GPUPE_VC_SPARE4 = 0x0044,
    GPUPE_VC_SPARE5 = 0x0045,
};

enum GPUPERFEVENT_VGT {
    GPUPE_VGT_SQ_EVENT_WINDOW_ACTIVE = 0x0000,
    GPUPE_VGT_SQ_SEND = 0x0001,
    GPUPE_VGT_SQ_STALLED = 0x0002,
    GPUPE_VGT_SQ_STARVED_BUSY = 0x0003,
    GPUPE_VGT_SQ_STARVED_IDLE = 0x0004,
    GPUPE_VGT_SQ_STATIC = 0x0005,
    GPUPE_VGT_PA_EVENT_WINDOW_ACTIVE = 0x0006,
    GPUPE_VGT_PA_CLIP_V_SEND = 0x0007,
    GPUPE_VGT_PA_CLIP_V_STALLED = 0x0008,
    GPUPE_VGT_PA_CLIP_V_STARVED_BUSY = 0x0009,
    GPUPE_VGT_PA_CLIP_V_STARVED_IDLE = 0x000a,
    GPUPE_VGT_PA_CLIP_V_STATIC = 0x000b,
    GPUPE_VGT_PA_CLIP_P_SEND = 0x000c,
    GPUPE_VGT_PA_CLIP_P_STALLED = 0x000d,
    GPUPE_VGT_PA_CLIP_P_STARVED_BUSY = 0x000e,
    GPUPE_VGT_PA_CLIP_P_STARVED_IDLE = 0x000f,
    GPUPE_VGT_PA_CLIP_P_STATIC = 0x0010,
    GPUPE_VGT_PA_CLIP_S_SEND = 0x0011,
    GPUPE_VGT_PA_CLIP_S_STALLED = 0x0012,
    GPUPE_VGT_PA_CLIP_S_STARVED_BUSY = 0x0013,
    GPUPE_VGT_PA_CLIP_S_STARVED_IDLE = 0x0014,
    GPUPE_VGT_PA_CLIP_S_STATIC = 0x0015,
    GPUPE_RBIU_FIFOS_EVENT_WINDOW_ACTIVE = 0x0016,
    GPUPE_RBIU_IMMED_DATA_FIFO_STARVED = 0x0017,
    GPUPE_RBIU_IMMED_DATA_FIFO_STALLED = 0x0018,
    GPUPE_RBIU_DMA_REQUEST_FIFO_STARVED = 0x0019,
    GPUPE_RBIU_DMA_REQUEST_FIFO_STALLED = 0x001a,
    GPUPE_RBIU_DRAW_INITIATOR_FIFO_STARVED = 0x001b,
    GPUPE_RBIU_DRAW_INITIATOR_FIFO_STALLED = 0x001c,
    GPUPE_SPARE29 = 0x001d,
    GPUPE_SPARE30 = 0x001e,
    GPUPE_SPARE31 = 0x001f,
    GPUPE_SPARE32 = 0x0020,
    GPUPE_SPARE33 = 0x0021,
    GPUPE_SPARE34 = 0x0022,
    GPUPE_SPARE35 = 0x0023,
    GPUPE_SPARE36 = 0x0024,
    GPUPE_SPARE37 = 0x0025,
    GPUPE_SPARE38 = 0x0026,
    GPUPE_SPARE39 = 0x0027,
    GPUPE_TE_SU_IN_VALID = 0x0028,
    GPUPE_TE_SU_IN_READ = 0x0029,
    GPUPE_TE_SU_IN_PRIM = 0x002a,
    GPUPE_TE_SU_IN_EOP = 0x002b,
    GPUPE_TE_SU_IN_NULL_PRIM = 0x002c,
    GPUPE_TE_WK_IN_VALID = 0x002d,
    GPUPE_TE_WK_IN_READ = 0x002e,
    GPUPE_TE_OUT_PRIM_VALID = 0x002f,
    GPUPE_TE_OUT_PRIM_READ = 0x0030,
};

enum GPUPERFEVENT_TP {
    GPUPE_POINT_QUADS = 0x0000,
    GPUPE_BILIN_QUADS = 0x0001,
    GPUPE_ANISO_QUADS = 0x0002,
    GPUPE_MIP_QUADS = 0x0003,
    GPUPE_VOL_QUADS = 0x0004,
    GPUPE_MIP_VOL_QUADS = 0x0005,
    GPUPE_MIP_ANISO_QUADS = 0x0006,
    GPUPE_VOL_ANISO_QUADS = 0x0007,
    GPUPE_ANISO_2_1_QUADS = 0x0008,
    GPUPE_ANISO_4_1_QUADS = 0x0009,
    GPUPE_ANISO_6_1_QUADS = 0x000a,
    GPUPE_ANISO_8_1_QUADS = 0x000b,
    GPUPE_ANISO_10_1_QUADS = 0x000c,
    GPUPE_ANISO_12_1_QUADS = 0x000d,
    GPUPE_ANISO_14_1_QUADS = 0x000e,
    GPUPE_ANISO_16_1_QUADS = 0x000f,
    GPUPE_MIP_VOL_ANISO_QUADS = 0x0010,
    GPUPE_ALIGN_2_QUADS = 0x0011,
    GPUPE_ALIGN_4_QUADS = 0x0012,
    GPUPE_PIX_0_QUAD = 0x0013,
    GPUPE_PIX_1_QUAD = 0x0014,
    GPUPE_PIX_2_QUAD = 0x0015,
    GPUPE_PIX_3_QUAD = 0x0016,
    GPUPE_PIX_4_QUAD = 0x0017,
    GPUPE_TP_MIPMAP_LOD0 = 0x0018,
    GPUPE_TP_MIPMAP_LOD1 = 0x0019,
    GPUPE_TP_MIPMAP_LOD2 = 0x001a,
    GPUPE_TP_MIPMAP_LOD3 = 0x001b,
    GPUPE_TP_MIPMAP_LOD4 = 0x001c,
    GPUPE_TP_MIPMAP_LOD5 = 0x001d,
    GPUPE_TP_MIPMAP_LOD6 = 0x001e,
    GPUPE_TP_MIPMAP_LOD7 = 0x001f,
    GPUPE_TP_MIPMAP_LOD8 = 0x0020,
    GPUPE_TP_MIPMAP_LOD9 = 0x0021,
    GPUPE_TP_MIPMAP_LOD10 = 0x0022,
    GPUPE_TP_MIPMAP_LOD11 = 0x0023,
    GPUPE_TP_MIPMAP_LOD12 = 0x0024,
    GPUPE_TP_MIPMAP_LOD13 = 0x0025,
    GPUPE_TP_MIPMAP_LOD14 = 0x0026,
};

enum GPUPERFEVENT_PA_SC {
    GPUPE_SC_SR_WINDOW_VALID = 0x0000,
    GPUPE_SC_CW_WINDOW_VALID = 0x0001,
    GPUPE_SC_QM_WINDOW_VALID = 0x0002,
    GPUPE_SC_QPP_WINDOW_VALID = 0x0003,
    GPUPE_SC_ITER_WINDOW_VALID = 0x0004,
    GPUPE_SC_STARVED_BY_PA = 0x0005,
    GPUPE_SC_STARVED_BY_RCC = 0x0006,
    GPUPE_SC_STALLED_BY_PRIM_FF = 0x0007,
    GPUPE_SC_STALLED_BY_RCC = 0x0008,
    GPUPE_SC_STALLED_BY_BC = 0x0009,
    GPUPE_SC_STALLED_BY_SX = 0x000a,
    GPUPE_SC_STALLED_BY_SX0 = 0x000b,
    GPUPE_SC_STALLED_BY_SX1 = 0x000c,
    GPUPE_SC_STALLED_BY_SQ = 0x000d,
    GPUPE_SC_STALLED_BY_SP = 0x000e,
    GPUPE_SC_WAIT_FOR_R1 = 0x000f,
    GPUPE_SC_SCISSOR_DISCARD = 0x0010,
    GPUPE_SC_BB_DISCARD = 0x0011,
    GPUPE_SC_SUPERT_PRIM_DISCARD = 0x0012,
    GPUPE_SC_RT_PRIM = 0x0013,
    GPUPE_SC_TILE_VALID = 0x0014,
    GPUPE_SC_TILE_PER_PRIM_H0 = 0x0015,
    GPUPE_SC_TILE_PER_PRIM_H1 = 0x0016,
    GPUPE_SC_TILE_PER_PRIM_H2 = 0x0017,
    GPUPE_SC_TILE_PER_PRIM_H3 = 0x0018,
    GPUPE_SC_TILE_PER_PRIM_H4 = 0x0019,
    GPUPE_SC_TILE_PER_PRIM_H5 = 0x001a,
    GPUPE_SC_TILE_PER_PRIM_H6 = 0x001b,
    GPUPE_SC_TILE_PER_PRIM_H7 = 0x001c,
    GPUPE_SC_TILE_PER_PRIM_H8 = 0x001d,
    GPUPE_SC_TILE_PER_PRIM_H9 = 0x001e,
    GPUPE_SC_TILE_PER_PRIM_H10 = 0x001f,
    GPUPE_SC_TILE_PER_PRIM_H11 = 0x0020,
    GPUPE_SC_TILE_PER_PRIM_H12 = 0x0021,
    GPUPE_SC_TILE_PER_PRIM_H13 = 0x0022,
    GPUPE_SC_TILE_PER_PRIM_H14 = 0x0023,
    GPUPE_SC_TILE_PER_PRIM_H15 = 0x0024,
    GPUPE_SC_TILE_PER_PRIM_H16 = 0x0025,
    GPUPE_SC_SUPERT_TILE_DISCARD = 0x0026,
    GPUPE_SC_QM_NUM_QUADS = 0x0027,
    GPUPE_SC_QM_MASK_H0 = 0x0028,
    GPUPE_SC_QM_MASK_H1 = 0x0029,
    GPUPE_SC_QM_MASK_H2 = 0x002a,
    GPUPE_SC_QM_MASK_H3 = 0x002b,
    GPUPE_SC_QM_MASK_H4 = 0x002c,
    GPUPE_SC_QM_MASK_H5 = 0x002d,
    GPUPE_SC_QM_MASK_H6 = 0x002e,
    GPUPE_SC_QM_MASK_H7 = 0x002f,
    GPUPE_SC_QM_MASK_H8 = 0x0030,
    GPUPE_SC_QM_MASK_H9 = 0x0031,
    GPUPE_SC_QM_MASK_H10 = 0x0032,
    GPUPE_SC_QM_MASK_H11 = 0x0033,
    GPUPE_SC_QM_MASK_H12 = 0x0034,
    GPUPE_SC_QM_MASK_H13 = 0x0035,
    GPUPE_SC_QM_MASK_H14 = 0x0036,
    GPUPE_SC_QM_MASK_H15 = 0x0037,
    GPUPE_SC_QM_MASK_H16 = 0x0038,
    GPUPE_SC_QM_COVERED_H0 = 0x0039,
    GPUPE_SC_QM_COVERED_H1 = 0x003a,
    GPUPE_SC_QM_COVERED_H2 = 0x003b,
    GPUPE_SC_QM_COVERED_H3 = 0x003c,
    GPUPE_SC_QM_COVERED_H4 = 0x003d,
    GPUPE_SC_QM_COVERED_H5 = 0x003e,
    GPUPE_SC_QM_COVERED_H6 = 0x003f,
    GPUPE_SC_QM_COVERED_H7 = 0x0040,
    GPUPE_SC_QM_COVERED_H8 = 0x0041,
    GPUPE_SC_QM_COVERED_H9 = 0x0042,
    GPUPE_SC_QM_COVERED_H10 = 0x0043,
    GPUPE_SC_QM_COVERED_H11 = 0x0044,
    GPUPE_SC_QM_COVERED_H12 = 0x0045,
    GPUPE_SC_QM_COVERED_H13 = 0x0046,
    GPUPE_SC_QM_COVERED_H14 = 0x0047,
    GPUPE_SC_QM_COVERED_H15 = 0x0048,
    GPUPE_SC_QM_COVERED_H16 = 0x0049,
    GPUPE_SC_HIER_NUM_QUADS = 0x004a,
    GPUPE_SC_HIER_MASK_H0 = 0x004b,
    GPUPE_SC_HIER_MASK_H1 = 0x004c,
    GPUPE_SC_HIER_MASK_H2 = 0x004d,
    GPUPE_SC_HIER_MASK_H3 = 0x004e,
    GPUPE_SC_HIER_MASK_H4 = 0x004f,
    GPUPE_SC_HIER_MASK_H5 = 0x0050,
    GPUPE_SC_HIER_MASK_H6 = 0x0051,
    GPUPE_SC_HIER_MASK_H7 = 0x0052,
    GPUPE_SC_HIER_MASK_H8 = 0x0053,
    GPUPE_SC_HIER_MASK_H9 = 0x0054,
    GPUPE_SC_HIER_MASK_H10 = 0x0055,
    GPUPE_SC_HIER_MASK_H11 = 0x0056,
    GPUPE_SC_HIER_MASK_H12 = 0x0057,
    GPUPE_SC_HIER_MASK_H13 = 0x0058,
    GPUPE_SC_HIER_MASK_H14 = 0x0059,
    GPUPE_SC_HIER_MASK_H15 = 0x005a,
    GPUPE_SC_HIER_MASK_H16 = 0x005b,
    GPUPE_SC_DET_NUM_QUADS = 0x005c,
    GPUPE_SC_PKR_QD_PER_ROW_H1 = 0x005d,
    GPUPE_SC_PKR_QD_PER_ROW_H2 = 0x005e,
    GPUPE_SC_PKR_QD_PER_ROW_H3 = 0x005f,
    GPUPE_SC_PKR_QD_PER_ROW_H4 = 0x0060,
    GPUPE_SC_PKR_END_OF_VECTOR = 0x0061,
    GPUPE_SC_PKR_ONE_CLK = 0x0062,
    GPUPE_SC_QD_WITH_1_PIX = 0x0063,
    GPUPE_SC_QD_WITH_2_PIX = 0x0064,
    GPUPE_SC_QD_WITH_3_PIX = 0x0065,
    GPUPE_SC_QD_WITH_4_PIX = 0x0066,
    GPUPE_SC_SR_WINDOW_VALID_BUSY = 0x0067,
    GPUPE_SC_CW_WINDOW_VALID_BUSY = 0x0068,
    GPUPE_SC_QM_WINDOW_VALID_BUSY = 0x0069,
    GPUPE_SC_QPP_WINDOW_VALID_BUSY = 0x006a,
    GPUPE_SC_ITER_WINDOW_VALID_BUSY = 0x006b,
};

enum GPUPERFEVENT_MH {
    GPUPE_CP_READ_MEMORY = 0x0000,
    GPUPE_CP_READ_PGLB = 0x0001,
    GPUPE_CP_WRITE_MEMORY = 0x0002,
    GPUPE_CP_WRITE_SNOOPED = 0x0003,
    GPUPE_CP_WRITE_WRITEBACK = 0x0004,
    GPUPE_MH_CP_RTR = 0x0005,
    GPUPE_VGT_READ_MEMORY = 0x0006,
    GPUPE_VGT_READ_PGLB = 0x0007,
    GPUPE_MH_VGT_RTR = 0x0008,
    GPUPE_IOC_READ = 0x0009,
    GPUPE_IOC_WRITE = 0x000a,
    GPUPE_IOC_READ_BYTE_COUNT = 0x000b,
    GPUPE_IOC_WRITE_BYTE_COUNT = 0x000c,
    GPUPE_IOC_URGENT = 0x000d,
    GPUPE_MH_IOC_RTR = 0x000e,
    GPUPE_TC0_READ = 0x000f,
    GPUPE_MH_TC0_RTR = 0x0010,
    GPUPE_TC1_READ = 0x0011,
    GPUPE_MH_TC1_RTR = 0x0012,
    GPUPE_VC0_READ_MEMORY = 0x0013,
    GPUPE_VC0_READ_PGLB = 0x0014,
    GPUPE_MH_VC0_RTR = 0x0015,
    GPUPE_VC1_READ_MEMORY = 0x0016,
    GPUPE_VC1_READ_PGLB = 0x0017,
    GPUPE_MH_VC1_RTR = 0x0018,
    GPUPE_BC0_CP_WRITE = 0x0019,
    GPUPE_BC0_EX_WRITE = 0x001a,
    GPUPE_MH_BC0_RTR = 0x001b,
    GPUPE_BC1_CP_WRITE = 0x001c,
    GPUPE_BC1_EX_WRITE = 0x001d,
    GPUPE_MH_BC1_RTR = 0x001e,
    GPUPE_DC_GRAPHICS_REQ = 0x001f,
    GPUPE_DC_OVERLAY_REQ = 0x0020,
    GPUPE_DC_URGENT = 0x0021,
    GPUPE_PGLB_BIU_REQ = 0x0022,
    GPUPE_BIU_PGL_READ_DATA = 0x0023,
    GPUPE_PGL_MHS_READ_DATA = 0x0024,
    GPUPE_MH_MC0_READ_REQS = 0x0025,
    GPUPE_MH_MC0_WRITE_REQS = 0x0026,
    GPUPE_MH_MC1_READ_REQS = 0x0027,
    GPUPE_MH_MC1_WRITE_REQS = 0x0028,
    GPUPE_MC0_MH_READ_DATA = 0x0029,
    GPUPE_MC1_MH_READ_DATA = 0x002a,
    GPUPE_MH_CP_SEND = 0x002b,
    GPUPE_MH_VGT_SEND = 0x002c,
    GPUPE_MH_IOC_SEND = 0x002d,
    GPUPE_MH_TC0_SEND = 0x002e,
    GPUPE_MH_TC1_SEND = 0x002f,
    GPUPE_MH_VC0_SEND = 0x0030,
    GPUPE_MH_VC1_SEND = 0x0031,
    GPUPE_MH_DC_SEND = 0x0032,
    GPUPE_DC0_INFLIGHT_FULL = 0x0033,
    GPUPE_DC1_INFLIGHT_FULL = 0x0034,
    GPUPE_VC0_INFLIGHT_FULL = 0x0035,
    GPUPE_VC1_INFLIGHT_FULL = 0x0036,
    GPUPE_TC0_INFLIGHT_FULL = 0x0037,
    GPUPE_TC1_INFLIGHT_FULL = 0x0038,
    GPUPE_CP0_INFLIGHT_FULL = 0x0039,
    GPUPE_CP1_INFLIGHT_FULL = 0x003a,
    GPUPE_VGT0_INFLIGHT_FULL = 0x003b,
    GPUPE_VGT1_INFLIGHT_FULL = 0x003c,
    GPUPE_SB0_INFLIGHT_FULL = 0x003d,
    GPUPE_SB1_INFLIGHT_FULL = 0x003e,
    GPUPE_VCPGL_INFLIGHT_FULL = 0x003f,
    GPUPE_CPPGL_INFLIGHT_FULL = 0x0040,
    GPUPE_VGTPGL_INFLIGHT_FULL = 0x0041,
    GPUPE_MC0_DC_Q_FULL = 0x0042,
    GPUPE_MC0_VC_Q_FULL = 0x0043,
    GPUPE_MC0_TC_Q_FULL = 0x0044,
    GPUPE_MC0_CP_Q_FULL = 0x0045,
    GPUPE_MC0_SB_Q_FULL = 0x0046,
    GPUPE_MC0_VGT_Q_FULL = 0x0047,
    GPUPE_MC0_BCCP_Q_FULL = 0x0048,
    GPUPE_MC0_BCEX_Q_FULL = 0x0049,
    GPUPE_MC1_DC_Q_FULL = 0x004a,
    GPUPE_MC1_VC_Q_FULL = 0x004b,
    GPUPE_MC1_TC_Q_FULL = 0x004c,
    GPUPE_MC1_CP_Q_FULL = 0x004d,
    GPUPE_MC1_SB_Q_FULL = 0x004e,
    GPUPE_MC1_VGT_Q_FULL = 0x004f,
    GPUPE_MC1_BCCP_Q_FULL = 0x0050,
    GPUPE_MC1_BCEX_Q_FULL = 0x0051,
    GPUPE_CP_PGL_CACHE_HIT = 0x0052,
    GPUPE_VGT_PGL_CACHE_HIT = 0x0053,
    GPUPE_VC_PGL_CACHE_HIT = 0x0054,
    GPUPE_CP_PGL_FULL = 0x0055,
    GPUPE_VGT_PGL_FULL = 0x0056,
    GPUPE_VC0_PGL_FULL = 0x0057,
    GPUPE_VC1_PGL_FULL = 0x0058,
    GPUPE_MC0_WDB_FULL = 0x0059,
    GPUPE_MC0_TAGBUF_FULL = 0x005a,
    GPUPE_MC1_WDB_FULL = 0x005b,
    GPUPE_MC1_TAGBUF_FULL = 0x005c,
    GPUPE_PGL_TAGBUF_FULL = 0x005d,
    GPUPE_CP_WRITENOTIFY = 0x005e,
    GPUPE_BC_WRITENOTIFY = 0x005f,
    GPUPE_IOC_SYNC = 0x0060,
    GPUPE_PGL_BIU_LATENCY = 0x0061,
    GPUPE_MH_MC0_LATENCY = 0x0062,
    GPUPE_MH_MC0_DC_LATENCY = 0x0063,
    GPUPE_MH_MC0_VC_LATENCY = 0x0064,
    GPUPE_MH_MC0_TC_LATENCY = 0x0065,
    GPUPE_MH_MC0_CP_LATENCY = 0x0066,
    GPUPE_MH_MC0_SB_LATENCY = 0x0067,
    GPUPE_MH_MC0_VGT_LATENCY = 0x0068,
    GPUPE_MH_MC1_LATENCY = 0x0069,
    GPUPE_MH_MC1_DC_LATENCY = 0x006a,
    GPUPE_MH_MC1_VC_LATENCY = 0x006b,
    GPUPE_MH_MC1_TC_LATENCY = 0x006c,
    GPUPE_MH_MC1_CP_LATENCY = 0x006d,
    GPUPE_MH_MC1_SB_LATENCY = 0x006e,
    GPUPE_MH_MC1_VGT_LATENCY = 0x006f,
    GPUPE_MH_PGL_LATENCY = 0x0070,
    GPUPE_MH_PGL_CP_LATENCY = 0x0071,
    GPUPE_MH_PGL_VC_LATENCY = 0x0072,
    GPUPE_MH_PGL_VGT_LATENCY = 0x0073,
    GPUPE_TC0_RDY_AND_NOROOM = 0x0074,
    GPUPE_TC1_RDY_AND_NOROOM = 0x0075,
    GPUPE_IOC_RDY_AND_NOROOM = 0x0076,
};

enum GPUPERFEVENT_SX {
    GPUPE_SX_SC_QUADS = 0x0000,
    GPUPE_SX_SC_QUAD_FIFO_FULL = 0x0001,
    GPUPE_SX_EXPORT_VECTORS = 0x0002,
    GPUPE_SX_DUMMY_QUADS = 0x0003,
    GPUPE_SX_ALPHA_FAIL = 0x0004,
    GPUPE_SX_RB_QUAD_BUSY = 0x0005,
    GPUPE_SX_RB_COLOR_BUSY = 0x0006,
    GPUPE_SX_RB_QUAD_STALL = 0x0007,
    GPUPE_SX_RB_COLOR_STALL = 0x0008,
};

enum GPUPERFEVENT_TCM {
    GPUPE_QUAD0_RD_LAT_FIFO_EMPTY = 0x0000,
    GPUPE_QUAD0_RD_LAT_FIFO_16TH_FULL = 0x0001,
    GPUPE_QUAD0_RD_LAT_FIFO_8TH_FULL = 0x0002,
    GPUPE_QUAD0_RD_LAT_FIFO_4TH_FULL = 0x0003,
    GPUPE_QUAD0_RD_LAT_FIFO_HALF_FULL = 0x0004,
    GPUPE_QUAD0_RD_LAT_FIFO_FULL = 0x0005,
    GPUPE_QUAD0_RD_LAT_FIFO_LT_16TH_FULL = 0x0006,
    GPUPE_QUAD1_RD_LAT_FIFO_EMPTY = 0x0007,
    GPUPE_QUAD1_RD_LAT_FIFO_16TH_FULL = 0x0008,
    GPUPE_QUAD1_RD_LAT_FIFO_8TH_FULL = 0x0009,
    GPUPE_QUAD1_RD_LAT_FIFO_4TH_FULL = 0x000a,
    GPUPE_QUAD1_RD_LAT_FIFO_HALF_FULL = 0x000b,
    GPUPE_QUAD1_RD_LAT_FIFO_FULL = 0x000c,
    GPUPE_QUAD1_RD_LAT_FIFO_LT_16TH_FULL = 0x000d,
    GPUPE_QUAD2_RD_LAT_FIFO_EMPTY = 0x000e,
    GPUPE_QUAD2_RD_LAT_FIFO_16TH_FULL = 0x000f,
    GPUPE_QUAD2_RD_LAT_FIFO_8TH_FULL = 0x0010,
    GPUPE_QUAD2_RD_LAT_FIFO_4TH_FULL = 0x0011,
    GPUPE_QUAD2_RD_LAT_FIFO_HALF_FULL = 0x0012,
    GPUPE_QUAD2_RD_LAT_FIFO_FULL = 0x0013,
    GPUPE_QUAD2_RD_LAT_FIFO_LT_16TH_FULL = 0x0014,
    GPUPE_QUAD3_RD_LAT_FIFO_EMPTY = 0x0015,
    GPUPE_QUAD3_RD_LAT_FIFO_16TH_FULL = 0x0016,
    GPUPE_QUAD3_RD_LAT_FIFO_8TH_FULL = 0x0017,
    GPUPE_QUAD3_RD_LAT_FIFO_4TH_FULL = 0x0018,
    GPUPE_QUAD3_RD_LAT_FIFO_HALF_FULL = 0x0019,
    GPUPE_QUAD3_RD_LAT_FIFO_FULL = 0x001a,
    GPUPE_QUAD3_RD_LAT_FIFO_LT_16TH_FULL = 0x001b,
    GPUPE_READ_STARVED_QUAD0 = 0x001c,
    GPUPE_READ_STARVED_QUAD1 = 0x001d,
    GPUPE_READ_STARVED_QUAD2 = 0x001e,
    GPUPE_READ_STARVED_QUAD3 = 0x001f,
    GPUPE_READ_STARVED = 0x0020,
    GPUPE_READ_STALLED_QUAD0 = 0x0021,
    GPUPE_READ_STALLED_QUAD1 = 0x0022,
    GPUPE_READ_STALLED_QUAD2 = 0x0023,
    GPUPE_READ_STALLED_QUAD3 = 0x0024,
    GPUPE_READ_STALLED = 0x0025,
    GPUPE_VALID_READ_QUAD0 = 0x0026,
    GPUPE_VALID_READ_QUAD1 = 0x0027,
    GPUPE_VALID_READ_QUAD2 = 0x0028,
    GPUPE_VALID_READ_QUAD3 = 0x0029,
    GPUPE_TC_TP_STARVED_QUAD0 = 0x002a,
    GPUPE_TC_TP_STARVED_QUAD1 = 0x002b,
    GPUPE_TC_TP_STARVED_QUAD2 = 0x002c,
    GPUPE_TC_TP_STARVED_QUAD3 = 0x002d,
    GPUPE_TC_TP_STARVED = 0x002e,
};

enum GPUPERFEVENT_BIF {
    GPUPE_0 = 0x0000,
    GPUPE_1 = 0x0001,
    GPUPE_2 = 0x0002,
    GPUPE_3 = 0x0003,
    GPUPE_4 = 0x0004,
    GPUPE_5 = 0x0005,
};

enum GPUPERFEVENT_CP {
    GPUPE_CP_COUNT = 0x0000,
    GPUPE_CP_RBIU_STALL = 0x0001,
    GPUPE_CP_RBIU_TAF = 0x0002,
    GPUPE_CP_PFP_STALL = 0x0003,
    GPUPE_CP_RESERVED0 = 0x0004,
    GPUPE_CP_RESERVED1 = 0x0005,
    GPUPE_CP_NRT_RCIU_STALL = 0x0006,
    GPUPE_CP_Reserved2 = 0x0007,
    GPUPE_CP_NRT_MIU_STALL = 0x0008,
    GPUPE_CP_CSF_PFP_I1_FULL = 0x0009,
    GPUPE_CP_CSF_PFP_I2_FULL = 0x000a,
    GPUPE_CP_CSF_PFP_REQ_FULL = 0x000b,
    GPUPE_CP_RESERVED3 = 0x000c,
    GPUPE_CP_RING_ROQ_FULL = 0x000d,
    GPUPE_CP_I1_ROQ_FULL = 0x000e,
    GPUPE_CP_I2_ROQ_FULL = 0x000f,
    GPUPE_CP_ST_ROQ_FULL = 0x0010,
    GPUPE_CP_RTST_ROQ_FULL = 0x0011,
    GPUPE_CP_MIU_TAG_MEM_FULL = 0x0012,
    GPUPE_CP_MIU_WRITECLEAN = 0x0013,
    GPUPE_CP_RESERVED4 = 0x0014,
    GPUPE_CP_RESERVED5 = 0x0015,
    GPUPE_CP_NRT_WRITE_STALL = 0x0016,
    GPUPE_CP_NRT_READ_STALL = 0x0017,
    GPUPE_CP_WC_FIFO_FULL = 0x0018,
    GPUPE_CP_VTX_DEALLOC_FIFO_FULL = 0x0019,
    GPUPE_CP_PIX_DEALLOC_FIFO_FULL = 0x001a,
    GPUPE_CP_VTX_EVENT_FIFO_FULL = 0x001b,
    GPUPE_CP_PIX_EVENT_FIFO_FULL = 0x001c,
    GPUPE_CP_CF_EVENT_FIFO_FULL = 0x001d,
    GPUPE_CP_ME_RB_STARVED = 0x001e,
    GPUPE_CP_ME_I1_STARVED = 0x001f,
    GPUPE_CP_ME_I2_STARVED = 0x0020,
    GPUPE_CP_ME_ST_STARVED = 0x0021,
    GPUPE_CP_RESERVED6 = 0x0022,
    GPUPE_CP_RESERVED7 = 0x0023,
    GPUPE_CP_RESERVED8 = 0x0024,
    GPUPE_CP_RESERVED9 = 0x0025,
    GPUPE_CP_RESERVED10 = 0x0026,
    GPUPE_CP_RESERVED11 = 0x0027,
    GPUPE_RCIU_RBBM_DWORD_SENT = 0x0028,
    GPUPE_ME_PARSER_BUSY_CLOCKS = 0x0029,
    GPUPE_ME_WAIT_CONTEXT_AVAIL = 0x002a,
    GPUPE_PFP_TYPE0_PACKET = 0x002b,
    GPUPE_PFP_TYPE3_PACKET = 0x002c,
    GPUPE_CSF_RB_WPTR_NEQ_RPTR = 0x002d,
    GPUPE_CSF_I1_SIZE_NEQ_ZERO = 0x002e,
    GPUPE_CSF_I2_SIZE_NEQ_ZERO = 0x002f,
    GPUPE_CSF_RB_I1_I2_FETCHING = 0x0030,
    GPUPE_CP_RESERVED12 = 0x0031,
    GPUPE_CP_RESERVED13 = 0x0032,
    GPUPE_CP_RESERVED14 = 0x0033,
    GPUPE_CP_RESERVED15 = 0x0034,
    GPUPE_CP_RESERVED16 = 0x0035,
    GPUPE_CP_RESERVED17 = 0x0036,
    GPUPE_CP_RESERVED18 = 0x0037,
    GPUPE_CP_RESERVED19 = 0x0038,
    GPUPE_CP_RESERVED20 = 0x0039,
    GPUPE_CP_RESERVED21 = 0x003a,
    GPUPE_CP_RESERVED22 = 0x003b,
    GPUPE_CP_RESERVED23 = 0x003c,
    GPUPE_CP_RESERVED24 = 0x003d,
    GPUPE_CP_RESERVED25 = 0x003e,
    GPUPE_CP_RESERVED26 = 0x003f,
};

enum GPUPERFEVENT_TCF {
    GPUPE_VALID_CYCLES = 0x0000,
    GPUPE_SINGLE_PHASES = 0x0001,
    GPUPE_ANISO_PHASES = 0x0002,
    GPUPE_MIP_PHASES = 0x0003,
    GPUPE_VOL_PHASES = 0x0004,
    GPUPE_MIP_VOL_PHASES = 0x0005,
    GPUPE_MIP_ANISO_PHASES = 0x0006,
    GPUPE_VOL_ANISO_PHASES = 0x0007,
    GPUPE_ANISO_2_1_PHASES = 0x0008,
    GPUPE_ANISO_4_1_PHASES = 0x0009,
    GPUPE_ANISO_6_1_PHASES = 0x000a,
    GPUPE_ANISO_8_1_PHASES = 0x000b,
    GPUPE_ANISO_10_1_PHASES = 0x000c,
    GPUPE_ANISO_12_1_PHASES = 0x000d,
    GPUPE_ANISO_14_1_PHASES = 0x000e,
    GPUPE_ANISO_16_1_PHASES = 0x000f,
    GPUPE_MIP_VOL_ANISO_PHASES = 0x0010,
    GPUPE_ALIGN_2_PHASES = 0x0011,
    GPUPE_ALIGN_4_PHASES = 0x0012,
    GPUPE_TPC_BUSY = 0x0013,
    GPUPE_TPC_STALLED = 0x0014,
    GPUPE_TPC_STARVED = 0x0015,
    GPUPE_TPC_WORKING = 0x0016,
    GPUPE_TPC_WALKER_BUSY = 0x0017,
    GPUPE_TPC_WALKER_STALLED = 0x0018,
    GPUPE_TPC_WALKER_WORKING = 0x0019,
    GPUPE_TPC_ALIGNER_BUSY = 0x001a,
    GPUPE_TPC_ALIGNER_STALLED = 0x001b,
    GPUPE_TPC_ALIGNER_STALLED_BY_BLEND = 0x001c,
    GPUPE_TPC_ALIGNER_STALLED_BY_CACHE = 0x001d,
    GPUPE_TPC_ALIGNER_WORKING = 0x001e,
    GPUPE_TPC_BLEND_BUSY = 0x001f,
    GPUPE_TPC_BLEND_SYNC = 0x0020,
    GPUPE_TPC_BLEND_STARVED = 0x0021,
    GPUPE_TPC_BLEND_WORKING = 0x0022,
    GPUPE_OPCODE_0X00 = 0x0023,
    GPUPE_OPCODE_0X01 = 0x0024,
    GPUPE_OPCODE_0X04 = 0x0025,
    GPUPE_OPCODE_0X10 = 0x0026,
    GPUPE_OPCODE_0X11 = 0x0027,
    GPUPE_OPCODE_0X12 = 0x0028,
    GPUPE_OPCODE_0X13 = 0x0029,
    GPUPE_OPCODE_0X18 = 0x002a,
    GPUPE_OPCODE_0X19 = 0x002b,
    GPUPE_OPCODE_0X1A = 0x002c,
    GPUPE_OPCODE_OTHER = 0x002d,
    GPUPE_RESERVED_46 = 0x002e,
    GPUPE_RESERVED_47 = 0x002f,
    GPUPE_RESERVED_48 = 0x0030,
    GPUPE_RESERVED_49 = 0x0031,
    GPUPE_RESERVED_50 = 0x0032,
    GPUPE_RESERVED_51 = 0x0033,
    GPUPE_RESERVED_52 = 0x0034,
    GPUPE_RESERVED_53 = 0x0035,
    GPUPE_RESERVED_54 = 0x0036,
    GPUPE_RESERVED_55 = 0x0037,
    GPUPE_IN_FIFO_0_EMPTY = 0x0038,
    GPUPE_IN_FIFO_0_LT_HALF_FULL = 0x0039,
    GPUPE_IN_FIFO_0_HALF_FULL = 0x003a,
    GPUPE_IN_FIFO_0_FULL = 0x003b,
    GPUPE_IN_FIFO_1_EMPTY = 0x003c,
    GPUPE_IN_FIFO_1_LT_HALF_FULL = 0x003d,
    GPUPE_IN_FIFO_1_HALF_FULL = 0x003e,
    GPUPE_IN_FIFO_1_FULL = 0x003f,
    GPUPE_IN_FIFO_2_EMPTY = 0x0040,
    GPUPE_IN_FIFO_2_LT_HALF_FULL = 0x0041,
    GPUPE_IN_FIFO_2_HALF_FULL = 0x0042,
    GPUPE_IN_FIFO_2_FULL = 0x0043,
    GPUPE_IN_FIFO_3_EMPTY = 0x0044,
    GPUPE_IN_FIFO_3_LT_HALF_FULL = 0x0045,
    GPUPE_IN_FIFO_3_HALF_FULL = 0x0046,
    GPUPE_IN_FIFO_3_FULL = 0x0047,
    GPUPE_IN_FIFO_TPC_EMPTY = 0x0048,
    GPUPE_IN_FIFO_TPC_LT_HALF_FULL = 0x0049,
    GPUPE_IN_FIFO_TPC_HALF_FULL = 0x004a,
    GPUPE_IN_FIFO_TPC_FULL = 0x004b,
    GPUPE_TPC_TC_XFC = 0x004c,
    GPUPE_TPC_TC_STATE = 0x004d,
    GPUPE_TC_STALL = 0x004e,
    GPUPE_QUAD0_TAPS = 0x004f,
    GPUPE_QUAD1_TAPS = 0x0050,
    GPUPE_QUAD2_TAPS = 0x0051,
    GPUPE_QUAD3_TAPS = 0x0052,
    GPUPE_QUADS = 0x0053,
    GPUPE_TCA_SYNC_STALL = 0x0054,
    GPUPE_TAG_STALL = 0x0055,
    GPUPE_SLICE_STALL = 0x0056,
    GPUPE_SKEW_STALL = 0x0057,
    GPUPE_TCB_SYNC_STALL = 0x0058,
    GPUPE_TCA_VALID = 0x0059,
    GPUPE_PROBES_VALID = 0x005a,
    GPUPE_MISS_STALL = 0x005b,
    GPUPE_FETCH_FIFO_STALL = 0x005c,
    GPUPE_TCO_STALL = 0x005d,
    GPUPE_ANY_STALL = 0x005e,
    GPUPE_TAG_MISSES = 0x005f,
    GPUPE_TAG_HITS = 0x0060,
    GPUPE_SUB_TAG_MISSES = 0x0061,
    GPUPE_SET0_INVALIDATES = 0x0062,
    GPUPE_SET1_INVALIDATES = 0x0063,
    GPUPE_SET2_INVALIDATES = 0x0064,
    GPUPE_SET3_INVALIDATES = 0x0065,
    GPUPE_SET0_TAG_MISSES = 0x0066,
    GPUPE_SET1_TAG_MISSES = 0x0067,
    GPUPE_SET2_TAG_MISSES = 0x0068,
    GPUPE_SET3_TAG_MISSES = 0x0069,
    GPUPE_SET0_TAG_HITS = 0x006a,
    GPUPE_SET1_TAG_HITS = 0x006b,
    GPUPE_SET2_TAG_HITS = 0x006c,
    GPUPE_SET3_TAG_HITS = 0x006d,
    GPUPE_SET0_SUB_TAG_MISSES = 0x006e,
    GPUPE_SET1_SUB_TAG_MISSES = 0x006f,
    GPUPE_SET2_SUB_TAG_MISSES = 0x0070,
    GPUPE_SET3_SUB_TAG_MISSES = 0x0071,
    GPUPE_SET0_EVICT1 = 0x0072,
    GPUPE_SET0_EVICT2 = 0x0073,
    GPUPE_SET0_EVICT3 = 0x0074,
    GPUPE_SET0_EVICT4 = 0x0075,
    GPUPE_SET0_EVICT5 = 0x0076,
    GPUPE_SET0_EVICT6 = 0x0077,
    GPUPE_SET0_EVICT7 = 0x0078,
    GPUPE_SET0_EVICT8 = 0x0079,
    GPUPE_SET0_EVICT9 = 0x007a,
    GPUPE_SET0_EVICT10 = 0x007b,
    GPUPE_SET0_EVICT11 = 0x007c,
    GPUPE_SET0_EVICT12 = 0x007d,
    GPUPE_SET0_EVICT13 = 0x007e,
    GPUPE_SET0_EVICT14 = 0x007f,
    GPUPE_SET0_EVICT15 = 0x0080,
    GPUPE_SET0_EVICT16 = 0x0081,
    GPUPE_SET1_EVICT1 = 0x0082,
    GPUPE_SET1_EVICT2 = 0x0083,
    GPUPE_SET1_EVICT3 = 0x0084,
    GPUPE_SET1_EVICT4 = 0x0085,
    GPUPE_SET1_EVICT5 = 0x0086,
    GPUPE_SET1_EVICT6 = 0x0087,
    GPUPE_SET1_EVICT7 = 0x0088,
    GPUPE_SET1_EVICT8 = 0x0089,
    GPUPE_SET1_EVICT9 = 0x008a,
    GPUPE_SET1_EVICT10 = 0x008b,
    GPUPE_SET1_EVICT11 = 0x008c,
    GPUPE_SET1_EVICT12 = 0x008d,
    GPUPE_SET1_EVICT13 = 0x008e,
    GPUPE_SET1_EVICT14 = 0x008f,
    GPUPE_SET1_EVICT15 = 0x0090,
    GPUPE_SET1_EVICT16 = 0x0091,
    GPUPE_SET2_EVICT1 = 0x0092,
    GPUPE_SET2_EVICT2 = 0x0093,
    GPUPE_SET2_EVICT3 = 0x0094,
    GPUPE_SET2_EVICT4 = 0x0095,
    GPUPE_SET2_EVICT5 = 0x0096,
    GPUPE_SET2_EVICT6 = 0x0097,
    GPUPE_SET2_EVICT7 = 0x0098,
    GPUPE_SET2_EVICT8 = 0x0099,
    GPUPE_SET2_EVICT9 = 0x009a,
    GPUPE_SET2_EVICT10 = 0x009b,
    GPUPE_SET2_EVICT11 = 0x009c,
    GPUPE_SET2_EVICT12 = 0x009d,
    GPUPE_SET2_EVICT13 = 0x009e,
    GPUPE_SET2_EVICT14 = 0x009f,
    GPUPE_SET2_EVICT15 = 0x00a0,
    GPUPE_SET2_EVICT16 = 0x00a1,
    GPUPE_SET3_EVICT1 = 0x00a2,
    GPUPE_SET3_EVICT2 = 0x00a3,
    GPUPE_SET3_EVICT3 = 0x00a4,
    GPUPE_SET3_EVICT4 = 0x00a5,
    GPUPE_SET3_EVICT5 = 0x00a6,
    GPUPE_SET3_EVICT6 = 0x00a7,
    GPUPE_SET3_EVICT7 = 0x00a8,
    GPUPE_SET3_EVICT8 = 0x00a9,
    GPUPE_SET3_EVICT9 = 0x00aa,
    GPUPE_SET3_EVICT10 = 0x00ab,
    GPUPE_SET3_EVICT11 = 0x00ac,
    GPUPE_SET3_EVICT12 = 0x00ad,
    GPUPE_SET3_EVICT13 = 0x00ae,
    GPUPE_SET3_EVICT14 = 0x00af,
    GPUPE_SET3_EVICT15 = 0x00b0,
    GPUPE_SET3_EVICT16 = 0x00b1,
    GPUPE_FF_EMPTY = 0x00b2,
    GPUPE_FF_LT_HALF_FULL = 0x00b3,
    GPUPE_FF_HALF_FULL = 0x00b4,
    GPUPE_FF_FULL = 0x00b5,
    GPUPE_FF_XFC = 0x00b6,
    GPUPE_FF_STALLED = 0x00b7,
    GPUPE_FG_MASKS = 0x00b8,
    GPUPE_FG_LEFT_MASKS = 0x00b9,
    GPUPE_FG_LEFT_MASK_STALLED = 0x00ba,
    GPUPE_FG_LEFT_NOT_DONE_STALL = 0x00bb,
    GPUPE_FG_LEFT_FG_STALL = 0x00bc,
    GPUPE_FG_LEFT_SECTORS = 0x00bd,
    GPUPE_FG_RIGHT_MASKS = 0x00be,
    GPUPE_FG_RIGHT_MASK_STALLED = 0x00bf,
    GPUPE_FG_RIGHT_NOT_DONE_STALL = 0x00c0,
    GPUPE_FG_RIGHT_FG_STALL = 0x00c1,
    GPUPE_FG_RIGHT_SECTORS = 0x00c2,
    GPUPE_FG0_REQUESTS = 0x00c3,
    GPUPE_FG0_STALLED = 0x00c4,
    GPUPE_FG1_REQUESTS = 0x00c5,
    GPUPE_FG1_STALLED = 0x00c6,
    GPUPE_MEM_REQ512 = 0x00c7,
    GPUPE_MEM_REQ_SENT = 0x00c8,
    GPUPE_MEM_AGP_READ_REQ = 0x00c9,
    GPUPE_MEM_LOCAL_READ_REQ = 0x00ca,
    GPUPE_TC0_MH_STALLED = 0x00cb,
    GPUPE_TC1_MH_STALLED = 0x00cc,
};

enum GPUPERFEVENT_BC {
    GPUPE_BC_CNTX0_BUSY = 0x0000,
    GPUPE_BC_CNTX17_BUSY = 0x0001,
    GPUPE_BC_RQ_STALLED = 0x0002,
    GPUPE_BC_AZ_STALLED = 0x0003,
    GPUPE_BC_MH_CPY_STALLED = 0x0004,
    GPUPE_BC_MH_EXP_STALLED = 0x0005,
    GPUPE_BC_SC_STARVED = 0x0006,
    GPUPE_BC_SX_STARVED = 0x0007,
    GPUPE_BC_ACC_COUNT = 0x0008,
    GPUPE_BC_DRAW_COUNT = 0x0009,
    GPUPE_BC_ACC2_COUNT = 0x000a,
    GPUPE_BC_DRAW2_COUNT = 0x000b,
    GPUPE_BC_SETZ_COUNT = 0x000c,
    GPUPE_BC_READ_COUNT = 0x000d,
    GPUPE_BC_READ_ACC_COUNT = 0x000e,
    GPUPE_BC_STATE_COUNT = 0x000f,
    GPUPE_BC_STATE2_COUNT = 0x0010,
    GPUPE_BC_COPY_WRITE_COUNT = 0x0011,
    GPUPE_BC_EXPORT_COUNT = 0x0012,
};
