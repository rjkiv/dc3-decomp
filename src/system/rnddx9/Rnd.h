#pragma once
#include "Memory.h"
#include "math/Color.h"
#include "movie/Splash.h"
#include "os/Debug.h"
#include "os/OSFuncs.h"
#include "rnddx9/Object.h"
#include "rnddx9/Tex.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Rnd_NG.h"
#include "xdk/D3D9.h"
#include "xdk/XGRAPHICS.h"
#include "xdk/xapilibi/xbase.h"
#include <types.h>

struct LargeQuadRenderData {
    D3DIndexBuffer *unk0;
    D3DVertexBuffer *unk4;
    int unk8;
    int unkc;
};

class DxRnd : public NgRnd {
public:
    enum RegisterAlloc {
        kNumRegAlloc = 4
    };
    DxRnd();
    virtual ~DxRnd();
    virtual DataNode Handle(DataArray *, bool);
    virtual void PreInit() { PreInit(nullptr); }
    virtual void Init() { Init(nullptr); }
    virtual void Terminate();
    virtual void Clear(unsigned int, const Hmx::Color &);
    virtual void DrawRect(
        const Hmx::Rect &,
        const Hmx::Color &,
        RndMat *,
        const Hmx::Color *,
        const Hmx::Color *
    );
    virtual Vector2 &
    DrawString(const char *, const Vector2 &, const Hmx::Color &, bool); // 0x80
    virtual void
    DrawLine(const Vector3 &, const Vector3 &, const Hmx::Color &, bool); // 0x84
    virtual void BeginDrawing();
    virtual void EndDrawing();
    virtual void MakeDrawTarget();
    virtual void SetSync(int sync);
    virtual RndTex *GetCurrentFrameTex(bool);
    virtual void CaptureNextGpuFrame() { unk3f6 = true; }
    virtual void SetAspect(Aspect a);
    virtual void SetShrinkToSafeArea(bool shrink);
    virtual void PushClipPlanesInternal(ObjPtrVec<RndTransformable> &);
    virtual void PopClipPlanesInternal(ObjPtrVec<RndTransformable> &);

    virtual void SetViewport(const Viewport &v);
    virtual void DrawRect(
        const Hmx::Rect &,
        RndMat *,
        ShaderType,
        const Hmx::Color &,
        const Hmx::Color *,
        const Hmx::Color *
    );
    virtual void DrawRectDepth(
        const Vector3 &, const Vector3 (&)[4], const Vector4 &, RndMat *, ShaderType
    );
    virtual bool Offscreen() const;
    virtual RndTex *PreProcessTexture() { return mPreProcessTex; }
    virtual RndTex *PostProcessTexture() { return mPostProcessTex; }
    virtual RndTex *PreDepthTexture() { return mPreDepthTex; }
    virtual void Suspend();
    virtual void Resume();
    virtual void CreateLargeQuad(int, int, LargeQuadRenderData &);
    virtual void
    DrawLargeQuad(const LargeQuadRenderData &, const Transform &, RndMat *, ShaderType);
    virtual void SetVertShaderTex(RndTex *, int);
    virtual void UpdateScalerParams();

    D3DDevice *Device() { return mD3DDevice; }
    void AutoRelease(D3DResource *r) {
        if (r) {
            if (unk1b4) {
                MILO_ASSERT(CurrentThreadId() != TheSplasher->SplashThreadId(), 0xF4);
                D3DResource_Release(r);
            } else {
                unk304.push_back(r);
            }
        }
    }
    void AutoDelete(D3DBaseTexture *t) {
        if (t) {
            if (unk1b4) {
                MILO_ASSERT(CurrentThreadId() != TheSplasher->SplashThreadId(), 0x105);
                UINT data;
                XGGetTextureLayout(
                    t,
                    &data,
                    nullptr,
                    nullptr,
                    nullptr,
                    0,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr,
                    0
                );
                PhysicalFreeTracked((void *)data, __FILE__, 0x109, "");
                delete t;
            } else {
                unk310.push_back(t);
            }
        }
    }

    u8 Unk301() const { return unk_0x301; }
    D3DSurface *BackBuffer() const;
    void PreInit(HWND__ *);
    void Init(HWND__ *);
    D3DTexture *FrontBuffer();
    D3DTexture *NotFrontBuffer();
    void ReleaseAutoRelease();
    void InitRenderState();
    D3DFORMAT D3DFormatForBitmap(const RndBitmap &);
    int BitmapOrderForD3DFormat(D3DFORMAT);
    long GetDeviceCaps(D3DCAPS9 *);
    void Present();
    void SetDefaultRenderStates();
    void SetShaderRegisterAlloc(RegisterAlloc);

    static const char *Error(long);

    XVIDEO_MODE *VideoMode() { return &mVideoMode; }

protected:
    virtual void DoPostProcess();
    virtual bool CanModal(Debug::ModalType);
    virtual void ModalDraw(Debug::ModalType, const char *);

private:
    virtual void DoWorldEnd();

