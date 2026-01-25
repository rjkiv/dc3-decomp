#include "Cam.h"
#include "Env.h"
#include "Lit.h"
#include "Mat.h"
#include "Memory.h"
#include "Mesh.h"
#include "Movie.h"
#include "MultiMesh.h"
#include "Part.h"
#include "RenderState.h"
#include "Tex.h"
#include "TexRenderer.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "os/Timer.h"
#include "rnddx9/CubeTex.h"
#include "rnddx9/OcclusionQueryMgr.h"
#include "rnddx9/Rnd.h"
#include "rndobj/Cam.h"
#include "rndobj/DOFProc_NG.h"
#include "rndobj/Flare.h"
#include "rndobj/HiResScreen.h"
#include "rndobj/Mat_NG.h"
#include "rndobj/Overlay.h"
#include "rndobj/PostProc.h"
#include "rndobj/PostProc_NG.h"
#include "rndobj/Rnd.h"
#include "rndobj/Rnd_NG.h"
#include "rndobj/Shader.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/ShadowMap.h"
#include "rndobj/Stats_NG.h"
#include "rndobj/Tex.h"
#include "utl/MemTrack.h"
#include "utl/Option.h"
#include "xdk/D3D9.h"
#include "xdk/d3d9i/d3d9.h"
#include "xdk/d3d9i/d3d9caps.h"
#include "xdk/d3d9i/d3d9types.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/processthreadsapi.h"
#include "xdk/xapilibi/xbase.h"
#include "xdk/xapilibi/xbox.h"

void CreateBackBuffers(int, int, D3DMULTISAMPLE_TYPE, unsigned int &, unsigned int &, D3DSurface *&, D3DSurface *&);

DxRnd::DxRnd()
    : unk220(0), mD3DDevice(nullptr), unk22c(0), mDeviceType(D3DDEVTYPE_HAL),
      unk_0x301(1), unk360(0), mAsyncSwapCurrent(0), mPerfCounterStart(0),
      mPerfCounterEnd(0), mGPUTimer(0), unk370(0), unk374(0), mCreatedPerfCounters(0),
      unk3a4(0), unk3f4(0), unk3f7(0), unk404(0), unk408(0) {
    unk220 = 1;
    mFrontBuffers[0] = 0;
    mFrontBuffers[1] = 0;
    mBackBuffer = 0;
    unk388 = 0;
    unk384 = 0;
    unk38c = 0;
    unk37c = 0;
    unk35c = 0;
    mNumTiles = 0;
    unk34d = true;
}

DxRnd::~DxRnd() {
    if ((unsigned int)unk220) {
        unk220 = 0;
    }
}

void CDError() {
    TheDxRnd.Suspend();
    ShowDirtyDiscError();
}

void DxModal(Debug::ModalType &t, FixedString &s, bool b) { TheDxRnd.Modal(t, s, b); }

void DxRnd::PreInit(HWND__ *) {
    if (!unk404) {
        unk404 = true;
        DataArray *cfg = SystemConfig("rnd");
        mDefaultVSRegAlloc = 32;
        mDefaultPSRegAlloc = 96;
        DataArray *gprArr = cfg->FindArray("shader_gpr_alloc", false);
        if (gprArr) {
            mDefaultVSRegAlloc = gprArr->Int(1);
            mDefaultPSRegAlloc = gprArr->Int(2);
        }
        MILO_ASSERT(mDefaultVSRegAlloc + mDefaultPSRegAlloc == GPU_GPRS, 0x1F0);
        MILO_ASSERT(mDefaultVSRegAlloc >= 16, 0x1F1);
        MILO_ASSERT(mDefaultPSRegAlloc >= 16, 0x1F2);
        SetDiskErrorCallback(CDError);
        unk3f5 = OptionBool("print_glitches", false);
        unk3f6 = false;
        mD3DDevice = nullptr;
        unk22c = 0;
        NgRnd::PreInit();
        InitBuffers();
        TheShaderMgr.PreInit();
        TheRenderState.Init();
        Suspend();
        REGISTER_OBJ_FACTORY(DxTexRenderer)
        REGISTER_OBJ_FACTORY(DxCam)
        REGISTER_OBJ_FACTORY(DxEnviron);
        REGISTER_OBJ_FACTORY(DxMesh)
        DxMat::Init();
        REGISTER_OBJ_FACTORY(DxTex)
        REGISTER_OBJ_FACTORY(DxCubeTex);
        DxMultiMesh::Init();
        REGISTER_OBJ_FACTORY(DxMovie)
        DxParticleSys::Init();
        DxLight::Init();
        CreatePostTextures();
        DxTex::SetEDRamChecksEnabled(false);
        NgPostProc::Init();
        NgDOFProc::Init();
        DxTex::SetEDRamChecksEnabled(true);
        RndShadowMap::Init();
        Rnd::CreateDefaults();
        TheDebug.SetModalCallback(DxModal);
    }
}

