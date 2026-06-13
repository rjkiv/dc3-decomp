#include "rndobj/Rnd.h"
#include "math/Color.h"
#include "math/Geo.h"
#include "math/Vec.h"
#include "obj/DataFunc.h"
#include "obj/Dir.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "os/Joypad.h"
#include "os/Keyboard.h"
#include "os/OSFuncs.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "os/Timer.h"
#include "rnddx9/Tex.h"
#include "rndobj/AmbientOcclusion.h"
#include "rndobj/AnimFilter.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Cam.h"
#include "rndobj/CamAnim.h"
#include "rndobj/Console.h"
#include "rndobj/CubeTex.h"
#include "rndobj/DOFProc.h"
#include "rndobj/Dir.h"
#include "rndobj/Draw.h"
#include "rndobj/Enter.h"
#include "rndobj/Env.h"
#include "rndobj/EventTrigger.h"
#include "rndobj/Flare.h"
#include "rndobj/Font.h"
#include "rndobj/FontBase.h"
#include "rndobj/Fur.h"
#include "rndobj/Gen.h"
#include "rndobj/Graph.h"
#include "rndobj/Group.h"
#include "rndobj/HiResScreen.h"
#include "rndobj/Line.h"
#include "rndobj/Lit.h"
#include "rndobj/LitAnim.h"
#include "rndobj/Mat.h"
#include "rndobj/MatAnim.h"
#include "rndobj/Mesh.h"
#include "rndobj/MeshAnim.h"
#include "rndobj/MeshDeform.h"
#include "rndobj/MetaMaterial.h"
#include "rndobj/Morph.h"
#include "rndobj/MotionBlur.h"
#include "rndobj/Movie.h"
#include "rndobj/MultiMesh.h"
#include "rndobj/MultiMeshProxy.h"
#include "rndobj/Part.h"
#include "rndobj/PartAnim.h"
#include "rndobj/PartLauncher.h"
#include "rndobj/PollAnim.h"
#include "rndobj/PostProcMgr.h"
#include "rndobj/PropAnim.h"
#include "rndobj/Ribbon.h"
#include "rndobj/ScreenMask.h"
#include "rndobj/Set.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/ShaderOptions.h"
#include "rndobj/Shockwave.h"
#include "rndobj/SoftParticles.h"
#include "rndobj/Spline.h"
#include "rndobj/Tex.h"
#include "rndobj/TexBlendController.h"
#include "rndobj/TexBlender.h"
#include "rndobj/TexProc.h"
#include "rndobj/TexRenderer.h"
#include "rndobj/Text.h"
#include "rndobj/Trans.h"
#include "rndobj/TransAnim.h"
#include "rndobj/TransProxy.h"
#include "rndobj/Utl.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Overlay.h"
#include "rndobj/PostProc.h"
#include "rndobj/Wind.h"
#include "utl/Cheats.h"
#include "utl/FileStream.h"
#include "utl/MemMgr.h"
#include "utl/Option.h"
#include "utl/TextStream.h"
#include "xdk/XAPILIB.h"
#include "xdk/xapilibi/synchapi.h"
#include "xdk/xapilibi/xbox.h"

int Rnd::sPostProcPanelCount = 0;
static DxTex *sTexture = nullptr;
static bool sCompressDone = false;
static bool gNotifyKeepGoing = false;
static bool gFailKeepGoing = false;
static bool gFailRestartConsole = false;
static void *sCompressDesc = nullptr;
static HANDLE gRndThread = nullptr;
static HANDLE gRndTextureEvent = nullptr;
static int gCurHeap = -1;

DataNode ModalKeyListener::OnMsg(const KeyboardKeyMsg &k) {
    if (k.GetKey() == 0x12e) {
        if (!GetEnabledKeyCheats() && !TheRnd.ConsoleShowing()) {
            TheRnd.ShowConsole(true);
            return 0;
        } else
            return DATA_UNHANDLED;
    } else {
        if (!TheRnd.ConsoleShowing()) {
            gNotifyKeepGoing = true;
            return 0;
        } else
            return DATA_UNHANDLED;
    }
}

BEGIN_HANDLERS(ModalKeyListener)
    HANDLE_MESSAGE(KeyboardKeyMsg)
END_HANDLERS

DataNode FailKeepGoing(DataArray *) {
    gFailKeepGoing = true;
    return 0;
}

DataNode FailRestartConsole(DataArray *) {
    gFailRestartConsole = true;
    return 0;
}

Rnd::Rnd()
    : mClearColor(0.3f, 0.3f, 0.3f), mWidth(640), mHeight(480), mScreenBpp(16),
      mDrawCount(0), mDrawTimer(), mTimersOverlay(0), mRateOverlay(0), mHeapOverlay(0),
      mWatchOverlay(0), mStatsOverlay(0), mDefaultMat(0), mOverlayMat(0), mOverdrawMat(0),
      mDefaultCam(0), mWorldCamCopy(0), mDefaultEnv(0), mDefaultLit(0),
      mCubeTex_Black(nullptr), mCubeTex_White(nullptr), mRateTotal(0), mRateCount(5),
      mFrameID(0), mRateGate("    "), mFont(nullptr), mSync(1), mGsTiming(0),
      mShowSafeArea(0), mDrawing(0), mWorldEnded(1), mAspect(kWidescreen),
      mDrawMode(kDrawNormal), mShowShaderCost(0), mShowOverdraw(0), mShrinkToSafe(1),
      mInGame(0), mVerboseTimers(0), mDisablePostProc(0), unk146(0), unk147(0), unk148(0),
      mWorldEndCallback(0), mDrawPreClearCallback(0), mPostProcOverride(this),
      mPostProcBlackLightOverride(nullptr), mPreClearList(this),
      mSplashPreClearList(this), mSplashing(0), mProcCmds(kProcessAll),
      mLastProcCmds(kProcessAll) {
    for (int i = 0; i < kDefaultTex_Max; i++) {
        mDefaultTex[i] = nullptr;
    }
}