    void InitBuffers();
    void PreDeviceReset();
    void PostDeviceReset();
    void CreatePostTextures();
    void ResetDevice();
    void TerminateBuffers();
    void SetupGamma();
    void BeginTiling(const Hmx::Color &, float, unsigned int);
    void PerfCountersInit();
    void PerfCountersStart();
    void PerfCountersStop();
    void EndTiling(D3DBaseTexture *, int);
    void SavePreBuffer();
    void SavePostBuffer();
    void SetFrameBuffersAsSource();
    void FinishPostProcess();
    void CopyPostProcess();
    void DoPointTests();
    void DrawSafeArea(float, bool, const Hmx::Color &);

    // static D3DXMATRIX sIdentityMtx;

    int unk220;
    D3DDevice *mD3DDevice; // 0x224
    int unk228; // 0x228 - current thread id?
    void *unk22c;
    D3DDEVTYPE mDeviceType; // 0x230
    D3DPRESENT_PARAMETERS mPresentParams; // 0x234
    std::list<DxObject *> unk2b0;
    int unk2b8;
    int unk2bc;
    int unk2c0;
    int unk2c4;
    int unk2c8;
    int unk2cc;
    Timer unk2d0;
    bool unk300;
    u8 unk_0x301;
    std::vector<D3DResource *> unk304; // 0x304 - released resources?
    std::vector<D3DBaseTexture *> unk310; // 0x310 - deleted textures?
    XVIDEO_MODE mVideoMode; // 0x31c
    bool unk34c;
    bool unk34d;
    D3DTexture *mFrontBuffers[2]; // 0x350
    D3DTexture *mFrontBufferDepth; // 0x358
    int unk35c; // 0x35c - buffer idx?
    bool unk360;
    bool mAsyncSwapCurrent; // 0x361
    D3DPerfCounters *mPerfCounterStart; // 0x364
    D3DPerfCounters *mPerfCounterEnd; // 0x368
    Timer *mGPUTimer; // 0x36c
    float unk370;
    float unk374;
    bool mCreatedPerfCounters; // 0x378
    int unk37c; // 0x37c - flags
    D3DSurface *mBackBuffer; // 0x380
    D3DSurface *unk384;
    D3DSurface *unk388;
    D3DSurface *unk38c;
    D3DTexture *mPreProcessBuffer; // 0x390
    D3DTexture *mPostProcessBuffer; // 0x394
    DxTex *mPreProcessTex; // 0x398
    DxTex *mPostProcessTex; // 0x39c
    DxTex *mPreDepthTex; // 0x3a0
    bool unk3a4;
    unsigned int unk3a8;
    unsigned int unk3ac;
    int mNumTiles; // 0x3b0
    D3DRECT unk3b4; // 0x3b4
    int unk3c4;
    int unk3c8;
    int unk3cc;
    int unk3d0;
    int unk3d4;
    int unk3d8;
    int unk3dc;
    int unk3e0;
    int unk3e4;
    int unk3e8;
    int unk3ec;
    int unk3f0;
    bool unk3f4;
    bool unk3f5;
    bool unk3f6;
    bool unk3f7;
    RegisterAlloc mRegAlloc; // 0x3f8
    int mDefaultVSRegAlloc; // 0x3fc
    int mDefaultPSRegAlloc; // 0x400
    bool unk404;
    int unk408;
};

#define GPU_GPRS 0x80

extern DxRnd TheDxRnd;

int D3DFORMAT_BitsPerPixel(D3DFORMAT);

inline unsigned long MakeColor(const Hmx::Color &c) {
    return ((unsigned long)(c.alpha * 255.0f) & 0xFF) << 24
        | ((unsigned long)(c.red * 255.0f) & 0xFF) << 16
        | ((unsigned long)(c.green * 255.0f) & 0xFF) << 8
        | ((unsigned long)(c.blue * 255.0f) & 0xFF);
}

#define DX_RELEASE(x) (TheDxRnd.AutoRelease(x), x = nullptr)
#define DX_DELETE(x) (TheDxRnd.AutoDelete(x), x = nullptr)

inline HRESULT DxCheck(void *v) { return v ? ERROR_SUCCESS : E_OUTOFMEMORY; }

// check that the thing allocated successfully (e.g. no E_OUTOFMEMORY)
#define DX_ASSERT(cond, line)                                                            \
    {                                                                                    \
        HRESULT code = DxCheck(cond);                                                    \
        ((code)                                                                          \
         && (TheDebugFailer << MakeString(                                               \
                 "File: %s Line: %d Error: %s\n", __FILE__, line, DxRnd::Error(code)     \
             ),                                                                          \
             0));                                                                        \
    }

// check that the thing allocated successfully (e.g. no E_OUTOFMEMORY)
#define DX_ASSERT_CODE(code, line)                                                       \
    {                                                                                    \
        ((code)                                                                          \
         && (TheDebugFailer << MakeString(                                               \
                 "File: %s Line: %d Error: %s\n", __FILE__, line, DxRnd::Error(code)     \
             ),                                                                          \
             0));                                                                        \
    }
