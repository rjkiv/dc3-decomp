#include "Cam.h"
#include "Env.h"
#include "Lit.h"
#include "Mat.h"
#include "math/Color.h"
#include "math/Geo.h"
#include "math/Mtx.h"
#include "os/Memory.h"
#include "Mesh.h"
#include "Movie.h"
#include "MultiMesh.h"
#include "Part.h"
#include "os/OSFuncs.h"
#include "rndobj/Mat.h"
#include "rndobj/RenderState.h"
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
#include "rndobj/ShaderOptions.h"
#include "rndobj/ShadowMap.h"
#include "rndobj/Stats_NG.h"
#include "rndobj/Tex.h"
#include "utl/MemTrack.h"
#include "utl/Option.h"
#include "vectorintrinsics.h"
#include "xdk/D3D9.h"
#include "xdk/d3d9i/d3d9.h"
#include "xdk/d3d9i/d3d9caps.h"
#include "xdk/d3d9i/d3d9types.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/processthreadsapi.h"
#include "xdk/xapilibi/xbase.h"
#include "xdk/xapilibi/xbox.h"

void CreateBackBuffers(
    int,
    int,
    D3DMULTISAMPLE_TYPE,
    unsigned int &,
    unsigned int &,
    D3DSurface *&,
    D3DSurface *&
);

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

// DrawString

void DxRnd::BeginDrawing() {
    {
        static Timer *t = AutoTimer::GetTimer("cpu");
        if (t) {
            t->Stop();
        }
    }
    {
        static Timer *t = AutoTimer::GetTimer("cpu");
        if (unk3f4) {
            Resume();
        } else if (unk3f5) {
            if (t->Ms() > 66.0f) {
                MILO_LOG("GLITCH: %i ms\n", (int)t->Ms());
            }
        }
    }
    if (unk3f6) {
        PIXCaptureGpuFrame("capture.pix2");
        unk3f6 = false;
    }
    Present();
    if (MainThread()) {
        ReleaseAutoRelease();
    }
    Rnd::BeginDrawing();
    if (mGsTiming) {
        PerfCountersInit();
        PerfCountersStart();
    }
    DrawPreClear();
    Hmx::Color c = mClearColor;
    mD3DDevice->Clear(0, nullptr, 0x31, MakeColor(c), 0, 0);
    if (mRegAlloc != 1) {
        mRegAlloc = (RegisterAlloc)1;
        mD3DDevice->SetShaderGPRAllocation(0, mDefaultVSRegAlloc, mDefaultPSRegAlloc);
    }
    ResetStats();
    {
        static Timer *t = AutoTimer::GetTimer("cpu");
        if (t) {
            t->Start();
        }
    }
    {
        static Timer *t = AutoTimer::GetTimer("draw");
        if (t) {
            t->Start();
        }
    }
    NgMat::SetCurrent(nullptr);
}

void DxRnd::EndDrawing() {
    EndWorld();
    if (mShowSafeArea) {
        Hmx::Color red(1, 0, 0);
        Hmx::Color green(0, 1, 0);
        if (mAspect == 2) {
            DrawSafeArea(0.9f, true, red);
        }
        DrawSafeArea(0.9f, false, red);
        if (mAspect == 2) {
            DrawSafeArea(0.95f, true, green);
        }
        DrawSafeArea(0.95f, false, green);
    }
    Rnd::EndDrawing();
    unk3a4 = false;
    EndTiling(mFrontBuffers[unk35c - 1 & 1], 0);
    mD3DDevice->SetRenderTarget(0, mBackBuffer);
    mD3DDevice->SetDepthStencilSurface(unk388);
    if (mRegAlloc != 0) {
        mRegAlloc = (RegisterAlloc)0;
        mD3DDevice->SetShaderGPRAllocation(0, 0, 0);
    }
    static Timer *drawTimer = AutoTimer::GetTimer("draw");
    if (drawTimer) {
        drawTimer->Stop();
    }
    if (mGsTiming) {
        {
            static Timer *cpuTimer = AutoTimer::GetTimer("cpu");
            if (cpuTimer) {
                cpuTimer->Stop();
            }
        }
        PerfCountersStop();
        {
            static Timer *cpuTimer = AutoTimer::GetTimer("cpu");
            if (cpuTimer) {
                cpuTimer->Start();
            }
        }
    }
}