BEGIN_HANDLERS(Rnd)
    HANDLE_ACTION(reset_postproc, RndPostProc::Reset())
    HANDLE_ACTION(reset_dof_proc, RndPostProc::ResetDofProc())
    HANDLE_ACTION(set_postproc_override, SetPostProcOverride(_msg->Obj<RndPostProc>(2)))
    HANDLE_ACTION(
        set_postproc_blacklight_override,
        SetPostProcBlacklightOverride(_msg->Obj<RndPostProc>(2))
    )
    HANDLE_EXPR(get_postproc_override, GetPostProcOverride())
    HANDLE_EXPR(get_selected_postproc, GetSelectedPostProc())
    HANDLE_ACTION(
        set_dof_depth_scale, RndPostProc::DOFOverrides().SetDepthScale(_msg->Float(2))
    )
    HANDLE_ACTION(
        set_dof_depth_offset, RndPostProc::DOFOverrides().SetDepthOffset(_msg->Float(2))
    )
    HANDLE_ACTION(
        set_dof_min_scale, RndPostProc::DOFOverrides().SetMinBlurScale(_msg->Float(2))
    )
    HANDLE_ACTION(
        set_dof_min_offset, RndPostProc::DOFOverrides().SetMinBlurOffset(_msg->Float(2))
    )
    HANDLE_ACTION(
        set_dof_max_scale, RndPostProc::DOFOverrides().SetMaxBlurScale(_msg->Float(2))
    )
    HANDLE_ACTION(
        set_dof_max_offset, RndPostProc::DOFOverrides().SetMaxBlurOffset(_msg->Float(2))
    )
    HANDLE_ACTION(
        set_dof_width_scale, RndPostProc::DOFOverrides().SetBlurWidthScale(_msg->Float(2))
    )
    HANDLE_ACTION(set_aspect, SetAspect((Aspect)_msg->Int(2)))
    HANDLE_EXPR(aspect, mAspect)
    HANDLE_EXPR(screen_width, mWidth)
    HANDLE_EXPR(screen_height, mHeight)
    HANDLE_EXPR(highlight_style, RndDrawable::GetHighlightStyle())
    HANDLE_ACTION(
        set_highlight_style, RndDrawable::SetHighlightStyle((HighlightStyle)_msg->Int(2))
    )
    HANDLE_EXPR(get_normal_display_length, RndDrawable::GetNormalDisplayLength())
    HANDLE_ACTION(
        set_normal_display_length, RndDrawable::SetNormalDisplayLength(_msg->Float(2))
    )
    HANDLE_EXPR(get_force_select_proxied_subparts, RndDrawable::GetForceSubpartSelection())
    HANDLE_ACTION(
        set_force_select_proxied_subparts,
        RndDrawable::SetForceSubpartSelection(_msg->Int(2))
    )
    HANDLE_ACTION(set_sync, SetSync(_msg->Int(2)))
    HANDLE_EXPR(get_sync, GetSync())
    HANDLE_ACTION(set_shrink_to_safe, SetShrinkToSafeArea(_msg->Int(2)))
    HANDLE(show_console, OnShowConsole)
    HANDLE(toggle_timers, OnToggleTimers)
    HANDLE(toggle_overlay_position, OnToggleOverlayPosition)
    HANDLE(toggle_timers_verbose, OnToggleTimersVerbose)
    HANDLE(toggle_overlay, OnToggleOverlay)
    HANDLE_EXPR(show_safe_area, mShowSafeArea)
    HANDLE_ACTION(set_show_safe_area, mShowSafeArea = _msg->Int(2))
    HANDLE(show_overlay, OnShowOverlay)
    HANDLE_EXPR(overlay_showing, RndOverlay::Find(_msg->Str(2))->Showing())
    HANDLE(overlay_print, OnOverlayPrint)
    HANDLE_ACTION(hi_res_screen, TheHiResScreen.TakeShot("ur_hi", _msg->Int(2)))
    HANDLE_ACTION(proc_lock, SetProcAndLock(ProcAndLock() == 0))
    HANDLE_ACTION(allow_per_pixel, TheShaderMgr.SetAllowPerPixel(_msg->Int(2)))
    HANDLE_ACTION(reload_shaders, TheShaderMgr.Invalidate((ShaderType)_msg->Int(2)))
    HANDLE_ACTION(reload_shaders_all, TheShaderMgr.Invalidate(kMaxShaderTypes)) {
        static Symbol _s("toggle_error_shaders");
        if (sym == _s) {
            TheShaderMgr.SetShaderErrorDisplay(!TheShaderMgr.GetShaderErrorDisplay());
            return TheShaderMgr.GetShaderErrorDisplay();
        }
    }
    HANDLE_ACTION(set_in_game, SetInGame(_msg->Int(2)))
    HANDLE_ACTION(toggle_in_game, SetInGame(!mInGame))
    HANDLE(clear_color_r, OnClearColorR)
    HANDLE(clear_color_g, OnClearColorG)
    HANDLE(clear_color_b, OnClearColorB)
    HANDLE(clear_color_packed, OnClearColorPacked)
    HANDLE(set_clear_color, OnSetClearColor)
    HANDLE(set_clear_color_packed, OnSetClearColorPacked)
    HANDLE(screen_dump, OnScreenDump)
    HANDLE(screen_dump_unique, OnScreenDumpUnique)
    HANDLE(scale_object, OnScaleObject)
    HANDLE(reflect, OnReflect)
    HANDLE(toggle_heap, OnToggleHeap)
    HANDLE(toggle_watch, OnToggleWatch)
    HANDLE_ACTION(
        fix_vert_order, FixVertOrder(_msg->Obj<RndMesh>(2), _msg->Obj<RndMesh>(3))
    )
    HANDLE(test_draw_groups, OnTestDrawGroups)
    HANDLE_ACTION(
        test_texture_size,
        TestTextureSize(
            _msg->Obj<ObjectDir>(2),
            _msg->Int(3),
            _msg->Int(4),
            _msg->Int(5),
            _msg->Int(6),
            _msg->Int(7)
        )
    )
    HANDLE_ACTION(test_texture_paths, TestTexturePaths(_msg->Obj<ObjectDir>(2)))
    HANDLE_ACTION(test_material_textures, TestMaterialTextures(_msg->Obj<ObjectDir>(2)))
    HANDLE_ACTION(set_gfx_mode, SetGfxMode((GfxMode)_msg->Int(2)))
    HANDLE_EXPR(default_cam, mDefaultCam)
    HANDLE_EXPR(last_proc_cmds, mLastProcCmds)
    HANDLE_EXPR(toggle_all_postprocs, mDisablePostProc = !mDisablePostProc)
    HANDLE_ACTION(recreate_defaults, CreateDefaults())
    HANDLE_ACTION(
        reload_mat_materials, RndMat::ReloadAndUpdateMat(_msg->Obj<ObjectDir>(2))
    )
    HANDLE(toggle_show_metamat_errors, OnToggleShowMetaMatErrors)
    HANDLE(toggle_show_shader_errors, OnToggleShowShaderErrors)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void TerminateCallback() {
    RndUtlTerminate();
    TheRnd.Terminate();
}

