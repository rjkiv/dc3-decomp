#include "lazer/meta_ham/HamUI.h"
#include "gesture/DrawUtl.h"
#include "gesture/GestureMgr.h"
#include "gesture/SkeletonViz.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "math/Rot.h"
#include "meta/HAQManager.h"
#include "meta_ham/BlacklightPanel.h"
#include "meta_ham/HelpBarPanel.h"
#include "meta_ham/LetterboxPanel.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/UIEventMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "rndobj/Dir.h"
#include "rndobj/Overlay.h"
#include "ui/UI.h"
#include "ui/UIPanel.h"
#include "ui/UIScreen.h"
#include "utl/Symbol.h"

namespace {
    UIPanel *FindPanel(const char *name) {
        UIPanel *p = ObjectDir::Main()->Find<UIPanel>(name);
        MILO_ASSERT(p->CheckIsLoaded(), 0x99);
        MILO_ASSERT(p->LoadedDir(), 0x9A);
        return p;
    }
}

HamUI::HamUI() {
    mHelpBar = 0;
    mLetterbox = nullptr;
    mBlacklight = nullptr;
    mEventDialogPanel = nullptr;
    mBackgroundPanel = nullptr;
    mContentLoadingPanel = nullptr;
    mOverlayPanel = 0;
    mGamePanel = 0;
    unk_0xF8 = nullptr;
    unk_0xFC = 0;
    unk_0xFD = 0;
    mEventScreen = 0;
    mShellInput = new ShellInput;
    unk_0x108 = 0;
    unk_0x118 = 0;
    unk_0x10C = -1;
    mUIOverlay = 0;
    mSkelRot = 0.0f;
    SetFullScreenDraw(false);
}

HamUI::~HamUI() { RELEASE(unk_0xF8); }

BEGIN_HANDLERS(HamUI)
    HANDLE_EXPR(display_next_camera_output, DisplayNextCameraOutput())
    HANDLE_EXPR(toggle_draw_skeletons, ToggleDrawSkeletons())
    HANDLE_EXPR(toggle_full_screen_draw, SetFullScreenDraw(mFullScreenDrawActive))
    HANDLE_EXPR(next_skeleton_draw_rot, NextSkeletonDrawRot())
    HANDLE_EXPR(cycle_draw_cursor, mShellInput->CycleDrawCursor())
    HANDLE_EXPR(set_button_spam, unk_0x118 = _msg->Int(2))
    HANDLE_EXPR(button_spam, unk_0x118)
    // HANDLE_EXPR(toggle_ui_overlay, mUIOverlay)
    HANDLE_ACTION(toggle_letterbox, mLetterbox->ToggleBlacklightMode(0))
    HANDLE_ACTION(toggle_letterbox_immediately, mLetterbox->ToggleBlacklightMode(1))
    HANDLE_EXPR(is_letterbox_in_transition, mLetterbox->InBlacklightTransition())
    HANDLE_EXPR(is_blacklight_mode, mLetterbox->IsBlacklightMode())
    HANDLE_ACTION(force_letterbox_off, ForceLetterboxOff())
    HANDLE_ACTION(force_letterbox_off_immediate, ForceLetterboxOffImmediate())
    HANDLE_ACTION(reset_snapshots, ResetSnapshots())
    HANDLE_EXPR(take_snapshot, TakeSnapshot())
    HANDLE_EXPR(num_snapshots, NumSnapshots())
    HANDLE_ACTION(
        apply_snapshot_to_mesh, ApplySnapshotToMesh(_msg->Int(2), _msg->Obj<RndMesh>(3))
    )
    HANDLE_EXPR(get_augmented_photo, unk_0xF8)
    HANDLE_EXPR(has_overlay_panel, mOverlayPanel != 0)
    HANDLE_ACTION(init_texture_store, InitTextureStore(_msg->Int(2)))
    HANDLE_ACTION(clear_texture_store, ClearTextureStore())
    HANDLE_EXPR(num_texture_store, NumStoredTextures())
    HANDLE_EXPR(store_texture, StoreTexture(_msg->Obj<RndTex>(2)))
    HANDLE_ACTION(store_texture_at, StoreTextureAt(_msg->Obj<RndTex>(2), _msg->Int(3)))
    HANDLE_ACTION(
        store_texture_clip_at,
        StoreTextureClipAt(
            _msg->Float(2),
            _msg->Float(3),
            _msg->Float(4),
            _msg->Float(5),
            _msg->Int(6),
            _msg->Int(7)
        )
    )
    HANDLE_EXPR(get_stored_texture, GetStoredTexture(_msg->Int(2)))
    HANDLE_ACTION(apply_texture_clip, ApplyTextureClip(_msg->Obj<RndMat>(2), _msg->Int(3)))
    HANDLE_ACTION(store_color_buff_at, StoreColorBufferAt(_msg->Int(2)))
    HANDLE_ACTION(
        store_color_buff_clip_at,
        StoreColorBufferClipAt(
            _msg->Float(2), _msg->Float(3), _msg->Float(4), _msg->Float(5), _msg->Int(6)
        )
    )
    HANDLE_ACTION(store_depth_buffer_at, StoreDepthBufferAt(_msg->Int(2)))
    HANDLE_ACTION(
        store_depth_buff_clip_at,
        StoreDepthBufferClipAt(
            _msg->Float(2), _msg->Float(3), _msg->Float(4), _msg->Float(5), _msg->Int(6)
        )
    )
    HANDLE_ACTION(reload_strings, ReloadStrings())
    HANDLE_MESSAGE(UITransitionCompleteMsg)
    HANDLE_MESSAGE(ContentReadFailureMsg)
    HANDLE_MESSAGE(ConnectionStatusChangedMsg)
    HANDLE_MESSAGE(DiskErrorMsg)
    HANDLE_MESSAGE(ButtonDownMsg)
    // HANDLE_MESSAGE(KinectGuideGestureMsg)
    HANDLE_SUPERCLASS(UIManager)