void DxRnd::Init(HWND__ *h) {
    PreInit(h);
    mOcclusionQueryMgr = new DxRndOcclusionQueryMgr();
    NgRnd::Init();
    DxTex::Init();
    unk360 = false;
}

void DxRnd::Terminate() {
    Resume();
    DxLight::Terminate();
    DxMultiMesh::Shutdown();
    NgPostProc::Terminate();
    NgRnd::Terminate();
    RELEASE(mPreProcessTex);
    RELEASE(mPostProcessTex);
    RELEASE(mPreDepthTex);
    TerminateBuffers();
}

void DxRnd::SetSync(int sync) {
    Rnd::SetSync(sync);
    Resume();
    if (mSync == 0) {
        D3DDevice_SetRenderState_PresentInterval(TheDxRnd.Device(), 0x80000000);
    } else if (mSync == 1) {
        D3DDevice_SetRenderState_PresentInterval(TheDxRnd.Device(), 1);
    } else if (mSync == 2) {
        D3DDevice_SetRenderState_PresentInterval(TheDxRnd.Device(), 2);
    } else {
        MILO_FAIL("Not allowed to sync %d\n", mSync);
    }
}

void DxRnd::SetAspect(Aspect a) {
    if (mAspect != a) {
        Rnd::SetAspect(a);
        UpdateScalerParams();
        ResetDevice();
    }
}

void DxRnd::SetShrinkToSafeArea(bool shrink) {
    if (shrink != mShrinkToSafe) {
        Rnd::SetShrinkToSafeArea(shrink);
        UpdateScalerParams();
        ResetDevice();
    }
}

void DxRnd::DoWorldEnd() {
    if (mProcCmds & kProcessWorld) {
        Rnd::DoWorldEnd();
        {
            START_AUTO_TIMER("draw");
            DoPointTests();
        }
        SavePreBuffer();
    }
}

void DxRnd::DoPostProcess() {
    SetFrameBuffersAsSource();
    if (mProcCmds & kProcessPost) {
        if (mRegAlloc != 2) {
            mRegAlloc = (RegisterAlloc)2;
            D3DDevice_SetShaderGPRAllocation(mD3DDevice, 0, 0x10, 0x70);
        }
        NgRnd::DoPostProcess();
        FinishPostProcess();
    }
    D3DDevice_SetRenderTarget_External(mD3DDevice, 0, unk384);
    D3DDevice_SetDepthStencilSurface(mD3DDevice, unk38c);
    BeginTiling(Hmx::Color(0, 0, 0.3), 0, 0);
    CopyPostProcess();
    if (mRegAlloc != 1) {
        mRegAlloc = (RegisterAlloc)1;
        D3DDevice_SetShaderGPRAllocation(
            mD3DDevice, 0, mDefaultVSRegAlloc, mDefaultPSRegAlloc
        );
    }
    unk3a4 = true;
}

void DxRnd::Suspend() {
    if (mD3DDevice && !mAsyncSwapCurrent) {
        MILO_ASSERT(!mDrawing, 0x695);
        if (!unk3f4) {
            static Timer *cpuTimer = AutoTimer::GetTimer("cpu");
            if (unk3f5 && cpuTimer->SplitMs() > 30.0f) {
                MILO_LOG("GLITCH (pre-suspend): %i ms\n", (int)cpuTimer->SplitMs());
            }
            unk360 = false;
            D3DDevice_Suspend(mD3DDevice);
        }
        unk3f4 = true;
    }
}

void DxRnd::Resume() {
    if ((int)mD3DDevice) {
        if (unk3f4) {
            MILO_ASSERT(mAsyncSwapCurrent == false, 0x6AE);
            D3DDevice_Resume(mD3DDevice);
            unk360 = false;
        }
        unk3f4 = false;
    }
}