void Rnd::PreInit() {
    SetName("rnd", ObjectDir::Main());
    TheDebug.AddExitCallback(TerminateCallback);
    DataArray *rndcfg = SystemConfig("rnd");
    rndcfg->FindData("bpp", mScreenBpp);
    rndcfg->FindData("height", mHeight);
    rndcfg->FindData("clear_color", mClearColor);
    rndcfg->FindData("sync", mSync);
    rndcfg->FindData("aspect", (int &)mAspect);
    if (OptionBool("widescreen", false)) {
        mAspect = kWidescreen;
    }
    mWidth = ((float)mHeight / Rnd::YRatio()) + 0.5f;
    MILO_ASSERT((mScreenBpp == 16) || (mScreenBpp == 32), 0x209);
    SetupFont();
    RndGraph::Init();
    RndUtlPreInit();
    RndDrawable::Init();
    RndFur::Init();
    RndTransformable::Init();
    RndSet::Init();
    RndAnimFilter::Init();
    RndFlare::Init();
    RndCam::Init();
    RndMesh::Init();
    RndMeshDeform::Init();
    RndText::Init();
    RndFontBase::Init();
    RndFont::Init();
    RndFont3d::Init();
    RndEnviron::Init();
    RndTex::Init();
    RndCubeTex::Init();
    RndMovie::Init();
    RndLight::Init();
    RndTransAnim::Init();
    RndLightAnim::Init();
    RndMeshAnim::Init();
    RndMatAnim::Init();
    RndTransProxy::Init();
    RndPartLauncher::Init();
    RndLine::Init();
    RndGenerator::Init();
    RndParticleSys::Init();
    RndParticleSysAnim::Init();
    RndRibbon::Init();
    RndMultiMesh::Init();
    RndMultiMeshProxy::Init();
    RndMorph::Init();
    RndCamAnim::Init();
    REGISTER_OBJ_FACTORY(RndTransformable)
    RndGroup::Init();
    RndDir::Init();
    RndMotionBlur::Init();
    RndTexBlendController::Init();
    RndTexBlender::Init();
    RndTexRenderer::Init();
    RndScreenMask::Init();
    RndSoftParticles::Init();
    REGISTER_OBJ_FACTORY(RndPostProc)
    RndPostProcMgr::Init();
    RndAmbientOcclusion::Init();
    RndOverlay::Init();
    RndPropAnim::Init();
    EventTrigger::Init();
    RndWind::Init();
    RndPollAnim::Init();
    BaseMaterial::Init();
    REGISTER_OBJ_FACTORY(MetaMaterial)
    RndEnterable::Init();
    RndMat::Init();
    RndSpline::Init();
    RndShockwave::Init();
    DOFProc::Init();
    TexProc::Init();
    // this is likely some other rndobj without a NewObject overload
    REGISTER_OBJ_FACTORY(Hmx::Object)
    InitShaderOptions();
    mRateOverlay = RndOverlay::Find("rate");
    mHeapOverlay = RndOverlay::Find("heap");
    // well ok then
    mWatcher.SetOverlay(mWatchOverlay = RndOverlay::Find("watch"));
    mWatcher.Init();
    mStatsOverlay = RndOverlay::Find("stats");
    mTimersOverlay = RndOverlay::Find("timers");
    mRateOverlay->SetCallback(this);
    mHeapOverlay->SetCallback(this);
    mWatchOverlay->SetCallback(this);
    mStatsOverlay->SetCallback(this);
    mTimersOverlay->SetCallback(this);
    mConsole = new RndConsole();
    mWorldEnded = true;
    mDrawing = false;
    mGsTiming = mTimersOverlay->Showing();
    CreateDefaults();
    InitParticleSystem();
    DataRegisterFunc("keep_going", FailKeepGoing);
    DataRegisterFunc("restart_console", FailRestartConsole);
}

DWORD CompressThread(HANDLE h) {
    while (true) {
        WaitForSingleObject(gRndTextureEvent, -1);
        if (!sTexture)
            break;
        sTexture->DoCompress(sCompressDesc);
        sCompressDone = true;
    }
    return 0;
}

void Rnd::Init() {
    DataArray *cfg = SystemConfig("rnd");
    DataArray *stats = cfg->FindArray("timer_stats", false);
    if (stats) {
        if (stats->Int(1)) {
            MILO_LOG("config showing timers\n");
            SetShowTimers(true, true);
        }
    }
    RndUtlInit();
    RndPostProc::Init();
    gRndTextureEvent = CreateEventA(nullptr, false, false, "texture_event");
    gRndThread = CreateThread(nullptr, 0, CompressThread, nullptr, 4, nullptr);
    XSetThreadProcessor(gRndThread, 1);
    ResumeThread(gRndThread);
}

void Rnd::Terminate() {
    RELEASE(mConsole);
    TheDebug.RemoveExitCallback(TerminateCallback);
    RndOverlay::Terminate();
    RndMultiMesh::Terminate();
    DOFProc::Terminate();
    RndMat::Terminate();
    SetName(nullptr, nullptr);
    SetEvent(gRndTextureEvent);
    CloseHandle(gRndThread);
    CloseHandle(gRndTextureEvent);
}

void Rnd::ScreenDump(const char *file) {
    RndTex *tex = Hmx::Object::New<RndTex>();
    RndBitmap bmap;
    tex->SetBitmap(0, 0, 0, RndTex::kFrontBuffer, false, nullptr);
    tex->LockBitmap(bmap, 1);
    FileStream stream(file, FileStream::kWrite, true);
    if (stream.Fail()) {
        MILO_NOTIFY("Screenshot failed; could not open destination file (%s).", file);
    } else {
        bmap.SaveBmp(&stream);
    }
    delete tex;
}

void Rnd::ScreenDumpUnique(const char *cc) {
    String filename = UniqueFilename(cc, "bmp");
    ScreenDump(filename.c_str());
}