END_HANDLERS

void HamUI::Init() {
    ThePlatformMgr.AddSink(this, "connection_status_changed");
    ThePlatformMgr.AddSink(this, "disk_error");
    ThePlatformMgr.AddSink(this, "kinect_guide_gesture");
    TheContentMgr.SetReadFailureHandler(this);
    UIEventMgr::Init();
    UIManager::Init();
    mShellInput->Init();
    UIPanel *helpBarPanel = ObjectDir::Main()->Find<UIPanel>("helpbar", false);
    for (ObjDirItr<UIScreen> it(ObjectDir::Main(), true); it != nullptr; ++it) {
        // HamScreen*
    }
    static Symbol ui("ui");
    mUIOverlay = RndOverlay::Find(ui, false);
    const char *photo = nullptr;
    if (SystemConfig("ui")->FindData("augmented_photo", photo, false) && photo) {
        ObjectDir *dir = DirLoader::LoadObjects(photo, nullptr, nullptr);
        unk_0xF8 = dynamic_cast<RndDir *>(dir);
        if (!unk_0xF8 && dir) {
            delete dir;
        }
    }
}

void HamUI::Terminate() {
    ThePlatformMgr.RemoveSink(this);
    UIManager::Terminate();
    UIEventMgr::Terminate();
    RELEASE(mShellInput);
}

void HamUI::Poll() {
    UIManager::Poll();
    mShellInput->Poll();
    TheProfileMgr.Poll();
}

bool HamUI::SetFullScreenDraw(bool b) {
    mFullScreenDrawActive = b;
    if (b) {
        SetDrawSpace(0, 0, 1);
    } else {
        SetDrawSpace(0.5, 0.05, 0.4);
    }
    return mFullScreenDrawActive;
}

float HamUI::NextSkeletonDrawRot() {
    mSkelRot += PI / 2;
    if (mSkelRot >= PI * 2) {
        mSkelRot = 0;
    }
    return mSkelRot * RAD2DEG;
}

int HamUI::NumSnapshots() {
    MILO_ASSERT(TheGestureMgr, 586);
    if (TheGestureMgr->GetLiveCameraInput())
        return TheGestureMgr->GetLiveCameraInput()->NumSnapshots();
    return 0;
}

