#include "world/Dir.h"
#include "Dir.h"
#include "SpotlightDrawer.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"
#include "ui/PanelDir.h"

WorldDir *TheWorld;

WorldDir::WorldDir()
    : mPresetOverrides(this), mBitmapOverrides(this), mMatOverrides(this),
      mHideOverrides(this), mCamShotOverrides(this), mPS3PerPixelShows(this),
      mPS3PerPixelHides(this), unk2e4(0), mShowHUD(0), mHUD(this), unk300(this),
      unk314(0), m3DSoundMgr(this), mLightPresetMgr(this), mPhysicsMgr(0), unk3e0(0),
      mEchoMsgs(0), unk3f4(0), mTestLightPreset1(this), mTestLightPreset2(this),
      mTestAnimTime(10), mExplicitPostProc(1) {
    ClearDeltas();
}

WorldDir::~WorldDir() {
    RELEASE(unk2e4);
    SpotlightDrawer::Current()->ClearLights();
    if (TheWorld == this) {
        SetTheWorld(nullptr);
    }
    RELEASE(mPhysicsMgr);
    if (unk314) {
        RELEASE(unk300);
    }
}

BEGIN_HANDLERS(WorldDir)
    if (mEchoMsgs && !_warn) {
        MILO_LOG("World msg: %s\n", sym);
    }
    HANDLE(get_physics_mgr, OnGetPhysicsManager)
    HANDLE_ACTION(sync_physics, mPhysicsMgr->SyncObjects(_msg->Int(2)))
    HANDLE_ACTION(
        reset_collidable_trans, mPhysicsMgr->ResetTrans(_msg->Obj<Hmx::Object>(2))
    )
    HANDLE_ACTION(
        set_physics_driven, mPhysicsMgr->MakePhysicsDriven(_msg->Obj<Hmx::Object>(2))
    )
    HANDLE_ACTION(set_anim_driven, mPhysicsMgr->MakeAnimDriven(_msg->Obj<Hmx::Object>(2)))
    HANDLE_ACTION(reset_trans, mPhysicsMgr->ResetTrans(_msg->Obj<Hmx::Object>(2)))
    HANDLE_EXPR(get_camera_mgr, unk300)
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

BEGIN_CUSTOM_PROPSYNC(WorldDir::PresetOverride)
    SYNC_PROP_OVERRIDE(preset, o.preset, o.Sync)
    SYNC_PROP_OVERRIDE(hue, o.hue, o.Sync)
END_CUSTOM_PROPSYNC

BEGIN_CUSTOM_PROPSYNC(WorldDir::BitmapOverride)
    SYNC_PROP_OVERRIDE(original, o.original, o.Sync)
    SYNC_PROP_OVERRIDE(replacement, o.replacement, o.Sync)
END_CUSTOM_PROPSYNC

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

void WorldDir::ClearDeltas() {
    for (int i = 0; i < 4; i++)
        mDeltaSincePoll[i] = 0;
}

void WorldDir::Init() {
    REGISTER_OBJ_FACTORY(WorldDir)
    SetTheWorld(nullptr);
}
