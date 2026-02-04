#include "world/Dir.h"
#include "Dir.h"
#include "PhysicsManager.h"
#include "SpotlightDrawer.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "os/Timer.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Cam.h"
#include "rndobj/Dir.h"
#include "rndobj/Mat.h"
#include "rndobj/PostProc.h"
#include "rndobj/Rnd.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"
#include "synth/FxSend.h"
#include "ui/PanelDir.h"
#include "utl/BinStream.h"
#include "world/CameraManager.h"

WorldDir *TheWorld;
std::vector<FilePath> gOldChars;
ObjectDir *gOldTexDir;

void SetTheWorld(WorldDir *w) {
    static DataNode &n = DataVariable("world");
    n = w;
    TheWorld = w;
}

WorldDir::WorldDir()
    : mPresetOverrides(this), mBitmapOverrides(this), mMatOverrides(this),
      mHideOverrides(this), mCamShotOverrides(this), mPS3PerPixelShows(this),
      mPS3PerPixelHides(this), mHUDDir(0), mShowHUD(0), mHUD(this), mCameraMgr(this),
      unk314(0), m3DSoundMgr(this), mLightPresetMgr(this), mPhysicsMgr(0), unk3e0(0),
      mEchoMsgs(0), unk3f4(0), mTestLightPreset1(this), mTestLightPreset2(this),
      mTestAnimTime(10), mExplicitPostProc(1) {
    ClearDeltas();
}

WorldDir::~WorldDir() {
    RELEASE(mHUDDir);
    SpotlightDrawer::Current()->ClearLights();
    if (TheWorld == this) {
        SetTheWorld(nullptr);
    }
    RELEASE(mPhysicsMgr);
    if (unk314) {
        RELEASE(mCameraMgr);
    }
}

BEGIN_HANDLERS(WorldDir)
    if (mEchoMsgs && !_warn) {
        MILO_LOG("World msg: %s\n", sym);
    }
    HANDLE(get_physics_mgr, OnGetPhysicsManager)
    HANDLE_ACTION(sync_physics, mPhysicsMgr->SyncObjects(_msg->Int(2)))
    HANDLE_ACTION(
        reset_collidable_trans, GetPhysicsManager()->ResetTrans(_msg->Obj<Hmx::Object>(2))
    )
    HANDLE_ACTION(
        set_physics_driven,
        GetPhysicsManager()->MakePhysicsDriven(_msg->Obj<Hmx::Object>(2))
    )
    HANDLE_ACTION(
        set_anim_driven, GetPhysicsManager()->MakeAnimDriven(_msg->Obj<Hmx::Object>(2))
    )
    HANDLE_ACTION(reset_trans, GetPhysicsManager()->ResetTrans(_msg->Obj<Hmx::Object>(2)))
    HANDLE_EXPR(get_camera_mgr, mCameraMgr)
    HANDLE_MEMBER_PTR((&mLightPresetMgr))
    HANDLE_SUPERCLASS(PanelDir)
END_HANDLERS

#define SYNC_PROP_OVERRIDE(s, member, sync_func)                                         \
    {                                                                                    \
        _NEW_STATIC_SYMBOL(s)                                                            \
        if (sym == _s) {                                                                 \
            if (!(_op & (kPropSize | kPropGet)))                                         \
                sync_func(false);                                                        \
            bool synced = PropSync(member, _val, _prop, _i + 1, _op);                    \
            if (synced) {                                                                \
                if (!(_op & (kPropSize | kPropGet)))                                     \
                    sync_func(true);                                                     \
                return true;                                                             \
            } else                                                                       \
                return false;                                                            \
        }                                                                                \
    }

void WorldDir::PresetOverride::Sync(bool set) {
    if (preset)
        preset->SetHue(set ? hue.Ptr() : nullptr);
}

BEGIN_CUSTOM_PROPSYNC(WorldDir::PresetOverride)
    SYNC_PROP_OVERRIDE(preset, o.preset, o.Sync)
    SYNC_PROP_OVERRIDE(hue, o.hue, o.Sync)