void DxRnd::SetSync(int sync) {
    Rnd::SetSync(sync);
    Resume();
    if (mSync == 0) {
        TheDxRnd.Device()->SetRenderState(D3DRS_PRESENTINTERVAL, 0x80000000);
    } else if (mSync == 1) {
        TheDxRnd.Device()->SetRenderState(D3DRS_PRESENTINTERVAL, 1);
    } else if (mSync == 2) {
        TheDxRnd.Device()->SetRenderState(D3DRS_PRESENTINTERVAL, 2);
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
            mD3DDevice->SetShaderGPRAllocation(0, 0x10, 0x70);
        }
        NgRnd::DoPostProcess();
        FinishPostProcess();
    }
    mD3DDevice->SetRenderTarget(0, unk384);
    mD3DDevice->SetDepthStencilSurface(unk38c);
    BeginTiling(Hmx::Color(0, 0, 0.3), 0, 0);
    CopyPostProcess();
    if (mRegAlloc != 1) {
        mRegAlloc = (RegisterAlloc)1;
        mD3DDevice->SetShaderGPRAllocation(0, mDefaultVSRegAlloc, mDefaultPSRegAlloc);
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
            mD3DDevice->Suspend();
        }
        unk3f4 = true;
    }
}

void DxRnd::Resume() {
    if (Device()) {
        if (unk3f4) {
            MILO_ASSERT(mAsyncSwapCurrent == false, 0x6AE);
            mD3DDevice->Resume();
            unk360 = false;
        }
        unk3f4 = false;
    }
}

void DxRnd::UpdateScalerParams() {
    float w = mVideoMode.dwDisplayWidth;
    float h = mVideoMode.dwDisplayHeight;
    bool b3 = mAspect == 3 && !unk1f8;
    if (b3 && w * 0.5625f < h) {
        h = w * 0.5625f;
    }
    if (mShrinkToSafe) {
        w *= 0.95f;
        if (!b3) {
            h *= 0.95f;
        }
    }
    D3DVIDEO_SCALER_PARAMETERS &vsp = mPresentParams.VideoScalerParameters;
    vsp.ScaledOutputWidth = w;
    vsp.ScaledOutputHeight = h;
}

D3DSurface *DxRnd::BackBuffer() const {
    mBackBuffer->AddRef();
    return mBackBuffer;
}

D3DTexture *DxRnd::FrontBuffer() { return mFrontBuffers[unk35c - 1 & 1]; }
D3DTexture *DxRnd::NotFrontBuffer() { return mFrontBuffers[unk35c]; }

const char *DxRnd::Error(HRESULT code) { return MakeString("code %d", code); }

void DxRnd::Present() {
    unk35c = (unk35c - 1) & 1;
    if (mAsyncSwapCurrent) {
        static D3DSWAP_STATUS sSwapStatus;
        while (mD3DDevice->QuerySwapStatus(&sSwapStatus),
               sSwapStatus.EnqueuedCount != 0) {
            Sleep(0);
        }

    } else {
        mD3DDevice->SynchronizeToPresentationInterval();
    }
    mD3DDevice->Swap(NotFrontBuffer(), nullptr);
    if (mAsyncSwapCurrent != unk360) {
        mAsyncSwapCurrent = unk360;
        mD3DDevice->BlockUntilIdle();
        mD3DDevice->SetSwapMode(mAsyncSwapCurrent);
    }
    unk3f7 = (PIXGetCaptureState() & 2) > 0;
}