D3DSurface *DxRnd::BackBuffer() const {
    D3DResource_AddRef(mBackBuffer);
    return mBackBuffer;
}

D3DTexture *DxRnd::FrontBuffer() { return mFrontBuffers[unk35c - 1 & 1]; }
D3DTexture *DxRnd::NotFrontBuffer() { return mFrontBuffers[unk35c]; }

const char *DxRnd::Error(long code) { return MakeString("code %d", code); }

void DxRnd::Present() {
    unk35c = (unk35c - 1) & 1;
    if (mAsyncSwapCurrent) {
        static D3DSWAP_STATUS sSwapStatus;
        while (D3DDevice_QuerySwapStatus(mD3DDevice, &sSwapStatus),
               sSwapStatus.EnqueuedCount != 0) {
            Sleep(0);
        }

    } else {
        D3DDevice_SynchronizeToPresentationInterval(mD3DDevice);
    }
    D3DDevice_Swap(mD3DDevice, NotFrontBuffer(), nullptr);
    if (mAsyncSwapCurrent != unk360) {
        mAsyncSwapCurrent = unk360;
        D3DDevice_BlockUntilIdle(mD3DDevice);
        D3DDevice_SetSwapMode(mD3DDevice, mAsyncSwapCurrent);
    }
    unk3f7 = (PIXGetCaptureState() & 2);
}

void DxRnd::TerminateBuffers() {
    PreDeviceReset();
    if (mD3DDevice) {
        D3DDevice_Release(mD3DDevice);
        mD3DDevice = nullptr;
    }
}

void DxRnd::SetupGamma() {
    DataArray *cfg = SystemConfig("rnd");
    float gamma;
    if (cfg->FindData("gamma", gamma, false)) {
        D3DGAMMARAMP ramp;
        for (int i = 0; i < 256; i++) {
            float powed = (float)std::pow(i * 0.00390625f, gamma) * 1024.0f;
            ramp.red[i] = powed;
            ramp.green[i] = powed;
            ramp.blue[i] = powed;
        }
        D3DDevice_SetGammaRamp(mD3DDevice, 0, &ramp);
    }
}

void DxRnd::SetDefaultRenderStates() {
    D3DCAPS9 caps;
    memset(&caps, 0, sizeof(D3DCAPS9));
    GetDeviceCaps(&caps);
    D3DDevice_SetRenderState_AlphaRef(TheDxRnd.Device(), 0);
    D3DDevice_SetRenderState_AlphaFunc(TheDxRnd.Device(), D3DCMP_GREATER);
    D3DDevice_SetRenderState_PointSizeMax(
        TheDxRnd.Device(), reinterpret_cast<UINT &>(caps.MaxPointSize) // ???
    );
    D3DDevice_SetRenderState_SeparateAlphaBlendEnable(TheDxRnd.Device(), 1);
    D3DDevice_SetRenderState_SrcBlendAlpha(TheDxRnd.Device(), 1);
    D3DDevice_SetRenderState_DestBlendAlpha(TheDxRnd.Device(), 1);
    D3DDevice_SetRenderState_BlendOpAlpha(TheDxRnd.Device(), 3);
    for (int i = 0; i < caps.MaxTextureBlendStages; i++) {
        D3DDevice_SetSamplerState_MinFilter(TheDxRnd.Device(), i, 1);
        D3DDevice_SetSamplerState_MagFilter(TheDxRnd.Device(), i, 1);
    }
    D3DDevice_SetRenderState_PresentImmediateThreshold(TheDxRnd.Device(), 100);
}

void DxRnd::BeginTiling(const Hmx::Color &c, float f, unsigned int ui) {
    if (mNumTiles == 0) {
        D3DDevice_Clear(mD3DDevice, 0, nullptr, 0x31, MakeColor(c), f, ui, 0);
    } else {
        XMVECTOR v;
        v.v[0] = c.red;
        v.v[1] = c.green;
        v.v[2] = c.blue;
        v.v[3] = c.alpha;
        D3DDevice_BeginTiling(mD3DDevice, 0, mNumTiles, &unk3b4, &v, f, ui);
        unk34c = true;
    }
}