END_CUSTOM_PROPSYNC

BEGIN_CUSTOM_PROPSYNC(WorldDir::BitmapOverride)
    SYNC_PROP_OVERRIDE(original, o.original, o.Sync)
    SYNC_PROP_OVERRIDE(replacement, o.replacement, o.Sync)
END_CUSTOM_PROPSYNC

void WorldDir::MatOverride::Sync(bool b) {
    if (!mat || !mesh)
        return;
    else if (!b) {
        if (mat2)
            mesh->SetMat(mat2);
    } else {
        mat2 = mesh->Mat();
        mesh->SetMat(mat);
    }
}

BEGIN_CUSTOM_PROPSYNC(WorldDir::MatOverride)
    SYNC_PROP_OVERRIDE(mesh, o.mesh, o.Sync)
    SYNC_PROP_OVERRIDE(mat, o.mat, o.Sync)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(WorldDir)
    SYNC_PROP_MODIFY(hud_filename, mHUDFilename, SyncHUD())
    SYNC_PROP_MODIFY(show_hud, mShowHUD, SyncHUD())
    SYNC_PROP(echo_msgs, mEchoMsgs)
    SYNC_PROP_OVERRIDE(hide_overrides, mHideOverrides, SyncHides)
    SYNC_PROP(bitmap_overrides, mBitmapOverrides)
    SYNC_PROP(mat_overrides, mMatOverrides)
    SYNC_PROP(preset_overrides, mPresetOverrides)
    SYNC_PROP_OVERRIDE(camshot_overrides, mCamShotOverrides, SyncCamShots)
    SYNC_PROP(ps3_per_pixel_hides, mPS3PerPixelHides)
    SYNC_PROP(ps3_per_pixel_shows, mPS3PerPixelShows)
    SYNC_PROP(test_light_preset_1, mTestLightPreset1)
    SYNC_PROP(test_light_preset_2, mTestLightPreset2)
    SYNC_PROP(test_animation_time, mTestAnimTime)
    SYNC_PROP_MODIFY(hud, mHUD, SyncObjects())
    SYNC_PROP_SET(
        doppler_power, m3DSoundMgr.mDopplerPower, m3DSoundMgr.mDopplerPower = _val.Float()
    )
    SYNC_PROP_SET(
        listener,
        m3DSoundMgr.mListener.Ptr(),
        m3DSoundMgr.SetListener(_val.Obj<RndTransformable>())
    )
    SYNC_PROP(explicit_postproc, mExplicitPostProc)
    SYNC_SUPERCLASS(PanelDir)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const WorldDir::PresetOverride &o) {
    bs << o.preset << o.hue;
    return bs;
}

BinStream &operator<<(BinStream &bs, const WorldDir::BitmapOverride &o) {
    bs << o.original << o.replacement;
    return bs;
}

BinStream &operator<<(BinStream &bs, const WorldDir::MatOverride &o) {
    bs << o.mesh << o.mat << o.mat2;
    return bs;
}

BEGIN_SAVES(WorldDir)
    SAVE_REVS(0x1D, 1)
    bs << mHUDFilename;
    SAVE_SUPERCLASS(PanelDir)
    bs << mHideOverrides << mBitmapOverrides << mMatOverrides << mPresetOverrides;
    bs << mCamShotOverrides << mPS3PerPixelHides << mPS3PerPixelShows;
    bs << mTestLightPreset1 << mTestLightPreset2 << mTestAnimTime;
    bs << mHUD;
    bs << m3DSoundMgr.mDopplerPower;
    ObjPtr<RndTransformable> listener(this, m3DSoundMgr.mListener);
    bs << listener;
    bs << mExplicitPostProc;
END_SAVES