void DxRnd::TerminateBuffers() {
    PreDeviceReset();
    if (mD3DDevice) {
        mD3DDevice->Release();
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
        mD3DDevice->SetGammaRamp(0, 0, &ramp);
    }
}

void DxRnd::SetDefaultRenderStates() {
    D3DCAPS9 caps;
    memset(&caps, 0, sizeof(D3DCAPS9));
    GetDeviceCaps(&caps);
    TheDxRnd.Device()->SetRenderState(D3DRS_ALPHAREF, 0);
    TheDxRnd.Device()->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
    TheDxRnd.Device()->SetRenderState(D3DRS_POINTSIZE_MAX, *(UINT *)(&caps.MaxPointSize));
    TheDxRnd.Device()->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, 1);
    TheDxRnd.Device()->SetRenderState(D3DRS_SRCBLENDALPHA, 1);
    TheDxRnd.Device()->SetRenderState(D3DRS_DESTBLENDALPHA, 1);
    TheDxRnd.Device()->SetRenderState(D3DRS_BLENDOPALPHA, 3);
    for (int i = 0; i < caps.MaxTextureBlendStages; i++) {
        TheDxRnd.Device()->SetSamplerState(i, D3DSAMP_MINFILTER, 1);
        TheDxRnd.Device()->SetSamplerState(i, D3DSAMP_MAGFILTER, 1);
        TheDxRnd.Device()->SetSamplerState(i, D3DSAMP_MIPFILTER, 1);
    }
    TheDxRnd.Device()->SetRenderState(D3DRS_PRESENTIMMEDIATETHRESHOLD, 100);
}

void DxRnd::BeginTiling(const Hmx::Color &c, float f, unsigned int ui) {
    if (mNumTiles == 0) {
        mD3DDevice->Clear(0, nullptr, 0x31, MakeColor(c), f, ui);
    } else {
        XMVECTOR v;
        v.v[0] = c.red;
        v.v[1] = c.green;
        v.v[2] = c.blue;
        v.v[3] = c.alpha;
        mD3DDevice->BeginTiling(0, mNumTiles, &unk3b4, &v, f, ui);
        unk34c = true;
    }
}

void DxRnd::PerfCountersInit() {
    if (!mCreatedPerfCounters) {
        mCreatedPerfCounters = true;
        HRESULT hr = mD3DDevice->CreatePerfCounters(&mPerfCounterStart, 1);
        DX_ASSERT(hr, 0x230);
        hr = mD3DDevice->CreatePerfCounters(&mPerfCounterEnd, 1);
        DX_ASSERT(hr, 0x231);
        D3DPERFCOUNTER_EVENTS perfEvents;
        memset(&perfEvents, 0, sizeof(D3DPERFCOUNTER_EVENTS));
        perfEvents.RBBM[0] = GPUPE_RBBM_NRT_BUSY;
        perfEvents.CP[0] = GPUPE_CP_COUNT;
        perfEvents.RBBM[1] = GPUPE_RBBM_COUNT;
        mD3DDevice->EnablePerfCounters(true);
        mD3DDevice->SetPerfCounterEvents(&perfEvents, 0);
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
    mD3DDevice->QueryPerfCounters(mPerfCounterStart, 1);
}

void DxRnd::PerfCountersStop() {
    MILO_ASSERT(mGsTiming == true, 0x25D);
    MILO_ASSERT(mCreatedPerfCounters == true, 0x25E);
    MILO_ASSERT(mGPUTimer != NULL, 0x25F);
    MILO_ASSERT(mPerfCounterStart != NULL, 0x260);
    MILO_ASSERT(mPerfCounterEnd != NULL, 0x261);
    mD3DDevice->QueryPerfCounters(mPerfCounterEnd, 1);
    D3DPERFCOUNTER_VALUES startValues;
    HRESULT hr = mPerfCounterStart->GetValues(&startValues, 0, nullptr);
    DX_ASSERT(hr, 0x269);
    D3DPERFCOUNTER_VALUES endValues;
    hr = mPerfCounterEnd->GetValues(&endValues, 0, nullptr);
    DX_ASSERT(hr, 0x26A);
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
        HRESULT hr = mD3DDevice->EndTiling(l2, nullptr, tex, nullptr, 0, 0, nullptr);
        DX_ASSERT(hr, 0x481);
        unk34c = false;
    } else {
        MILO_ASSERT(mNumTiles == 0, 0x486);
        mD3DDevice->Resolve(l2, nullptr, tex, nullptr, 0, 0, nullptr, 0, 0, nullptr);
    }
}

