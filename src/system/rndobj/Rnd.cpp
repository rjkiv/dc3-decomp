#include "rndobj/Rnd.h"
#include "AmbientOcclusion.h"
#include "CamAnim.h"
#include "Enter.h"
#include "Group.h"
#include "Morph.h"
#include "MotionBlur.h"
#include "MultiMeshProxy.h"
#include "PollAnim.h"
#include "PostProcMgr.h"
#include "PropAnim.h"
#include "ScreenMask.h"
#include "Shockwave.h"
#include "SoftParticles.h"
#include "Spline.h"
#include "TexBlendController.h"
#include "TexBlender.h"
#include "math/Color.h"
#include "obj/DataFunc.h"
#include "obj/Dir.h"
#include "os/Debug.h"
#include "os/OSFuncs.h"
#include "os/System.h"
#include "os/Timer.h"
#include "rndobj/AnimFilter.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Cam.h"
#include "rndobj/Console.h"
#include "rndobj/CubeTex.h"
#include "rndobj/DOFProc.h"
#include "rndobj/Dir.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/EventTrigger.h"
#include "rndobj/Flare.h"
#include "rndobj/Font.h"
#include "rndobj/FontBase.h"
#include "rndobj/Fur.h"
#include "rndobj/Gen.h"
#include "rndobj/Graph.h"
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
#include "rndobj/Movie.h"
#include "rndobj/MultiMesh.h"
#include "rndobj/MultiMeshProxy.h"
#include "rndobj/Part.h"
#include "rndobj/PartAnim.h"
#include "rndobj/PartLauncher.h"
#include "rndobj/Ribbon.h"
#include "rndobj/Set.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/ShaderOptions.h"
#include "rndobj/Tex.h"
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
#include "utl/Option.h"
#include "utl/TextStream.h"
#include "xdk/XAPILIB.h"

// Rnd & TheRnd;
bool gNotifyKeepGoing;
bool gFailKeepGoing;
bool gFailRestartConsole;