void DxRnd::PerfCountersInit() {
    if (!mCreatedPerfCounters) {
        mCreatedPerfCounters = true;
        mPerfCounterStart = D3DDevice_CreatePerfCounters(mD3DDevice, 1);
        DX_ASSERT(mPerfCounterStart, 0x230);
        mPerfCounterEnd = D3DDevice_CreatePerfCounters(mD3DDevice, 1);
        DX_ASSERT(mPerfCounterEnd, 0x231);
        D3DPERFCOUNTER_EVENTS perfEvents;
        memset(&perfEvents, 0, sizeof(D3DPERFCOUNTER_EVENTS));
        perfEvents.RBBM[0] = GPUPE_RBBM_NRT_BUSY;
        perfEvents.CP[0] = GPUPE_CP_COUNT;
        perfEvents.RBBM[1] = GPUPE_RBBM_COUNT;
        D3DDevice_EnablePerfCounters(mD3DDevice, true);
        D3DDevice_SetPerfCounterEvents(mD3DDevice, &perfEvents, 0);
        mGPUTimer = AutoTimer::GetTimer("gs");
    }
}

void DxRnd::PerfCountersStart() {
    MILO_ASSERT(mGsTiming == true, 0x249);
    MILO_ASSERT(mCreatedPerfCounters == true, 0x24A);
    MILO_ASSERT(mGPUTimer != NULL, 0x24B);
    MILO_ASSERT(mPerfCounterStart != NULL, 0x24C);
    MILO_ASSERT(mPerfCounterEnd != NULL, 0x24D);
    mGPUTimer->SetLastMs(unk370 * 1.075f);
    D3DDevice_QueryPerfCounters(mD3DDevice, mPerfCounterStart, 1);
}

void DxRnd::PerfCountersStop() {
    MILO_ASSERT(mGsTiming == true, 0x25D);
    MILO_ASSERT(mCreatedPerfCounters == true, 0x25E);
    MILO_ASSERT(mGPUTimer != NULL, 0x25F);
    MILO_ASSERT(mPerfCounterStart != NULL, 0x260);
    MILO_ASSERT(mPerfCounterEnd != NULL, 0x261);
    D3DDevice_QueryPerfCounters(mD3DDevice, mPerfCounterEnd, 1);
    D3DPERFCOUNTER_VALUES startValues;
    HRESULT code = D3DPerfCounters_GetValues(mPerfCounterStart, &startValues, 0, nullptr);
    DX_ASSERT_CODE(code, 0x269);
    D3DPERFCOUNTER_VALUES endValues;
    code = D3DPerfCounters_GetValues(mPerfCounterEnd, &endValues, 0, nullptr);
    DX_ASSERT_CODE(code, 0x26A);
    ULARGE_INTEGER *startLargeIntegers = (ULARGE_INTEGER *)&startValues;
    ULARGE_INTEGER *endLargeIntegers = (ULARGE_INTEGER *)&endValues;
    for (int i = 0; i < (sizeof(D3DPERFCOUNTER_VALUES) / sizeof(ULARGE_INTEGER)); i++) {
        endLargeIntegers[i].QuadPart =
            endLargeIntegers[i].QuadPart - startLargeIntegers[i].QuadPart;
    }
    unk370 = endLargeIntegers[1].QuadPart * 2e-06f;
    unk374 = endLargeIntegers[2].QuadPart * 2e-06f;
}

void DxRnd::EndTiling(D3DBaseTexture *tex, int i2) {
    int l2 = 0;
    if (tex && i2) {
        l2 = (i2 & 0x3F) << 0x1A;
    }
    if (unk34c) {
        MILO_ASSERT(mNumTiles > 0, 0x480);
        HRESULT hr =
            D3DDevice_EndTiling(mD3DDevice, l2, nullptr, tex, nullptr, 0, 0, nullptr);
        DX_ASSERT_CODE(hr, 0x481);
        unk34c = false;
    } else {
        MILO_ASSERT(mNumTiles == 0, 0x486);
        D3DDevice_Resolve(
            mD3DDevice, l2, nullptr, tex, nullptr, 0, 0, nullptr, 0, 0, nullptr
        );
    }
}