BEGIN_COPYS(WorldDir)
    COPY_SUPERCLASS(PanelDir)
    CREATE_COPY(WorldDir)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mHUDFilename)
        SyncHides(false);
        COPY_MEMBER(mHideOverrides)
        SyncHides(true);
        SyncBitmaps(false);
        COPY_MEMBER(mBitmapOverrides)
        SyncBitmaps(true);
        SyncMats(false);
        COPY_MEMBER(mMatOverrides)
        SyncMats(true);
        SyncPresets(false);
        COPY_MEMBER(mPresetOverrides)
        SyncPresets(true);
        SyncCamShots(false);
        COPY_MEMBER(mCamShotOverrides)
        SyncCamShots(true);
        COPY_MEMBER(mPS3PerPixelHides)
        COPY_MEMBER(mPS3PerPixelShows)
        COPY_MEMBER(mTestLightPreset1)
        COPY_MEMBER(mTestLightPreset2)
        COPY_MEMBER(mTestAnimTime)
        COPY_MEMBER(mHUD)
        COPY_MEMBER(m3DSoundMgr.mDopplerPower)
        m3DSoundMgr.SetListener(c->m3DSoundMgr.mListener);
        SyncHUD();
        COPY_MEMBER(mExplicitPostProc)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(0x1D, 1)

void WorldDir::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(0x1D, 1)
    MILO_ASSERT(d.rev >= 4, 0x174);
    if (d.rev > 0 && d.rev < 5) {
        ObjPtr<RndCam> cam(this);
        d >> cam;
    }
    if (d.rev > 1 && d.rev < 0x15) {
        int x, y;
        d >> x >> y;
    }
    if (d.rev > 9) {
        d >> mHUDFilename;
    }
    if (d.rev < 9) {
        if (d.rev > 7) {
            OldLoadProxies(d.stream, 0);
        } else if (d.rev > 2) {
            d >> gOldChars;
        }
    }
    PanelDir::PreLoad(d.stream);
    d.PushRev(this);
}

BinStream &operator>>(BinStream &bs, WorldDir::PresetOverride &o) {
    bs >> o.preset >> o.hue;
    return bs;
}

BinStreamRev &operator>>(BinStreamRev &d, WorldDir::PresetOverride &o) {
    d.stream >> o;
    return d;
}

BinStream &operator>>(BinStream &bs, WorldDir::BitmapOverride &c) {
    bs >> c.original;
    if (gOldTexDir) {
        FilePath bitmap;
        bs >> bitmap;
        if (!bitmap.empty()) {
            const char *chr = strrchr(bitmap.c_str(), 0x2F);
            if (!chr) {
                chr = bitmap.c_str();
            } else {
                chr = chr + 1;
            }
            c.replacement = gOldTexDir->Find<RndTex>(chr, false);
            if (!c.replacement) {
                MILO_WARN(
                    "Loading %s synchronously, please resave %s",
                    chr,
                    gOldTexDir->Loader()->LoaderFile()
                );
                c.replacement = gOldTexDir->New<RndTex>(chr);
                c.replacement->SetBitmap(bitmap);
            } else {
                MILO_ASSERT(c.replacement->File() == bitmap, 0x163);
            }
        } else
            c.replacement = nullptr;
    } else
        bs >> c.replacement;
    return bs;
    return bs;
}

BinStreamRev &operator>>(BinStreamRev &d, WorldDir::BitmapOverride &o) {
    d.stream >> o;
    return d;
}

BinStream &operator>>(BinStream &bs, WorldDir::MatOverride &o) {
    bs >> o.mesh >> o.mat;
    return bs;
}

BinStreamRev &operator>>(BinStreamRev &d, WorldDir::MatOverride &o) {
    d.stream >> o;
    return d;
}

