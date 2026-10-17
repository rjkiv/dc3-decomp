#pragma once
#include "os/Memory.h"
#include "math/Color.h"
#include "movie/Splash.h"
#include "os/Debug.h"
#include "os/OSFuncs.h"
#include "rnddx9/Object.h"
#include "rnddx9/Tex.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Rnd_NG.h"
#include "xdk/D3D9.h"
#include "xdk/D3DX9.h"
#include "xdk/XGRAPHICS.h"
#include "xdk/d3d9i/d3d9.h"
#include "xdk/d3d9i/d3d9types.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/xbase.h"
#include <types.h>

class DxRnd : public NgRnd {
public:
    enum BufferConfig {
        kBufferConfig_Default = 0x0000,
        kBufferConfig_LoRes = 0x0001,
        kBufferConfig_Tiles_Horiz = 0x0002,
    };
    enum RegisterAlloc {
        kRegInit = 0x0000,
        kRegDefault = 0x0001,
        kRegPostProc = 0x0002,
        kRegStream = 0x0003,
        kNumRegAlloc = 0x0004,
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
    virtual void CaptureNextGpuFrame() { mCaptureNextGpuFrame = true; }
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
    virtual RndTex *PreProcessTexture() { return mPreProcessTexture; }
    virtual RndTex *PostProcessTexture() { return mPostProcessTexture; }
    virtual RndTex *PreDepthTexture() { return mPreDepthTexture; }
    virtual void Suspend();
    virtual void Resume();
    virtual void CreateLargeQuad(int, int, LargeQuadRenderData &);
    virtual void
    DrawLargeQuad(const LargeQuadRenderData &, const Transform &, RndMat *, ShaderType);
    virtual void SetVertShaderTex(RndTex *, int);
    virtual void UpdateScalerParams();

    D3DDevice *Device() const { return mD3DDevice; }
    void AutoRelease(D3DResource *r) {
        if (r) {
            if (mSplashing) {
                MILO_ASSERT(CurrentThreadId() != TheSplasher->SplashThreadId(), 0xF4);
                r->Release();
            } else {
                mAutoRel.push_back(r);
            }
        }
    }
    void AutoDelete(D3DBaseTexture *t) {
        if (t) {
            if (mSplashing) {
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
                mAutoDel.push_back(t);
            }
        }
    }

    D3DDevice *operator->() const { return mD3DDevice; }
    bool InverseZ() const { return mInverseZ; }
    void SetInverseZ(bool b) { mInverseZ = b; }
    D3DSurface *BackBuffer() const;
    void PreInit(HWND__ *);
    void Init(HWND__ *);
    D3DTexture *FrontBuffer();
    D3DTexture *NotFrontBuffer();
    void ReleaseAutoRelease();
    void InitRenderState();
    D3DFORMAT D3DFormatForBitmap(const RndBitmap &);
    int BitmapOrderForD3DFormat(D3DFORMAT);
    HRESULT GetDeviceCaps(D3DCAPS9 *);
    void Present();
    void SetDefaultRenderStates();
    void SetShaderRegisterAlloc(RegisterAlloc);

    static const char *Error(HRESULT);

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

    static D3DXMATRIX sIdentityMtx;

    Direct3D *mD3D; // 0x220
    D3DDevice *mD3DDevice; // 0x224
    DWORD mDeviceOwningThread; // 0x228
    HWND__ *mWnd; // 0x22c
    D3DDEVTYPE mDevType; // 0x230
    D3DPRESENT_PARAMETERS mPresentParams; // 0x234
    std::list<DxObject *> mObjects; // 0x2b0
    int mStatPolys; // 0x2b8
    int mStatMats; // 0x2bc
    int mStatMulti; // 0x2c0
    int mStatMeshes; // 0x2c4
    float mSyncMs; // 0x2c8
    Timer mSyncTimer; // 0x2d0
    bool mMaxFrameRate; // 0x300
    bool mInverseZ; // 0x301
    std::vector<D3DResource *> mAutoRel; // 0x304
    std::vector<D3DBaseTexture *> mAutoDel; // 0x310 - deleted textures?
    XVIDEO_MODE mVideoMode; // 0x31c
    bool mTiling; // 0x34c
    bool mHdrEnabled; // 0x34d
    D3DTexture *mFrontBuffer[2]; // 0x350
    D3DTexture *mDepthFrontBuffer; // 0x358
    unsigned int mCurrFront; // 0x35c
    bool mAsyncSwapDesired; // 0x360
    bool mAsyncSwapCurrent; // 0x361
    D3DPerfCounters *mPerfCounterStart; // 0x364
    D3DPerfCounters *mPerfCounterEnd; // 0x368
    Timer *mGPUTimer; // 0x36c
    float mMsGPUBusy; // 0x370
    float mMsGPUCycles; // 0x374
    bool mCreatedPerfCounters; // 0x378
    unsigned int mBackBufferFlags; // 0x37c
    D3DSurface *mBackBuffer_World; // 0x380
    D3DSurface *mBackBuffer_UI; // 0x384
    D3DSurface *mDepthBuffer_World; // 0x388
    D3DSurface *mDepthBuffer_UI; // 0x38c
    D3DTexture *mPreProcessBuffer; // 0x390
    D3DTexture *mPostProcessBuffer; // 0x394
    DxTex *mPreProcessTexture; // 0x398
    DxTex *mPostProcessTexture; // 0x39c
    DxTex *mPreDepthTexture; // 0x3a0
    bool mDrawingUI; // 0x3a4
    unsigned int mEDRamBase; // 0x3a8
    unsigned int mHierZBase; // 0x3ac
    int mNumTiles; // 0x3b0
    D3DRECT mTileRects[4]; // 0x3b4
    bool mSuspended; // 0x3f4
    bool mPrintGlitches; // 0x3f5
    bool mCaptureNextGpuFrame; // 0x3f6
    bool mGPUCaptureInProgress; // 0x3f7
    RegisterAlloc mRegisterAlloc; // 0x3f8
    int mDefaultVSRegAlloc; // 0x3fc
    int mDefaultPSRegAlloc; // 0x400
    bool mPreInitialized; // 0x404
    int mNumClipPlanes; // 0x408
};

extern DxRnd TheDxRnd;

#define GPU_GPRS 0x80

int D3DFORMAT_BitsPerPixel(D3DFORMAT);

inline DWORD MakeColor(const Hmx::Color &c) {
    return D3DCOLOR_COLORVALUE(c.red, c.green, c.blue, c.alpha);
}

#define DX_RELEASE(x) (TheDxRnd.AutoRelease(x), x = nullptr)
#define DX_DELETE(x) (TheDxRnd.AutoDelete(x), x = nullptr)

// check that the thing allocated successfully (e.g. no E_OUTOFMEMORY)
#define DX_ASSERT(code, line)                                                            \
    {                                                                                    \
        ((code)                                                                          \
         && (TheDebugFailer << MakeString(                                               \
                 "File: %s Line: %d Error: %s\n", __FILE__, line, DxRnd::Error(code)     \
             ),                                                                          \
             0));                                                                        \
    }