void DxRnd::SavePreBuffer() {
    XMVECTOR vector;
    vector.v[0] = mClearColor.red;
    vector.v[1] = mClearColor.green;
    vector.v[2] = mClearColor.blue;
    vector.v[3] = 0;
    D3DDevice_Resolve(
        mD3DDevice, 0x14, nullptr, mFrontBufferDepth, nullptr, 0, 0, nullptr, 1, 0, nullptr
    );
    D3DDevice_Resolve(
        mD3DDevice, 0x300, nullptr, mPreProcessBuffer, nullptr, 0, 0, &vector, 0, 0, nullptr
    );
}

void DxRnd::SavePostBuffer() {
    D3DDevice_Resolve(
        mD3DDevice, 0, nullptr, mPostProcessBuffer, nullptr, 0, 0, nullptr, 0, 0, nullptr
    );
}

void DxRnd::SetShaderRegisterAlloc(RegisterAlloc s) {
    MILO_ASSERT(s >=0 && s < kNumRegAlloc, 0x6BA);
    if (mRegAlloc != s) {
        mRegAlloc = s;
        switch (s) {
        case 0:
            D3DDevice_SetShaderGPRAllocation(mD3DDevice, 0, 0, 0);
            break;
        case 1:
            D3DDevice_SetShaderGPRAllocation(
                mD3DDevice, 0, mDefaultVSRegAlloc, mDefaultPSRegAlloc
            );
            break;
        case 2:
            D3DDevice_SetShaderGPRAllocation(mD3DDevice, 0, 0x10, 0x70);
            break;
        case 3:
            D3DDevice_SetShaderGPRAllocation(mD3DDevice, 0, 0x10, 0x70);
            break;
        default:
            MILO_NOTIFY("Invalid Shader Register Allocation");
            break;
        }
    }
}

RndTex *DxRnd::GetCurrentFrameTex(bool b1) {
    if (!unk3a4) {
        if (b1) {
            D3DDevice_Resolve(
                mD3DDevice,
                0,
                nullptr,
                mPreProcessBuffer,
                nullptr,
                0,
                0,
                nullptr,
                0,
                0,
                nullptr
            );
        }
        return PreProcessTexture();
    }
    return PostProcessTexture();
}

bool DxRnd::CanModal(Debug::ModalType t) {
    if (unk34c) {
        if (t == Debug::kModalFail) {
            EndTiling(FrontBuffer(), 0);
        } else {
            return false;
        }
    }
    return true;
}

void DxRnd::ModalDraw(Debug::ModalType t, const char *cc) {
    bool d3f4 = unk3f4;
    Resume();
    D3DSurface *renderTarget = D3DDevice_GetRenderTarget(mD3DDevice, 0);
    D3DSurface *stencilSurface = D3DDevice_GetDepthStencilSurface(mD3DDevice);
    D3DDevice_SetRenderTarget_External(mD3DDevice, 0, mBackBuffer);
    D3DDevice_SetDepthStencilSurface(mD3DDevice, 0);
    Hmx::Color color(0, 0.1, 0.5, 0);
    if (t == Debug::kModalFail) {
        color.alpha = 0.25f;
        color.green = 0;
        color.blue = 0;
    }
    D3DDevice_Clear(mD3DDevice, 0, nullptr, 0x31, MakeColor(color), 0, 0, 0);
    Rnd::DrawStringScreen(cc, Vector2(0.025f, 0.025f), Hmx::Color(1, 1, 1, 1), true);
    RndOverlay::DrawAll(true);
    D3DDevice_Resolve(
        mD3DDevice, 0, nullptr, FrontBuffer(), nullptr, 0, 0, nullptr, 0, 0, nullptr
    );
    if (mRegAlloc != 0) {
        mRegAlloc = (RegisterAlloc)0;
        D3DDevice_SetShaderGPRAllocation(mD3DDevice, 0, 0, 0);
    }
    Present();
    D3DDevice_SetRenderTarget_External(mD3DDevice, 0, renderTarget);
    D3DDevice_SetDepthStencilSurface(mD3DDevice, stencilSurface);
    if (d3f4) {
        Suspend();
    }
}