void WorldDir::PostLoad(BinStream &bs) {
    BinStreamRev d(bs, bs.PopRev(this));
    PanelDir::PostLoad(bs);
    if (d.rev > 4 && d.rev < 6) {
        ObjPtr<RndCam> cam(this);
        d >> cam;
        SetCam(cam);
    }
    if (d.rev < 8) {
        for (int i = 0; i < gOldChars.size(); i++) {
            RndDir *p = dynamic_cast<RndDir *>(
                Hmx::Object::NewObject(DirLoader::GetDirClass(gOldChars[i].c_str()))
            );
            MILO_ASSERT(p, 0x1AA);
            p->SetProxyFile(gOldChars[i], false);
            char buf[0x80];
            d.stream.ReadString(buf, 0x80);
            p->SetName(buf, this);
            p->RndTransformable::Load(d.stream);
            bool showing;
            d >> showing;
            float fff;
            d.stream >> fff;
            if (p) {
                p->SetShowing(showing);
                p->SetOrder(fff);
            }
            d.stream.ReadString(buf, 0x80);
            if (p && *buf != '\0') {
                p->SetEnv(Find<RndEnviron>(buf, true));
            }
        }
        gOldChars.clear();
    }
    if (d.rev < 0x19) {
        if (d.rev > 0xA) {
            Transform tf;
            d >> tf;
        } else if (d.rev > 6 && mCam) {
            mCam->RndTransformable::Load(d.stream);
        }
    }
    if (d.rev > 0xB) {
        SyncHides(false);
        d >> mHideOverrides;
        SyncHides(true);
        SyncBitmaps(false);
        gOldTexDir = d.rev > 0xC ? nullptr : Dir();
        d >> mBitmapOverrides;
        SyncBitmaps(true);
    }
    if (d.rev > 0xD) {
        SyncMats(false);
        d >> mMatOverrides;
        SyncMats(true);
    }
    if (d.rev > 0xE) {
        SyncPresets(false);
        d >> mPresetOverrides;
        SyncPresets(true);
    }
    if (d.rev > 0xF) {
        SyncCamShots(false);
        mCamShotOverrides.Load(d.stream, false, nullptr, true);
        SyncCamShots(true);
    }
    if (d.rev > 0x10 && d.rev != 0x17) {
        d >> mPS3PerPixelHides >> mPS3PerPixelShows;
    }
    if (d.rev > 0x11 && d.rev < 0x16) {
        Symbol s;
        d >> s;
    }
    if (d.rev > 0x12) {
        d >> mTestLightPreset1 >> mTestLightPreset2 >> mTestAnimTime;
    }
    if (d.rev > 0x13) {
        d >> mHUD;
    }
    SyncHUD();
    if (d.rev == 0x1A) {
        ObjPtr<FxSend> send(this);
        d >> send;
    }
    if (d.rev >= 0x1C) {
        float x;
        d >> x;
        m3DSoundMgr.mDopplerPower = x;
    }
    if (d.rev >= 0x1D) {
        ObjPtr<RndTransformable> trans(this);
        d >> trans;
        m3DSoundMgr.SetListener(trans);
    }
    if (d.altRev > 0) {
        d >> mExplicitPostProc;
    }
}

// class PhysicsManager * (__cdecl* CreatePhysicsManager)(class RndDir *)

void WorldDir::SyncObjects() {
    PanelDir::SyncObjects();
    if (!unk314) {
        mCameraMgr = nullptr;
    }
    if (IsSubDir())
        return;
    if (!mCameraMgr) {
        ObjDirItr<CameraManager> it(this, true);
        if (it) {
            mCameraMgr = it;
        }
    }
    if (!mCameraMgr) {
        unk314 = true;
        mCameraMgr = new CameraManager(this);
    }
    if (mCameraMgr) {
        mCameraMgr->SyncObjects(this);
    }
    m3DSoundMgr.SyncObjects();
    mLightPresetMgr.SyncObjects();
    if (!mPhysicsMgr) {
        if (CreatePhysicsManager) {
            mPhysicsMgr = CreatePhysicsManager(this);
            if (unk3e0) {
                mPhysicsMgr->Enter();
            }
        }
    }
    mPhysicsMgr->SyncObjects(false);
    if (mHUD) {
        VectorRemove(mDraws, mHUD);
    }
}