void HamUI::ResetSnapshots() {
    MILO_ASSERT(TheGestureMgr, 595);
    if (TheGestureMgr->GetLiveCameraInput())
        TheGestureMgr->GetLiveCameraInput()->ResetSnapshots();
}

int HamUI::TakeSnapshot() {
    MILO_ASSERT(TheGestureMgr, 602);
    unk_0xFC = true;
    for (int i = 0; i < 2; i++) {
        HamPlayerData *pPlayer = TheGameData->Player(i);
        MILO_ASSERT(pPlayer, 609);
        HamProfile *profile = TheProfileMgr.GetProfileFromPad(pPlayer->PadNum());
        if (profile) {
            if (profile->HasValidSaveData()) {
                profile->GetMetagameStats()->PhotoTaken();
            }
        }
    }
    return NumSnapshots();
}

void HamUI::ApplySnapshotToMesh(int idx, RndMesh *mesh) {
    MILO_ASSERT(TheGestureMgr, 635);
    if (TheGestureMgr->GetLiveCameraInput())
        mesh->SetMat(TheGestureMgr->GetLiveCameraInput()->GetSnapshot(idx));
}

void HamUI::InitTextureStore(int size) {
    MILO_ASSERT(TheGestureMgr, 648);
    if (TheGestureMgr->GetLiveCameraInput())
        TheGestureMgr->GetLiveCameraInput()->InitTextureStore(size);
}

void HamUI::ClearTextureStore() {
    MILO_ASSERT(TheGestureMgr, 656);
    if (TheGestureMgr->GetLiveCameraInput())
        TheGestureMgr->GetLiveCameraInput()->ClearTextureStore();
}

int HamUI::NumStoredTextures() const {
    MILO_ASSERT(TheGestureMgr, 664);
    if (TheGestureMgr->GetLiveCameraInput())
        return TheGestureMgr->GetLiveCameraInput()->NumStoredTextures();
    return 0;
}

int HamUI::StoreTexture(RndTex *tex) {
    MILO_ASSERT(TheGestureMgr, 674);
    if (TheGestureMgr->GetLiveCameraInput())
        return TheGestureMgr->GetLiveCameraInput()->StoreTexture(tex);
    return 0;
}

void HamUI::StoreTextureAt(RndTex *tex, int idx) {
    MILO_ASSERT(TheGestureMgr, 683);
    if (TheGestureMgr->GetLiveCameraInput())
        TheGestureMgr->GetLiveCameraInput()->StoreTextureAt(tex, idx);
}

void HamUI::StoreTextureClipAt(float f1, float f2, float f3, float f4, int i1, int i2) {
    MILO_ASSERT(TheGestureMgr, 691);
    if (TheGestureMgr->GetLiveCameraInput())
        TheGestureMgr->GetLiveCameraInput()->StoreTextureClipAt(f1, f2, f3, f4, i1, i2);
}

RndTex *HamUI::GetStoredTexture(int idx) const {
    MILO_ASSERT(TheGestureMgr, 699);
    if (TheGestureMgr->GetLiveCameraInput())
        return TheGestureMgr->GetLiveCameraInput()->GetStoredTexture(idx);
    return nullptr;
}

void HamUI::ApplyTextureClip(RndMat *mat, int idx) const {
    MILO_ASSERT(TheGestureMgr, 708);
    if (TheGestureMgr->GetLiveCameraInput())
        TheGestureMgr->GetLiveCameraInput()->ApplyTextureClip(mat, idx);
}

void HamUI::StoreColorBufferAt(int idx) {
    MILO_ASSERT(TheGestureMgr, 716);
    if (TheGestureMgr->GetLiveCameraInput())
        TheGestureMgr->GetLiveCameraInput()->StoreColorBuffer(idx);
}

void HamUI::StoreColorBufferClipAt(float f1, float f2, float f3, float f4, int i1) {
    MILO_ASSERT(TheGestureMgr, 724);
    if (TheGestureMgr->GetLiveCameraInput())
        TheGestureMgr->GetLiveCameraInput()->StoreColorBufferClip(f1, f2, f3, f4, i1);
}