void DxRnd::InitBuffers() {
    PhysMemTypeTracker tracker("D3D(phys):DxRndBuffer");
    memset(&mPresentParams, 0, sizeof(D3DPRESENT_PARAMETERS));
    memset(&mVideoMode, 0, sizeof(XVIDEO_MODE));
    XGetVideoMode(&mVideoMode);
    static Symbol rnd("rnd");
    static Symbol low_res("low_res");
    static Symbol force_hd("force_hd");
    if (SystemConfig(rnd)->FindInt(force_hd) != 0) {
        mVideoMode.fIsHiDef = true;
        mVideoMode.fIsWideScreen = true;
    } else if (SystemConfig(rnd)->FindInt(low_res) != 0) {
        unk37c |= 1;
    }
    unk1f8 = unk37c;
    mAspect = unk1f8 ? kWidescreen : kRegular;
    mHeight = unk37c & 1 ? 540 : 720;
    int i11, i10;
    if (mVideoMode.fIsHiDef || unk1f8) {
        i11 = (mHeight << 4) / 9;
        i10 = (mHeight << 4) / 9;
    } else {
        i11 = (mHeight << 2) / 3;
        i10 = (mHeight << 2) / 3;
    }
    mWidth = i11;
    if (!(unk37c & 1)) {
        mNumTiles = 2;
        // stuff
        if (!(unk37c & 2)) {
            // things
        }
    }
    mPresentParams.Windowed = 0;
    mPresentParams.DisableAutoBackBuffer = 1;
    mPresentParams.DisableAutoFrontBuffer = 1;
    mPresentParams.BackBufferWidth = mWidth;
    mPresentParams.BackBufferHeight = mHeight;
    mPresentParams.PresentationInterval = 0;
    mPresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
    // D3DPRESENT_PARAMETERS mPresentParams; // 0x234
    //   local_88 = this + 0x294;
    //   *(this + 0x26c) = 1;DisableAutoBackBuffer
    //   *(this + 0x270) = 1;DisableAutoFrontBuffer
    //   *pDVar18 = *(this + 0x40);
    //   *(this + 0x238) = *(this + 0x44);
    //   *(this + 0x268) = 0;PresentationInterval
    //   *(this + 0x24c) = 1;SwapEffect
    //   *(this + 0x288) = 0x600000;
    //   *(this + 0x290) = 0xc;
    //   *(this + 0x294) = 0;
    //   *(this + 0x298) = 0;
    //   *(this + 0x29c) = *(this + 0x40);
    //   *(this + 0x2a0) = *(this + 0x44);
    //   *(this + 0x2ac) = 0;
    //   (**(*this + 0x154))(this);
    UpdateScalerParams();
    unk228 = GetCurrentThreadId();
    {
        BeginMemTrackObjectName("D3D->CreateDevice");
        HRESULT hr = Direct3D_CreateDevice(
            0, mDeviceType, &unk22c, 1, &mPresentParams, &mD3DDevice
        );
        DX_ASSERT_CODE(hr, 0x367);
        EndMemTrackObjectName();
    }
    if (!(unk37c & 1)) {
        MILO_ASSERT(mNumTiles > 0, 0x36D);
        BeginMemTrackObjectName("CreateBackBuffers:World");
        CreateBackBuffers(
            mWidth, mHeight, D3DMULTISAMPLE_NONE, unk3a8, unk3ac, mBackBuffer, unk388
        );
        EndMemTrackObjectName();
        BeginMemTrackObjectName("CreateBackBuffers:UI");
    } else {
        MILO_ASSERT(mNumTiles == 0, 0x37E);
        BeginMemTrackObjectName("CreateBackBuffers:World");
        CreateBackBuffers(
            mWidth, mHeight, D3DMULTISAMPLE_2_SAMPLES, unk3a8, unk3ac, mBackBuffer, unk388
        );
        EndMemTrackObjectName();
        BeginMemTrackObjectName("CreateBackBuffers:UI");
    }
    CreateBackBuffers(
        mWidth, mHeight, D3DMULTISAMPLE_2_SAMPLES, unk3a8, unk3ac, unk384, unk38c
    );
    EndMemTrackObjectName();
    {
        BeginMemTrackObjectName("CreateTexture:PreProcessBuffer");
        mPreProcessBuffer = static_cast<D3DTexture *>(D3DDevice_CreateTexture(
            mWidth, mHeight, 1, 1, 0, D3DFMT_A8R8G8B8, 0, D3DRTYPE_TEXTURE
        ));
        DX_ASSERT(mPreProcessBuffer, 0x390);
        EndMemTrackObjectName();
    }
    {
        BeginMemTrackObjectName("CreateTexture:PostProcessBuffer");
        mPostProcessBuffer = static_cast<D3DTexture *>(D3DDevice_CreateTexture(
            mWidth, mHeight, 1, 1, 0, D3DFMT_A8R8G8B8, 0, D3DRTYPE_TEXTURE
        ));
        DX_ASSERT(mPostProcessBuffer, 0x394);
        EndMemTrackObjectName();
    }
    for (int i = 0; i < 2; i++) {
        BeginMemTrackObjectName("CreateTexture:FrontBuffer");
        mFrontBuffers[i] = static_cast<D3DTexture *>(D3DDevice_CreateTexture(
            mWidth, mHeight, 1, 1, 0, D3DFMT_A8R8G8B8, 0, D3DRTYPE_TEXTURE
        ));
        DX_ASSERT(mFrontBuffers[i], 0x39C);
        EndMemTrackObjectName();
    }

    BeginMemTrackObjectName("CreateTexture:FrontBufferDepth");
    mFrontBufferDepth = static_cast<D3DTexture *>(D3DDevice_CreateTexture(
        mWidth, mHeight, 1, 1, 0, D3DFMT_D24FS8, 0, D3DRTYPE_TEXTURE
    ));
    DX_ASSERT(mFrontBufferDepth, 0x3A2);
    EndMemTrackObjectName();
    PostDeviceReset();
    for (int i = 0; i < 2; i++) {
    }
    mRegAlloc = (RegisterAlloc)0;
    D3DDevice_SetShaderGPRAllocation(mD3DDevice, 0, 0, 0);
    Present();
    SetSync(mSync);
}