Vector2 &Rnd::DrawString(const char *, const Vector2 &v, const Hmx::Color &, bool) {
    static Vector2 s;
    s = v;
    return s;
}

void Rnd::BeginDrawing() {
    mDrawing = true;
    mWorldEnded = false;
    mDrawTimer.Restart();
    AutoTimer::ResetTimers();
    mLastProcCmds = mProcCmds;
    mProcCmds = mProcCounter.ProcCommands();
    mDefaultCam->Select();
    mDefaultEnv->Select(nullptr);
    if (!TheHiResScreen.IsActive()) {
        mPointTests.clear();
    }
    mDrawCount++;
    if (mPostProcBlackLightOverride) {
        mPostProcBlackLightOverride->SetBloomColor();
    } else if (mPostProcOverride) {
        mPostProcOverride->SetBloomColor();
    } else if (RndPostProc::Current()) {
        RndPostProc::Current()->SetBloomColor();
    }
}

void Rnd::EndDrawing() {
    EndWorld();
    if (MainThread()) {
        {
            static Timer *cpuStop = AutoTimer::GetTimer("cpu");
            if (cpuStop)
                cpuStop->Stop();
        }
        {
            static Timer *drawStop = AutoTimer::GetTimer("draw");
            if (drawStop)
                drawStop->Stop();
        }
        static Timer *t = AutoTimer::GetTimer("overlays");
        AutoTimer at(t, 50.0f, NULL, NULL);
        AutoSlowFrame asf("RndOverlay::DrawAll", 10.0f);
        if (RndCam::Current()->TargetTex()) {
            mDefaultCam->Select();
        }
        RndOverlay::DrawAll(false);
        RndGraph::DrawAll();
        {
            static Timer *cpuStart = AutoTimer::GetTimer("cpu");
            if (cpuStart)
                cpuStart->Start();
        }
        {
            static Timer *drawStart = AutoTimer::GetTimer("draw");
            if (drawStart)
                drawStart->Start();
        }
    }
    mDrawing = false;
    mFrameID++;
}

void Rnd::RemovePointTest(RndFlare *flare) {
    if (!TheHiResScreen.IsActive()) {
        for (std::list<PointTest>::iterator it = mPointTests.begin();
             it != mPointTests.end();) {
            if (it->unkc == flare) {
                it = mPointTests.erase(it);
            } else
                ++it;
        }
    }
}

float Rnd::YRatio() {
    static const float kRatio[5] = { 1.0f, 0.75f, 0.5625f, 0.5625f, 0.6f };
    return kRatio[mAspect];
}

struct SortPostProc {
    bool operator()(PostProcessor *p1, PostProcessor *p2) const {
        return p1->Priority() < p2->Priority();
    }
};

void Rnd::ShowConsole(bool show) { mConsole->SetShowing(show); }
bool Rnd::ConsoleShowing() { return mConsole->Showing(); }

void Rnd::EndWorld() {
    if (!mWorldEnded) {
        if (mWorldEndCallback) {
            mWorldEndCallback();
        }
        DoWorldEnd();
        DoPostProcess();
        mWorldEnded = true;
    }
}

void Rnd::SetShowTimers(bool show, bool verbose) {
    mTimersOverlay->SetShowing(show);
    mVerboseTimers = verbose;
    SetGSTiming(show);
}

void Rnd::SetProcAndLock(bool b) { mProcCounter.SetProcAndLock(b); }

bool Rnd::ProcAndLock() const { return mProcCounter.ProcAndLock(); }

void Rnd::ResetProcCounter() {
    if (mDrawing) {
        mProcCounter.SetCount(-1);
    } else
        mLastProcCmds = ProcessCmd(mLastProcCmds | kProcessWorld);
}

bool Rnd::GetEvenOddDisabled() const { return mProcCounter.EvenOddDisabled(); }
void Rnd::SetEvenOddDisabled(bool b) { mProcCounter.SetEvenOddDisabled(b); }

void Rnd::DrawRectScreen(
    const Hmx::Rect &r,
    const Hmx::Color &c1,
    RndMat *mat,
    const Hmx::Color *cptr1,
    const Hmx::Color *cptr2
) {
    Hmx::Rect rect(r.x * mWidth, r.y * mHeight, r.w * mWidth, r.h * mHeight);
    DrawRect(rect, c1, mat, cptr1, cptr2);
}

const Vector2 &
Rnd::DrawStringScreen(const char *c, const Vector2 &v, const Hmx::Color &color, bool b4) {
    float fwidth = mWidth;
    float fheight = mHeight;
    Vector2 &vres = DrawString(c, Vector2(v.x * fwidth, v.y * fheight), color, b4);
    vres.x /= fwidth;
    vres.y /= fheight;
    return vres;
}

RndPostProc *Rnd::GetPostProcOverride() { return mPostProcOverride; }

RndPostProc *Rnd::GetSelectedPostProc() {
    RndPostProc *selected = nullptr;
    FOREACH (it, mPostProcessors) {
        RndPostProc *set = dynamic_cast<RndPostProc *>(*it);
        if (selected) {
            MILO_NOTIFY("More than one postproc selected: %s", PathName(set));
        } else
            selected = set;
    }
    return selected;
}

void Rnd::DoWorldBegin() {
    if (mPostProcBlackLightOverride) {
        mPostProcBlackLightOverride->BeginWorld();
    } else if (mPostProcOverride) {
        mPostProcOverride->BeginWorld();
    } else {
        FOREACH (it, mPostProcessors) {
            (*it)->BeginWorld();
        }
    }
}

void Rnd::DoWorldEnd() {
    if (!unk147) {
        CopyWorldCam(nullptr);
    }
    unk147 = false;
    if (mPostProcBlackLightOverride) {
        mPostProcBlackLightOverride->EndWorld();
    } else if (mPostProcOverride) {
        mPostProcOverride->EndWorld();
    } else {
        auto it = mPostProcessors.begin();
        auto itEnd = mPostProcessors.end();
        for (; it != itEnd; ++it) {
            (*it)->EndWorld();
        }
    }
}

void Rnd::DoPostProcess() {
    if (!mDisablePostProc) {
        if (mPostProcBlackLightOverride) {
            mPostProcBlackLightOverride->DoPost();
        } else if (mPostProcOverride) {
            mPostProcOverride->DoPost();
        } else {
            FOREACH (it, mPostProcessors) {
                (*it)->DoPost();
            }
        }
    }
}