HANDLE gRndTextureEvent;
HANDLE gRndThread;

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
      mDefaultCam(0), mWorldCamCopy(0), mDefaultEnv(0), mDefaultLit(0), unk110(nullptr),
      unk114(nullptr), unk118(0), unk120(5), mFrameID(0), mRateGate("    "),
      mFont(nullptr), mSync(1), mGsTiming(0), mShowSafeArea(0), mDrawing(0),
      mWorldEnded(1), mAspect(kWidescreen), mDrawMode(kDrawNormal), unk140(0), unk141(0),
      mShrinkToSafe(1), mInGame(0), mVerboseTimers(0), mDisablePostProc(0), unk146(0),
      unk147(0), unk148(0), unk14c(0), unk150(0), mPostProcOverride(this),
      mPostProcBlackLightOverride(nullptr), unk18c(this), mDraws(this), unk1b4(0),
      mProcCmds(kProcessAll), mLastProcCmds(kProcessAll) {
    for (int i = 0; i < 8; i++)
        mDefaultTex[i] = nullptr;
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
    HANDLE_EXPR(overlay_showing, RndOverlay::Find(_msg->Str(2), true)->Showing())
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
    rndcfg->FindData("bpp", mScreenBpp, true);
    rndcfg->FindData("height", mHeight, true);
    rndcfg->FindData("clear_color", mClearColor, true);
    rndcfg->FindData("sync", mSync, true);
    rndcfg->FindData("aspect", (int &)mAspect, true);
    if (OptionBool("widescreen", false))
        mAspect = kWidescreen;
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
    mRateOverlay = RndOverlay::Find("rate", true);
    mHeapOverlay = RndOverlay::Find("heap", true);
    // well ok then
    mWatcher.SetOverlay(mWatchOverlay = RndOverlay::Find("watch", true));
    mWatcher.Init();
    mStatsOverlay = RndOverlay::Find("stats", true);
    mTimersOverlay = RndOverlay::Find("timers", true);
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

DWORD CompressThread(HANDLE h);
// {
//     while(true){
//         WaitForSingleObject(gRndTextureEvent, -1);
//         if(!sTexture) break;
//     }
// }

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
        if (unk14c) {
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
    for (std::list<PostProcessor *>::iterator it = mPostProcessors.begin();
         it != mPostProcessors.end();
         ++it) {
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
        for (std::list<PostProcessor *>::iterator it = mPostProcessors.begin();
             it != mPostProcessors.end();
             ++it) {
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
        for (std::list<PostProcessor *>::iterator it = mPostProcessors.begin();
             it != mPostProcessors.end();
             ++it) {
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
            for (std::list<PostProcessor *>::iterator it = mPostProcessors.begin();
                 it != mPostProcessors.end();
                 ++it) {
                (*it)->DoPost();
            }
        }
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
    RndOverlay *o = RndOverlay::Find(da->Str(2), true);
    String str;
    for (int i = 3; i < da->Size(); i++) {
        da->Evaluate(i).Print(str, true, 0);
    }
    o->Print(str.c_str());
    return 0;
}

DataNode Rnd::OnReflect(const DataArray *da) {
    RndOverlay *o = RndOverlay::Find(da->Sym(2), true);
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
    RndOverlay *o = RndOverlay::Find(da->Str(2), true);
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

void Rnd::UnregisterPostProcessor(PostProcessor *proc) { mPostProcessors.remove(proc); }

void PreClearCompilerHelper(ObjPtrList<RndDrawable> &list, RndDrawable *draw) {
    for (ObjPtrList<RndDrawable>::iterator it = list.begin(); it != list.end(); ++it) {
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
        mWorldCamCopy->Copy(cam ? cam : RndCam::Current(), kCopyShallow);
        mWorldCamCopy->SetTransParent(nullptr, false);
        unk147 = true;
    }
}

RndTex *Rnd::GetNullTexture() { return mDefaultTex[kUnk7]; }

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
    unk110 = Hmx::Object::New<RndCubeTex>();
    unk114 = Hmx::Object::New<RndCubeTex>();
    for (unsigned int i = 0; i < RndCubeTex::kNumCubeFaces; i++) {
        RndCubeTex::CubeFace cf = (RndCubeTex::CubeFace)i;
        RndBitmap &bm110 = unk110->GetBitmap(cf);
        RndBitmap &bm114 = unk114->GetBitmap(cf);
        bm110.Create(32, 32, 0, 32, 0, nullptr, nullptr, nullptr);
        bm114.Create(32, 32, 0, 32, 0, nullptr, nullptr, nullptr);
        for (int j = 0; j < 32; j++) {
            for (int k = 0; k < 32; k++) {
                bm110.SetPixelColor(k, j, 0, 0, 0, 0);
                bm114.SetPixelColor(k, j, 255, 255, 255, 255);
            }
        }
        unk110->UpdateFace(cf);
        unk114->UpdateFace(cf);
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
    RndOverlay *ppOverlay = RndOverlay::Find("postproc", true);
    if (ppOverlay->Showing()) {
        TextStream *old = TheDebug.Reflect();
        TheDebug.SetReflect(ppOverlay);
        MILO_LOG("SETPROSTPROCOVERRIDE: %s\n", pp == 0 ? "NULL" : PathName(pp));
        TheDebug.SetReflect(old);
    }
}

void Rnd::SetPostProcBlacklightOverride(RndPostProc *pp) {
    mPostProcBlackLightOverride = pp;
    RndOverlay *ppOverlay = RndOverlay::Find("postproc", true);
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
    RELEASE(unk110);
    RELEASE(unk114);
    CreateCubeTextures();
}

int Rnd::CompressTexture(
    RndTex *tex, RndTex::AlphaCompress a, CompressTextureCallback *cb
) {
    for (std::list<CompressTexDesc *>::iterator it = unk1d8.begin(); it != unk1d8.end();
         ++it) {
        if (tex == (*it)->tex) {
            MILO_NOTIFY("%s: texture added to compression twice", PathName(tex));
        }
    }
    CompressTexDesc *desc = new CompressTexDesc(tex, a, cb);
    unk1d8.push_back(desc);
    return (int)desc;
}

void Rnd::PreClearDrawAddOrRemove(RndDrawable *d, bool b2, bool b3) {
    ObjPtrList<RndDrawable> &list = b3 ? unk18c : mDraws;
    if (!b2) {
        list.remove(d);
    } else {
        PreClearCompilerHelper(list, d);
    }
}