void DxRnd::SavePreBuffer() {
    Hmx::Color clearColor = mClearColor;
    mD3DDevice->Resolve(
        0x14, nullptr, mFrontBufferDepth, nullptr, 0, 0, nullptr, 1, 0, nullptr
    );
    mD3DDevice->Resolve(
        0x300,
        nullptr,
        mPreProcessBuffer,
        nullptr,
        0,
        0,
        (XMVECTOR *)&clearColor,
        0,
        0,
        nullptr
    );
}

void DxRnd::SavePostBuffer() {
    mD3DDevice->Resolve(
        0, nullptr, mPostProcessBuffer, nullptr, 0, 0, nullptr, 0, 0, nullptr
    );
}

void DxRnd::SetShaderRegisterAlloc(RegisterAlloc s) {
    MILO_ASSERT(s >=0 && s < kNumRegAlloc, 0x6BA);
    if (mRegAlloc != s) {
        mRegAlloc = s;
        switch (s) {
        case 0:
            mD3DDevice->SetShaderGPRAllocation(0, 0, 0);
            break;
        case 1:
            mD3DDevice->SetShaderGPRAllocation(0, mDefaultVSRegAlloc, mDefaultPSRegAlloc);
            break;
        case 2:
            mD3DDevice->SetShaderGPRAllocation(0, 0x10, 0x70);
            break;
        case 3:
            mD3DDevice->SetShaderGPRAllocation(0, 0x10, 0x70);
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
            mD3DDevice->Resolve(
                0, nullptr, mPreProcessBuffer, nullptr, 0, 0, nullptr, 0, 0, nullptr
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
    D3DSurface *renderTarget;
    mD3DDevice->GetRenderTarget(0, &renderTarget);
    D3DSurface *stencilSurface;
    mD3DDevice->GetDepthStencilSurface(&stencilSurface);
    mD3DDevice->SetRenderTarget(0, mBackBuffer);
    mD3DDevice->SetDepthStencilSurface(nullptr);
    Hmx::Color color(0, 0.1, 0.5, 0);
    if (t == Debug::kModalFail) {
        color.alpha = 0.25f;
        color.green = 0;
        color.blue = 0;
    }
    mD3DDevice->Clear(0, nullptr, 0x31, MakeColor(color), 0, 0);
    Rnd::DrawStringScreen(cc, Vector2(0.025f, 0.025f), Hmx::Color(1, 1, 1, 1), true);
    RndOverlay::DrawAll(true);
    mD3DDevice->Resolve(0, nullptr, FrontBuffer(), nullptr, 0, 0, nullptr, 0, 0, nullptr);
    if (mRegAlloc != 0) {
        mRegAlloc = (RegisterAlloc)0;
        mD3DDevice->SetShaderGPRAllocation(0, 0, 0);
    }
    Present();
    mD3DDevice->SetRenderTarget(0, renderTarget);
    mD3DDevice->SetDepthStencilSurface(stencilSurface);
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
        HRESULT hr = Direct3D::CreateDevice(
            0, mDeviceType, &unk22c, 1, &mPresentParams, &mD3DDevice
        );
        DX_ASSERT(hr, 0x367);
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
        HRESULT hr = TheDxRnd.Device()->CreateTexture(
            mWidth, mHeight, 1, 0, D3DFMT_A8R8G8B8, 0, &mPreProcessBuffer, nullptr
        );
        DX_ASSERT(hr, 0x390);
        EndMemTrackObjectName();
    }
    {
        BeginMemTrackObjectName("CreateTexture:PostProcessBuffer");
        HRESULT hr = TheDxRnd.Device()->CreateTexture(
            mWidth, mHeight, 1, 0, D3DFMT_A8R8G8B8, 0, &mPostProcessBuffer, nullptr
        );
        DX_ASSERT(hr, 0x394);
        EndMemTrackObjectName();
    }
    for (int i = 0; i < 2; i++) {
        BeginMemTrackObjectName("CreateTexture:FrontBuffer");
        HRESULT hr = TheDxRnd.Device()->CreateTexture(
            mWidth, mHeight, 1, 0, D3DFMT_LE_A8R8G8B8, 0, &mFrontBuffers[i], nullptr
        );
        DX_ASSERT(hr, 0x39C);
        EndMemTrackObjectName();
    }

    BeginMemTrackObjectName("CreateTexture:FrontBufferDepth");
    HRESULT hr = TheDxRnd.Device()->CreateTexture(
        mWidth, mHeight, 1, 0, D3DFMT_D24FS8, 0, &mFrontBufferDepth, nullptr
    );
    DX_ASSERT(hr, 0x3A2);
    EndMemTrackObjectName();
    PostDeviceReset();
    for (int i = 0; i < 2; i++) {
    }
    mRegAlloc = (RegisterAlloc)0;
    mD3DDevice->SetShaderGPRAllocation(0, 0, 0);
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

void DxRnd::SetFrameBuffersAsSource() {
    mD3DDevice->SetTexture(6, mPreProcessBuffer);
    TheDxRnd.Device()->SetSamplerState(6, D3DSAMP_MINFILTER, 1);
    TheDxRnd.Device()->SetSamplerState(6, D3DSAMP_MAGFILTER, 1);
    TheDxRnd.Device()->SetSamplerState(6, D3DSAMP_ADDRESSU, 2);
    TheDxRnd.Device()->SetSamplerState(6, D3DSAMP_ADDRESSV, 2);

    mD3DDevice->SetTexture(9, mFrontBufferDepth);
    TheDxRnd.Device()->SetSamplerState(9, D3DSAMP_MINFILTER, 0);
    TheDxRnd.Device()->SetSamplerState(9, D3DSAMP_MAGFILTER, 0);
    TheDxRnd.Device()->SetSamplerState(9, D3DSAMP_ADDRESSU, 2);
    TheDxRnd.Device()->SetSamplerState(9, D3DSAMP_ADDRESSV, 2);

    mD3DDevice->SetTexture(14, mPostProcessBuffer);
    TheDxRnd.Device()->SetSamplerState(14, D3DSAMP_MINFILTER, 1);
    TheDxRnd.Device()->SetSamplerState(14, D3DSAMP_MAGFILTER, 1);
    TheDxRnd.Device()->SetSamplerState(14, D3DSAMP_ADDRESSU, 2);
    TheDxRnd.Device()->SetSamplerState(14, D3DSAMP_ADDRESSV, 2);
}

void DxRnd::CopyPostProcess() {
    if (mRegAlloc != 2) {
        mRegAlloc = (RegisterAlloc)2;
        mD3DDevice->SetShaderGPRAllocation(0, 0x10, 0x70);
    }
    Hmx::Rect r(0, 0, mWidth, mHeight);
    RndMat *mat = TheShaderMgr.GetPostProcMat();
    mat->SetBlend(RndMat::kBlendSrc);
    mat->SetZMode(kZModeDisable);
    TheShaderMgr.SetUnk30(true);
    static bool sInitted = true;
    if (sInitted) {
        sInitted = true;
        TheDxRnd.Device()->SetSamplerState(0xE, D3DSAMP_MINFILTER, 1);
        TheDxRnd.Device()->SetSamplerState(0xE, D3DSAMP_MAGFILTER, 1);
    }
    DrawRect(r, mat, kPostprocessShader, Hmx::Color(), nullptr, nullptr);
    if (mRegAlloc != 1) {
        mRegAlloc = (RegisterAlloc)1;
        mD3DDevice->SetShaderGPRAllocation(0, mDefaultVSRegAlloc, mDefaultPSRegAlloc);
    }
}

static DWORD sPointTestFence = -1;

void DxRnd::DoPointTests() {
    if (sPointTestFence != -1) {
        mD3DDevice->BlockOnFence(sPointTestFence);
        sPointTestFence = -1;
    }
    if (mOcclusionQueryMgr && !TheHiResScreen.IsActive()) {
        FOREACH (it, unk20c) {
            unsigned int uiRef;
            if (mOcclusionQueryMgr->GetQueryResults(it->unk4, uiRef)) {
                it->unk0->SetVisible(uiRef);
            }
            if (mOcclusionQueryMgr->GetQueryResults(it->unk4, uiRef)) {
                RndFlare *flare = it->unk0;
                flare->SetVisibleArea(uiRef);
            }
        }
        mOcclusionQueryMgr->ToggleFrameIndex();
        mOcclusionQueryMgr->OnEndFrame();
        mOcclusionQueryMgr->IncrementFrameCounter();
        mOcclusionQueryMgr->OnBeginFrame();

        unk20c.resize(mPointTests.size());

        if (!mPointTests.empty()) {
            Transform xfm;
            xfm.Reset();
            TheShaderMgr.SetTransform(xfm);
            TheShaderMgr.SetVConstant((VShaderConstant)4, Hmx::Matrix4(xfm));
            RndShader::SelectConfig(nullptr, kStandardShader, false);
            mD3DDevice->SetPixelShader(nullptr);
            mD3DDevice->SetFVF(D3DFVF_XYZW | D3DFVF_DIFFUSE);
            TheDxRnd.Device()->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
            TheDxRnd.Device()->SetRenderState(D3DRS_ALPHABLENDENABLE, 0);
            TheDxRnd.Device()->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
            TheDxRnd.Device()->SetRenderState(D3DRS_ZWRITEENABLE, 0);
            TheDxRnd.Device()->SetRenderState(D3DRS_ZENABLE, 1);
            TheDxRnd.Device()->SetRenderState(
                D3DRS_ZFUNC, unk_0x301 ? D3DCMP_GREATER : D3DCMP_LESS
            );
            float ptSize = 1;
            TheDxRnd.Device()->SetRenderState(D3DRS_POINTSIZE, *(DWORD *)&ptSize);
            TheDxRnd.Device()->SetRenderState(D3DRS_VIEWPORTENABLE, 0);
            TheDxRnd.Device()->SetRenderState(D3DRS_HALFPIXELOFFSET, 1);
            FOREACH (it, mPointTests) {
                TheNgStats->mFlares++;

                unsigned int uiRef;
                if (mOcclusionQueryMgr->CreateQuery(uiRef)) {
                    mOcclusionQueryMgr->BeginQuery(uiRef);
                    mOcclusionQueryMgr->EndQuery(uiRef);
                }
            }
            sPointTestFence = mD3DDevice->InsertFence();
            NgMat::SetCurrent(nullptr);
            TheDxRnd.Device()->SetRenderState(D3DRS_COLORWRITEENABLE, 0xF);
            TheDxRnd.Device()->SetRenderState(D3DRS_VIEWPORTENABLE, 1);
            TheDxRnd.Device()->SetRenderState(D3DRS_HALFPIXELOFFSET, 0);
            if (RndCam::Current()) {
                TheShaderMgr.SetVConstant(
                    (VShaderConstant)4, RndCam::Current()->GetMatrix300()
                );
            }
        }
    }
}