void Rnd::DrawPreClear() {
    if (mDrawPreClearCallback) {
        mDrawPreClearCallback();
    }
    if (sCompressDone) {
        sTexture->FinishCompress(sCompressDesc);
        sCompressDesc = nullptr;
        MILO_ASSERT(sTexture, 0x481);
        CompressTexDesc *cur = mCompressTexDescs.front();
        if (cur->tex) {
            RndTex *cTex = cur->tex;
            ReplaceObject(cTex, sTexture, false, false, false);
            sTexture = static_cast<DxTex *>(cTex);
        }
        mCompressTexDescs.pop_front();
        delete cur;
        RELEASE(sTexture);
        sCompressDone = false;
    }
    if (!sTexture && !mCompressTexDescs.empty()) {
        for (auto it = mCompressTexDescs.begin(); it != mCompressTexDescs.end();) {
            CompressTexDesc *cur = *it;
            if (cur->tex && cur->callback) {
                ++it;
            } else {
                it = mCompressTexDescs.erase(it);
                delete cur;
            }
        }
        if (mCompressTexDescs.size() != 0) {
            CompressTexDesc *desc = mCompressTexDescs.front();
            sTexture = static_cast<DxTex *>(desc->tex.Ptr());
            RndTex *rTex;
            {
                MemDoTempAllocations t;
                rTex = Hmx::Object::New<RndTex>();
            }
            ReplaceObject(sTexture, rTex, false, false, false);
            sCompressDesc = sTexture->StartCompress(desc->alpha);
            MILO_ASSERT(!sCompressDone, 0x4C3);
            SetEvent(gRndTextureEvent);
        }
    }
    auto &drawList = mSplashing ? mPreClearList : mSplashPreClearList;
    if (!drawList.empty()) {
        unk148 = true;
        RndCam *cam = RndCam::Current();
        FOREACH (it, drawList) {
            RndDrawable *cur = *it;
            if (cur) {
                cur->DrawPreClear();
            }
        }
        if (cam && cam != RndCam::Current()) {
            cam->Select();
        }
        unk148 = false;
    }
}

float Rnd::UpdateOverlay(RndOverlay *o, float f) {
    if (o == mRateOverlay) {
        UpdateRate();
    } else if (o == mHeapOverlay) {
        UpdateHeap();
    } else if (o == mWatchOverlay) {
        mWatcher.Update();
    } else if (o == mTimersOverlay) {
        f = DrawTimers(f);
    }
    return f;
}

DataNode Rnd::OnShowOverlay(const DataArray *da) {
    RndOverlay *o = RndOverlay::Find(da->Str(2), false);
    if (o) {
        o->SetShowing(da->Int(3));
        if (da->Size() > 4) {
            o->SetTimeout(da->Float(4));
        }
    }
    return 0;
}

DataNode Rnd::OnOverlayPrint(const DataArray *da) {
    RndOverlay *o = RndOverlay::Find(da->Str(2));
    String str;
    for (int i = 3; i < da->Size(); i++) {
        da->Evaluate(i).Print(str, true, 0);
    }
    o->Print(str.c_str());
    return 0;
}

DataNode Rnd::OnReflect(const DataArray *da) {
    RndOverlay *o = RndOverlay::Find(da->Sym(2));
    if (o->Showing()) {
        TextStream *idk = TheDebug.SetReflect(o);
        for (int i = 3; i < da->Size(); i++) {
            da->Command(i)->Execute(true);
        }
        TheDebug.SetReflect(idk);
    }
    return 0;
}

DataNode Rnd::OnToggleOverlay(const DataArray *da) {
    RndOverlay *o = RndOverlay::Find(da->Str(2));
    o->SetShowing(!o->Showing());
    if (o->Showing()) {
        o->SetDumpCount(1);
    }
    return o->Showing();
}

DataNode Rnd::OnToggleOverlayPosition(const DataArray *) {
    RndOverlay::TogglePosition();
    return 0;
}

DataNode Rnd::OnShowConsole(const DataArray *) {
    ShowConsole(true);
    return 0;
}

DataNode Rnd::OnToggleTimers(const DataArray *) {
    SetShowTimers(mVerboseTimers || !TimersShowing(), false);
    return 0;
}

DataNode Rnd::OnToggleTimersVerbose(const DataArray *) {
    SetShowTimers(mVerboseTimers == 0, mVerboseTimers == 0);
    return 0;
}

DataNode Rnd::OnClearColorR(const DataArray *) { return mClearColor.red; }
DataNode Rnd::OnClearColorG(const DataArray *) { return mClearColor.green; }
DataNode Rnd::OnClearColorB(const DataArray *) { return mClearColor.blue; }
DataNode Rnd::OnClearColorPacked(const DataArray *) { return mClearColor.Pack(); }

DataNode Rnd::OnSetClearColor(const DataArray *da) {
    SetClearColor(Hmx::Color(da->Float(2), da->Float(3), da->Float(4)));
    return 0;
}

DataNode Rnd::OnSetClearColorPacked(const DataArray *da) {
    SetClearColor(
        Hmx::Color(
            (da->Int(2) & 255) / 255.0f,
            ((da->Int(2) >> 8) & 255) / 255.0f,
            ((da->Int(2) >> 0x10) & 255) / 255.0f
        )
    );
    return 0;
}

DataNode Rnd::OnScreenDump(const DataArray *da) {
    ScreenDump(da->Str(2));
    return 0;
}

DataNode Rnd::OnScreenDumpUnique(const DataArray *da) {
    ScreenDumpUnique(da->Str(2));
    return 0;
}

DataNode Rnd::OnScaleObject(const DataArray *da) {
    RndScaleObject(da->GetObj(2), da->Float(3), da->Float(4));
    return 0;
}

DataNode Rnd::OnToggleHeap(const DataArray *) {
    int num = MemNumHeaps() + 1;
    if (!mHeapOverlay->Showing()) {
        mHeapOverlay->SetShowing(true);
    } else {
        gCurHeap++;
        if (gCurHeap >= num) {
            gCurHeap = -1;
            mHeapOverlay->SetShowing(false);
        } else {
            mHeapOverlay->SetShowing(true);
        }
    }
    return 0;
}

void Rnd::UnregisterPostProcessor(PostProcessor *proc) { mPostProcessors.remove(proc); }