void HamUI::StoreDepthBufferAt(int idx) {
    MILO_ASSERT(TheGestureMgr, 732);
    if (TheGestureMgr->GetLiveCameraInput())
        TheGestureMgr->GetLiveCameraInput()->StoreDepthBuffer(idx);
}

void HamUI::StoreDepthBufferClipAt(float f1, float f2, float f3, float f4, int i1) {
    MILO_ASSERT(TheGestureMgr, 740);
    if (TheGestureMgr->GetLiveCameraInput())
        TheGestureMgr->GetLiveCameraInput()->StoreDepthBufferClip(f1, f2, f3, f4, i1);
}

// Symbol HamUI::DisplayNextCameraOutput() { return Symbol(); }

DataNode HamUI::OnMsg(const UITransitionCompleteMsg &msg) {
    HAQManager::Print(HAQType::kHAQType_Screen);
    HAQManager::Print(HAQType::kHAQType_Focus);
    UIScreen *scr = msg->Obj<UIScreen>(2);
    if (scr) {
        Symbol scrname(scr->Name()), disable_screen_saver("disable_screen_saver");
        auto prop = scr->Property(disable_screen_saver, false);
    }

    return 1;
}

DataNode HamUI::OnMsg(DiskErrorMsg const &) {
    static Symbol disc_error("disc_error");
    TheUIEventMgr->TriggerEvent(disc_error, 0);
    return 1;
}

void HamUI::ReloadStrings() {
    Message reload("reload_strings");

    UIManager::ReloadStrings();
}

void HamUI::GotoEventScreen(UIScreen *scr) {
    mEventScreen = scr;
    AttemptEventTranstion();
}

void HamUI::AttemptEventTranstion() {
    MILO_ASSERT(mEventScreen, 0x1a7);
    if (!TheUI->InTransition()) {
        if (TheUI->BottomScreen() == TheUI->CurrentScreen()) {
            TheUI->GotoScreen(mEventScreen, 0, 0);
        } else {
            TheUI->PopScreen(mEventScreen);
        }
        mEventScreen = nullptr;
    }
}

bool HamUI::IsBlacklightMode() { return mLetterbox && mLetterbox->IsBlacklightMode(); }

void HamUI::ForceLetterboxOff() {
    MILO_LOG("HamUI::ForceLetterboxOff()\n");
    if (mLetterbox && mLetterbox->IsBlacklightMode())
        mLetterbox->ToggleBlacklightMode(false);
}

void HamUI::ForceLetterboxOffImmediate() {
    MILO_LOG("HamUI::ForceLetterboxOffImmediate()\n");
    if (mLetterbox && mLetterbox->IsBlacklightMode()) {
        mLetterbox->ToggleBlacklightMode(true);
        return;
    }
    if (mLetterbox && mLetterbox->IsLeavingBlacklightMode()) {
        MILO_LOG(
            "ForceLetterboxOffImmediate request - but blacklight is already transitioning to OFF\n"
        );
    }
}

void HamUI::InitPanels() {
    if (!mHelpBar) {
        mHelpBar = dynamic_cast<HelpBarPanel *>(FindPanel("helpbar"));
        mEventDialogPanel = FindPanel("event_dialog_panel");
        mContentLoadingPanel = FindPanel("content_loading_panel");
        mBackgroundPanel = ObjectDir::Main()->Find<UIPanel>("background_panel");
        // gamepanel find
        mLetterbox = dynamic_cast<LetterboxPanel *>(FindPanel("letterbox"));
        mBlacklight = dynamic_cast<BlacklightPanel *>(FindPanel("blacklight"));
    }
}

bool ToggleDrawSkeletons() {
    MILO_ASSERT(TheSkeletonViz, 0xe2);
    TheSkeletonViz->SetShowing(TheSkeletonViz->GetForceSubpartSelection());
    return TheSkeletonViz->GetForceSubpartSelection();
}