void DxRnd::CreatePostTextures() {
    RELEASE(mPreProcessTex);
    mPreProcessTex = Hmx::Object::New<DxTex>();
    mPreProcessTex->SetDeviceTex(mPreProcessBuffer);
    RELEASE(mPreDepthTex);
    mPreDepthTex = Hmx::Object::New<DxTex>();
    mPreDepthTex->SetDeviceTex(mFrontBufferDepth);
    RELEASE(mPostProcessTex);
    mPostProcessTex = Hmx::Object::New<DxTex>();
    mPostProcessTex->SetDeviceTex(mPostProcessBuffer);
}

static DWORD sPointTestFence = -1;

void DxRnd::DoPointTests() {
    // Block on previous fence if set
    if (sPointTestFence != (DWORD)-1) {
        D3DDevice_BlockOnFence(sPointTestFence);
        sPointTestFence = -1;
    }

    // Early out if no occlusion query manager or hi-res screen is active
    if (!mOcclusionQueryMgr)
        return;
    if (TheHiResScreen.IsActive())
        return;

    // Process query results from previous frame
    for (std::vector<RndPointTest>::iterator it = unk20c.begin(); it != unk20c.end(); ++it) {
        unsigned int result;
        if (mOcclusionQueryMgr->GetQueryResults(it->unk4, result)) {
            it->unk0->SetOcclusionReady(true);
            it->unk0->SetVisible(result != 0);
        }
        if (mOcclusionQueryMgr->GetQueryResults(it->unk8, result)) {
            it->unk0->SetOcclusionResult((float)(int)result);
            it->unk0->SetOcclusionReady(true);
        }
    }

    // Update frame index - both direct manipulation and virtual call
    mOcclusionQueryMgr->ToggleFrameIndex();
    mOcclusionQueryMgr->OnBeginFrame();
    mOcclusionQueryMgr->IncrementFrameCounter();
    mOcclusionQueryMgr->OnEndFrame();

    // Count point tests needed
    int numTests = 0;
    for (std::list<PointTest>::iterator it = mPointTests.begin(); it != mPointTests.end(); ++it) {
        numTests++;
    }

    // Resize unk20c to match
    unk20c.resize(numTests);

    // Early out if no point tests
    if (mPointTests.empty())
        return;

    // Setup identity transform
    Transform xfm;
    xfm.Reset();
    TheShaderMgr.SetTransform(xfm);

    // Setup view matrix
    Hmx::Matrix4 viewMtx(xfm);
    TheShaderMgr.SetVConstant((VShaderConstant)4, viewMtx);

    // Setup shader state
    RndShader::SelectConfig(nullptr, kStandardShader, false);
    D3DDevice_SetPixelShader(mD3DDevice, nullptr);
    D3DDevice_SetFVF(mD3DDevice, 0x4042);

    // Disable color writes and blending for occlusion testing
    D3DDevice_SetRenderState_ColorWriteEnable(TheDxRnd.Device(), 0);
    D3DDevice_SetRenderState_AlphaBlendEnable(TheDxRnd.Device(), 0);
    D3DDevice_SetRenderState_AlphaTestEnable(TheDxRnd.Device(), 0);
    D3DDevice_SetRenderState_ZWriteEnable(TheDxRnd.Device(), 0);
    D3DDevice_SetRenderState_ZEnable(TheDxRnd.Device(), 1);

    // Set z-compare function based on unk_0x301
    D3DDevice_SetRenderState_ZFunc(TheDxRnd.Device(), (D3DCMPFUNC)(unk_0x301 ? 3 : 1));

    // Set point size
    float pointSize = 1.0f;
    D3DDevice_SetRenderState_PointSize(TheDxRnd.Device(), *(DWORD*)&pointSize);
    D3DDevice_SetRenderState_ViewportEnable(TheDxRnd.Device(), 0);
    D3DDevice_SetRenderState_HalfPixelOffset(TheDxRnd.Device(), 1);

    // Process each point test
    int idx = 0;
    for (std::list<PointTest>::iterator it = mPointTests.begin(); it != mPointTests.end(); ++it, ++idx) {
        TheNgStats->mFlares++;

        RndFlare *flare = it->unkc;
        RndPointTest &test = unk20c[idx];
        test.unk4 = -1;
        test.unk8 = -1;
        test.unk0 = flare;

        // Point test
        if (flare->GetPointTest()) {
            struct PointVertex {
                float x, y, z;
                float w;
                DWORD color;
            };
            PointVertex vtx;
            vtx.x = (float)it->unk4;
            vtx.y = (float)it->unk8;
            vtx.z = (float)it->unk0 * 5.9604651881e-08f;
            vtx.w = 1.0f;
            vtx.color = 0;

            unsigned int queryIdx;
            if (mOcclusionQueryMgr->CreateQuery(queryIdx)) {
                test.unk4 = queryIdx;
                mOcclusionQueryMgr->BeginQuery(test.unk4);
                D3DDevice_DrawVerticesUP(mD3DDevice, D3DPT_POINTLIST, 1, &vtx, sizeof(PointVertex));
                mOcclusionQueryMgr->EndQuery(test.unk4);
            }
        }

        // Area test
        if (flare->GetAreaTest()) {
            struct QuadVertex {
                float x, y, z;
                float w;
                DWORD color;
            };
            float z = (float)it->unk0 * 5.9604651881e-08f;
            QuadVertex verts[4];

            // Initialize vertices
            verts[0].x = flare->GetArea().x;
            verts[0].y = flare->GetArea().y;
            verts[0].z = z;
            verts[0].w = 1.0f;
            verts[0].color = 0;

            verts[1] = verts[0];
            verts[1].y += flare->GetArea().h;

            verts[2] = verts[0];
            verts[2].x += flare->GetArea().w;

            verts[3] = verts[1];
            verts[3].x += flare->GetArea().w;

            unsigned int queryIdx;
            if (mOcclusionQueryMgr->CreateQuery(queryIdx)) {
                test.unk8 = queryIdx;
                mOcclusionQueryMgr->BeginQuery(test.unk8);
                D3DDevice_DrawVerticesUP(mD3DDevice, D3DPT_TRIANGLESTRIP, 4, verts, sizeof(QuadVertex));
                mOcclusionQueryMgr->EndQuery(test.unk8);
            }
        } else {
            flare->SetOcclusionReady(true);
            flare->SetVisible(true);
        }
    }

    // Insert fence for next frame
    sPointTestFence = D3DDevice_InsertFence(mD3DDevice);

    // Clear current material
    NgMat::SetCurrent(nullptr);

    // Restore render states
    D3DDevice_SetRenderState_ColorWriteEnable(TheDxRnd.Device(), 0xF);
    D3DDevice_SetRenderState_ViewportEnable(TheDxRnd.Device(), 1);
    D3DDevice_SetRenderState_HalfPixelOffset(TheDxRnd.Device(), 0);

    // Restore camera if set
    if (RndCam::Current()) {
        TheShaderMgr.SetVConstant((VShaderConstant)4, RndCam::Current()->GetMatrix300());
    }
}