void PreClearCompilerHelper(ObjPtrList<RndDrawable> &list, RndDrawable *draw) {
    FOREACH (it, list) {
        if (*it == draw)
            return;
    }
    list.push_back(draw);
    list.sort(SortDraws);
}

void Rnd::RegisterPostProcessor(PostProcessor *proc) {
    sPostProcPanelCount++;
    mPostProcessors.push_back(proc);
    mPostProcessors.sort(SortPostProc());
}

void Rnd::CopyWorldCam(RndCam *cam) {
    if (mProcCmds & kProcessWorld) {
        if (!cam) {
            cam = RndCam::Current();
        }
        mWorldCamCopy->Copy(cam, kCopyShallow);
        mWorldCamCopy->SetTransParent(nullptr, false);
        unk147 = true;
    }
}

RndTex *Rnd::GetNullTexture() { return mDefaultTex[kDefaultTex_Error]; }

void Rnd::SetupFont() {
    mFont = SystemConfig("rnd", "font");
    for (int i = 0; i < 26; i++) {
        DataArray *arr = mFont->Array(i + 66)->Clone(true, false, 0);
        for (int j = 0; j < arr->Size(); j++) {
            DataArray *jArr = arr->Array(j);
            for (int k = 1; k < jArr->Size(); k += 2) {
                jArr->Node(k) = jArr->Float(k) * 0.7f + 0.3f;
            }
        }
        mFont->Node(i + 98) = arr;
        arr->Release();
    }
}

void Rnd::CreateCubeTextures() {
    mCubeTex_Black = Hmx::Object::New<RndCubeTex>();
    mCubeTex_White = Hmx::Object::New<RndCubeTex>();
    for (unsigned int i = 0; i < RndCubeTex::kNumCubeFaces; i++) {
        RndCubeTex::CubeFace cf = (RndCubeTex::CubeFace)i;
        RndBitmap &bm110 = mCubeTex_Black->GetBitmap(cf);
        RndBitmap &bm114 = mCubeTex_White->GetBitmap(cf);
        bm110.Create(32, 32, 0, 32, 0, nullptr, nullptr, nullptr);
        bm114.Create(32, 32, 0, 32, 0, nullptr, nullptr, nullptr);
        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 32; k++) {
                bm110.SetPixelColor(k, j, 0, 0, 0, 0);
                bm114.SetPixelColor(k, j, 255, 255, 255, 255);
            }
        }
        mCubeTex_Black->UpdateFace(cf);
        mCubeTex_White->UpdateFace(cf);
    }
}

DataNode Rnd::OnToggleWatch(const DataArray *) {
    mWatchOverlay->SetShowing(!mWatchOverlay->Showing());
    return 0;
}

DataNode Rnd::OnToggleShowMetaMatErrors(const DataArray *) {
    TheShaderMgr.ToggleShowMetaMatErrors();
    return 0;
}

DataNode Rnd::OnToggleShowShaderErrors(const DataArray *) {
    TheShaderMgr.ToggleShowShaderErrors();
    return 0;
}

void Rnd::SetPostProcOverride(RndPostProc *pp) {
    MILO_LOG(
        "Rnd::SetPostProcOverride: %s -> %s\n",
        mPostProcOverride == 0 ? "NULL" : PathName(mPostProcOverride),
        pp == 0 ? "NULL" : PathName(pp)
    );
    mPostProcOverride = pp;
    RndOverlay *ppOverlay = RndOverlay::Find("postproc");
    if (ppOverlay->Showing()) {
        TextStream *old = TheDebug.Reflect();
        TheDebug.SetReflect(ppOverlay);
        MILO_LOG("SETPROSTPROCOVERRIDE: %s\n", pp == 0 ? "NULL" : PathName(pp));
        TheDebug.SetReflect(old);
    }
}

void Rnd::SetPostProcBlacklightOverride(RndPostProc *pp) {
    mPostProcBlackLightOverride = pp;
    RndOverlay *ppOverlay = RndOverlay::Find("postproc");
    if (ppOverlay->Showing()) {
        TextStream *old = TheDebug.Reflect();
        TheDebug.SetReflect(ppOverlay);
        MILO_LOG("SETBLACKLIGHTOVERRIDE: %s\n", pp == 0 ? "NULL" : PathName(pp));
        TheDebug.SetReflect(old);
    }
}

void Rnd::CreateDefaults() {
    RELEASE(mWorldCamCopy);
    RELEASE(mDefaultCam);
    RELEASE(mDefaultEnv);
    RELEASE(mDefaultLit);
    RELEASE(mDefaultMat);
    RELEASE(mOverlayMat);
    RELEASE(mOverdrawMat);
    mWorldCamCopy = ObjectDir::Main()->New<RndCam>("[world cam copy]");
    mDefaultCam = ObjectDir::Main()->New<RndCam>("[default cam]");
    mDefaultEnv = ObjectDir::Main()->New<RndEnviron>("[default env]");
    mDefaultLit = ObjectDir::Main()->New<RndLight>("[default lit]");
    mDefaultLit->SetTransParent(mDefaultCam, false);
    mDefaultLit->SetLightType(RndLight::kDirectional);
    mDefaultEnv->AddLight(mDefaultLit);
    mDefaultEnv->SetUseApproxes(true);
    mDefaultEnv->SetUseApproxGlobal(false);
    mDefaultMat = Hmx::Object::New<RndMat>();
    mDefaultMat->SetUseEnv(false);
    mDefaultMat->SetPreLit(true);
    CreateAndSetMetaMat(mDefaultMat);
    mOverlayMat = Hmx::Object::New<RndMat>();
    mOverlayMat->SetUseEnv(false);
    mOverlayMat->SetPreLit(true);
    mOverlayMat->SetBlend(RndMat::kBlendSrcAlpha);
    mOverlayMat->SetZMode(kZModeForce);
    CreateAndSetMetaMat(mOverlayMat);
    mOverdrawMat = Hmx::Object::New<RndMat>();
    mOverdrawMat->SetUseEnv(false);
    mOverdrawMat->SetBlend(RndMat::kBlendSrcAlpha);
    mOverdrawMat->SetColor(1, 0, 0);
    mOverdrawMat->SetAlpha(0.2);
    CreateAndSetMetaMat(mOverdrawMat);
    for (unsigned int i = 0; i < kDefaultTex_Max; i++) {
        RELEASE(mDefaultTex[i]);
        mDefaultTex[i] = CreateDefaultTexture((DefaultTextureType)i);
    }
    RELEASE(mCubeTex_Black);
    RELEASE(mCubeTex_White);
    CreateCubeTextures();
}