void WorldDir::Poll() {
    START_AUTO_TIMER("world_poll");
    if (TheWorld) {
        MILO_ASSERT(TheWorld != this, 0xC3);
        RndDir::Poll();
    } else {
        SetTheWorld(this);
        float deltas[4];
        AccumulateDeltas(deltas);
        bool b = (unk3f4 || (TheRnd.ProcCmds() & kProcessPost));
        unk3f4 = false;
        if (b) {
            for (int i = 0; i < 4; i++) {
                TheTaskMgr.SetDeltaTime((TaskUnits)i, mDeltaSincePoll[i]);
            }
            static Message select_camera("select_camera");
            HandleType(select_camera);
            if (mCameraMgr) {
                mCameraMgr->PrePoll();
            }
            m3DSoundMgr.Poll();
            mLightPresetMgr.Poll();
            RndDir::Poll();
            if (mCameraMgr) {
                mCameraMgr->Poll();
            }
            {
                START_AUTO_TIMER("phys_mgr_poll");
                if (mPhysicsMgr) {
                    mPhysicsMgr->Poll();
                }
            }
            RestoreDeltas(deltas);
        }
        SetTheWorld(nullptr);
    }
}

void WorldDir::Enter() {
    if (!TheWorld) {
        SetTheWorld(this);
        static DataNode &n = DataVariable("world.last_entered");
        n = this;
    }
    mLightPresetMgr.Enter();
    if (mCameraMgr) {
        mCameraMgr->Enter();
    }
    if (mPhysicsMgr) {
        mPhysicsMgr->Enter();
    } else {
        unk3e0 = true;
    }
    PanelDir::Enter();
    ClearDeltas();
    unk3f4 = true;
    TheRnd.SetProcAndLock(false);
    TheRnd.ResetProcCounter();
    if (TheWorld == this) {
        SetTheWorld(nullptr);
    }
}

void WorldDir::SyncBitmaps(bool b) {
    FOREACH (it, mBitmapOverrides) {
        it->Sync(b);
    }
}

void WorldDir::SyncMats(bool b) {
    FOREACH (it, mMatOverrides) {
        it->Sync(b);
    }
}

void WorldDir::SyncPresets(bool b) {
    FOREACH (it, mPresetOverrides) {
        it->Sync(b);
    }
}

void WorldDir::ClearDeltas() {
    for (int i = 0; i < 4; i++)
        mDeltaSincePoll[i] = 0;
}

void WorldDir::Init() {
    REGISTER_OBJ_FACTORY(WorldDir)
    SetTheWorld(nullptr);
    sGlowMat = Hmx::Object::New<RndMat>();
    sGlowMat->SetBlend(BaseMaterial::kBlendSrcAlpha);
    sGlowMat->SetZMode(kZModeDisable);
    sGlowMat->SetPreLit(true);
    CreateAndSetMetaMat(sGlowMat);
}

void WorldDir::AccumulateDeltas(float *deltas) {
    for (int i = 0; i < 4; i++) {
        deltas[i] = TheTaskMgr.DeltaTime((TaskUnits)i);
        mDeltaSincePoll[i] += deltas[i];
    }
}

void WorldDir::RestoreDeltas(float *f) {
    for (int i = 0; i < 4; i++) {
        mDeltaSincePoll[i] = 0;
        TheTaskMgr.SetDeltaTime((TaskUnits)i, f[i]);
    }
}

void WorldDir::SyncHUD() {
    RELEASE(mHUDDir);
    if (mShowHUD && !mHUDFilename.empty()) {
        mHUDDir = dynamic_cast<RndDir *>(
            DirLoader::LoadObjects(mHUDFilename, nullptr, nullptr)
        );
        if (mHUDDir)
            mHUDDir->Enter();
    }
}

void WorldDir::SyncHides(bool b) {
    FOREACH (it, mHideOverrides) {
        (*it)->SetShowing(!b);
    }
}

void WorldDir::SyncCamShots(bool b) {
    FOREACH (it, mCamShotOverrides) {
        (*it)->Disable(b, 1);
    }
}

DataNode WorldDir::OnGetPhysicsManager(const DataArray *) { return mPhysicsMgr; }