int Rnd::CompressTexture(
    RndTex *tex, RndTex::AlphaCompress a, CompressTextureCallback *cb
) {
    FOREACH (it, mCompressTexDescs) {
        if (tex == (*it)->tex) {
            MILO_NOTIFY("%s: texture added to compression twice", PathName(tex));
        }
    }
    CompressTexDesc *desc = new CompressTexDesc(tex, a, cb);
    mCompressTexDescs.push_back(desc);
    return (int)desc;
}

void Rnd::PreClearDrawAddOrRemove(RndDrawable *d, bool b2, bool b3) {
    ObjPtrList<RndDrawable> &list = b3 ? mPreClearList : mSplashPreClearList;
    if (!b2) {
        list.remove(d);
    } else {
        PreClearCompilerHelper(list, d);
    }
}

void Rnd::UpdateHeap() {
    int lines;
    if (gCurHeap == -1) {
        lines = MemNumHeaps() + 1;
    } else {
        lines = 1;
    }
    mHeapOverlay->SetLines(lines);
    int i1;
    if (gCurHeap == -1) {
        i1 = -3;
    } else {
        i1 = gCurHeap == MemNumHeaps() ? -2 : gCurHeap;
    }
    char buf[2048];
    MemPrintOverview(i1, buf);
    *mHeapOverlay << buf;
}

void Rnd::UpdateRate() {
    mRateTotal += mDrawTimer.GetLastMs();
    static Timer *cpuTimer = AutoTimer::GetTimer("cpu");
    static Timer *gsTimer = AutoTimer::GetTimer("gs");
    if (gsTimer && cpuTimer && gsTimer->GetLastMs() > 16.7f) {
        if (gsTimer->GetLastMs() > cpuTimer->GetLastMs() + 0.1f) {
            mRateGate = " gs ";
        } else {
            mRateGate = " cpu";
        }
    }
    if (--mRateCount == 0) {
        *mRateOverlay << "rate:" << (mRateTotal ? (int)(5000.0f / mRateTotal + 0.5f) : 0)
                      << mRateGate;
        *mRateOverlay << "\n";
        mRateTotal = 0;
        mRateCount = 5;
        mRateGate = "    ";
    }
}

void WordWrap(const char *src, int wrap, char *dst, int dstSize) {
    char *dstEnd = dst + dstSize - 2;
    const char *srcEnd = src + strlen(src);
    while (true) {
        const char *srcSpace = nullptr;
        char *dstSpace = nullptr;
        for (int i = 0; i < wrap && src < srcEnd && dst < dstEnd && *src != '\n';
             src++, i++, dst++) {
            if (*src == ' ') {
                srcSpace = src;
                dstSpace = dst;
            }
            *dst = *src;
        }
        if (dst == dstEnd || src == srcEnd) {
            break;
        }

        if (*src != '\n') {
            if (srcSpace) {
                if (10 >= (int)src - (int)srcSpace) {
                    src = srcSpace;
                    dst = dstSpace;
                } else {
                    src--;
                }
            } else {
                src--;
            }
        }
        *dst++ = '\n';
        src++;
    }
    *dst = '\0';
}

void Rnd::Modal(Debug::ModalType &t, FixedString &msg, bool wait) {
    if (wait) {
        MILO_LOG("%s\n", msg.c_str());
    }
    if (CanModal(t)) {
        AutoSlowFrame frame(__FUNCTION__, 6e+06f);
        char buffer[4096];
        WordWrap(msg.c_str(), 90, buffer, sizeof(buffer));
        if (!wait) {
            strcat(buffer, "\n\n-- Waiting on Stack Trace --\n");
        } else if (t == Debug::kModalFail) {
            strcat(buffer, "\n\n-- Program ended --\n");
        } else {
            strcat(buffer, "\n\n-- Press any button to continue --\n");
        }
        bool showing = mConsole->Showing();
        mConsole->SetShowing(false);
        if (t != Debug::kModalFail || !wait) {
            RndSplasherSuspend();
        }
        ModalDraw(t, buffer);
        if (wait) {
            bool screensaver = ThePlatformMgr.ScreenSaver();
            ThePlatformMgr.SetScreenSaver(false);
            ThePlatformMgr.SetScreenSaver(screensaver);
            gFailKeepGoing = false;
            gNotifyKeepGoing = false;
            gFailRestartConsole = false;
            ModalKeyListener listener;
            KeyboardSubscribe(&listener);
            int i10 = 0x800;
            if (t != Debug::kModalFail) {
                i10 = -1;
            }
            while (!(JoypadPollForButton(-1) & i10)) {
                KeyboardPoll();
                ModalDraw(t, buffer);
                if (t == Debug::kModalFail && gFailKeepGoing) {
                    t = Debug::kModalNotify;
                    break;
                }
                if (t != Debug::kModalFail && gNotifyKeepGoing) {
                    break;
                }
                if (t == Debug::kModalFail && gFailRestartConsole) {
                    XLaunchNewImage(TheSystemArgs[0], 0);
                }
            }
            KeyboardUnsubscribe(&listener);
            mConsole->SetShowing(false);
            ModalDraw(t, "");
            RndSplasherResume();
        }
        mConsole->SetShowing(showing);
    }
}

void Rnd::TestPoint(const Vector3 &v, RndFlare *flare) {
    if (!TheHiResScreen.IsActive()) {
        if (RndCam::Current()->TargetTex()) {
            MILO_NOTIFY_ONCE("Flare %s can't be drawn in rendered texture", flare->Name());
            flare->SetUnks(true, false);
        } else {
            RndCam *cur = RndCam::Current();
            Vector2 v2;
            float f7 = cur->WorldToScreen(v, v2);
            if (f7 >= cur->NearPlane() && f7 <= cur->FarPlane() && v2.x >= 0 && v2.y >= 0
                && v2.x < 1 && v2.y < 1) {
                mPointTests.push_back(PointTest());
                PointTest &back = mPointTests.back();
                back.unkc = flare;
                back.unk0 = mWidth * v2.x;
                back.unk4 = mHeight * v2.y;
                back.unk8 = cur->ProjectZ(f7);
                return;
            }
            flare->SetUnks(true, false);
        }
    }
}

RndTex *Rnd::CreateDefaultTexture(DefaultTextureType textureType) {
    MILO_ASSERT(textureType < kDefaultTex_Max, 0x5E4);
    // clang-format off
    static const int sDefSize[kDefaultTex_Max][2] = { 
        8, 8, 
        8, 8,   
        8, 8,
        8, 8, 
        8, 8,   
        0x40, 0x40,
        0x100, 8, 
        0x80, 0x80
    };
    static const unsigned char sDefColor[kDefaultTex_Max][4] = {
        0, 0, 0, 0xFF, 
        0, 0, 0, 0,
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0,
        0x7f, 0x7f, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff,
        0,    0,    0,    0xff
    };
    // clang-format on
    int width = sDefSize[textureType][0];
    int height = sDefSize[textureType][1];
    unsigned char red = sDefColor[textureType][0];
    unsigned char green = sDefColor[textureType][1];
    unsigned char blue = sDefColor[textureType][2];
    unsigned char alpha = sDefColor[textureType][3];
    unsigned int order = GetDefaultTexBitmapOrder();
    RndBitmap bmap;
    bmap.Create(width, height, 0, 0x20, order, 0, 0, 0);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            bmap.SetPixelColor(j, i, red, green, blue, alpha);
        }
    }
    switch (textureType) {
    case kDefaultTex_Gradient: {
        for (int i = 0; i < width; i++) {
            unsigned char u10 = 0xFF - (i * 255) / (width - 1);
            for (int j = 0; j < height; j++) {
                bmap.SetPixelColor(i, j, u10, u10, u10, alpha);
            }
        }
        break;
    }
    case kDefaultTex_Hue: {
        Hmx::Color color;
        for (int i = 0; i < width; i++) {
            MakeColor((float)i / 255.0f, 1, 0.5f, color);
            unsigned char thisRed = color.red * 255.0f;
            unsigned char thisGreen = color.green * 255.0f;
            unsigned char thisBlue = color.blue * 255.0f;
            for (int j = 0; j < height; j++) {
                bmap.SetPixelColor(i, j, thisRed, thisGreen, thisBlue, alpha);
            }
        }
        break;
    }
    case kDefaultTex_Error: {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int blk_y = i >> 2;
                int blk_x = j >> 2;

                if (((blk_y ^ blk_x)) & 1) {
                    bmap.SetPixelColor(j, i, 0xff, 0x80, 0x40, alpha);
                } else {
                    bmap.SetPixelColor(j, i, 0, 0, 0, alpha);
                }
            }
        }
        break;
    }
    default:
        break;
    }
    EndianSwapBitmap(bmap);
    RndTex *tex = Hmx::Object::New<RndTex>();
    tex->SetBitmap(bmap, nullptr, true, RndTex::kRegular);
    return tex;
}

float Rnd::DrawTimers(float y) {
    static DataArray *timerScriptArr =
        SystemConfig("rnd")->FindArray("timer_script", false);
    if (timerScriptArr) {
        timerScriptArr->ExecuteScript(1, nullptr, nullptr, 1);
    }
    if (mVerboseTimers) {
        AutoTimer::CollectTimerStats();
    }
    int numDrawnTimers = 0;
    FOREACH (it, AutoTimer::Timers()) {
        if (it->first.Draw()) {
            numDrawnTimers++;
        }
    }
    float numTimersFloat = numDrawnTimers * 0.045f;
    Hmx::Rect r(0.025f, y, 0.95f, numTimersFloat);
    DrawRectScreen(r, Hmx::Color(0, 0, 0, 0.5f), mOverlayMat, nullptr, nullptr);
    Hmx::Color half(0.5f, 0.5f, 0.5f);
    Hmx::Color red(0.5f, 0, 0);
    Hmx::Color gb(0, 0.5f, 0.5f);
    r.h = 0.0268f;
    FOREACH (it, AutoTimer::Timers()) {
        Timer &curTimer = it->first;
        if (curTimer.Draw()) {
            float budget = curTimer.Budget();
            bool check = budget != 0 && curTimer.GetLastMs() > budget;
            if (check) {
                r.w = budget * 0.019f;
                DrawRectScreen(r, half, nullptr, nullptr, nullptr);
                r.x = r.w + 0.025f;
                r.w = (curTimer.GetLastMs() - curTimer.Budget()) * 0.019f;
                DrawRectScreen(r, red, nullptr, nullptr, nullptr);
            } else {
                r.w = curTimer.GetLastMs() * 0.019f;
                DrawRectScreen(r, half, nullptr, nullptr, nullptr);
            }
            if (curTimer.GetWorstMs() > curTimer.GetLastMs()) {
                r.x += r.w;
                r.w = (curTimer.GetWorstMs() - curTimer.GetLastMs()) * 0.019f;
                DrawRectScreen(r, gb, nullptr, nullptr, nullptr);
            }
            r.x = 0.025f;
            r.y += 0.045f;
        }
    }
    r.y = y;
    r.h = numTimersFloat;
    half.Set(0.25f, 0.25f, 0.25f);
    r.w = 0.001f;
    for (int i = 0; i < 10; i++) {
        DrawRectScreen(r, half, nullptr, nullptr, nullptr);
        r.x += 0.095f;
    }

    half.Set(1, 1, 1);
    Vector2 v2(0.02891f, y + 0.00446f);
    FOREACH (it, AutoTimer::Timers()) {
        Timer &curTimer = it->first;
        if (curTimer.Draw()) {
            float lastMs = curTimer.GetLastMs();
            if (lastMs >= 0.05f) {
                if (mVerboseTimers && AutoTimer::CollectingStats()) {
                    TimerStats &curStats = it->second;
                    DrawStringScreen(
                        MakeString(
                            "%s %2.1f (%.2f, %.2f) %.2f",
                            curTimer.Name(),
                            lastMs,
                            curStats.mAvgMs,
                            curStats.mStdDevMs,
                            curStats.mMaxMs
                        ),
                        v2,
                        half,
                        true
                    );
                } else {
                    DrawStringScreen(
                        MakeString(
                            "%s %.2f (%.2f)",
                            curTimer.Name(),
                            lastMs,
                            curTimer.GetWorstMs()
                        ),
                        v2,
                        half,
                        true
                    );
                }
            } else {
                DrawStringScreen(curTimer.Name().Str(), v2, half, true);
            }
            v2.y += 0.045f;
        }
    }
    return v2.y;
}
