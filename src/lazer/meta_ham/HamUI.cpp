#include "lazer/meta_ham/HamUI.h"
#include "gesture/LiveCameraInput.h"
#include "SkeletonIdentifier.h"
#include "game/Game.h"
#include "game/GamePanel.h"
#include "game/PresenceMgr.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/DrawUtl.h"
#include "gesture/GestureMgr.h"
#include "gesture/LiveCameraInput.h"
#include "gesture/SkeletonViz.h"
#include "gesture/StreamRenderer.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "math/Rot.h"
#include "meta/HAQManager.h"
#include "meta_ham/BlacklightPanel.h"
#include "meta_ham/HamScreen.h"
#include "meta_ham/HelpBarPanel.h"
#include "meta_ham/LetterboxPanel.h"
#include "meta_ham/PassiveMessenger.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/ShellInput.h"
#include "meta_ham/SkeletonChooser.h"
#include "meta_ham/UIEventMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/Joypad.h"
#include "os/JoypadMsgs.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "rndobj/Dir.h"
#include "rndobj/Mesh.h"
#include "rndobj/Overlay.h"
#include "rndobj/TexRenderer.h"
#include "rndobj/Text.h"
#include "ui/UI.h"
#include "ui/UILabel.h"
#include "ui/UIPanel.h"
#include "ui/UIScreen.h"
#include "utl/Symbol.h"
#include "xdk/xapilibi/xbox.h"

namespace {
    UIPanel *FindPanel(const char *name) {
        UIPanel *p = ObjectDir::Main()->Find<UIPanel>(name);
        MILO_ASSERT(p->CheckIsLoaded(), 0x99);
        MILO_ASSERT(p->LoadedDir(), 0x9A);
        return p;
    }
}

HamUI::HamUI()
    : mHelpBar(nullptr), mLetterbox(nullptr), mBlacklight(nullptr),
      mEventDialogPanel(nullptr), mBackgroundPanel(nullptr),
      mContentLoadingPanel(nullptr), mOverlayPanel(nullptr), mGamePanel(nullptr),
      mAugmentedPhoto(nullptr), unk_0xFC(false), unk_0xFD(false), mEventScreen(nullptr),
      mShellInput(new ShellInput()), mPadNum(0), mBufferType(LiveCameraInput::kBufferOff),
      mSkelRot(0), mButtonSpam(false), mUIOverlay(nullptr) {
    SetFullScreenDraw(false);
}

HamUI::~HamUI() { RELEASE(mAugmentedPhoto); }

BEGIN_HANDLERS(HamUI)
    HANDLE_EXPR(display_next_camera_output, DisplayNextCameraOutput())
    HANDLE_EXPR(toggle_draw_skeletons, ToggleDrawSkeletons())
    HANDLE_EXPR(toggle_full_screen_draw, SetFullScreenDraw(!mFullScreenDrawActive))
    HANDLE_EXPR(next_skeleton_draw_rot, NextSkeletonDrawRot())
    HANDLE_EXPR(cycle_draw_cursor, mShellInput->CycleDrawCursor())
    HANDLE_ACTION(set_button_spam, mButtonSpam = _msg->Int(2))
    HANDLE_EXPR(button_spam, mButtonSpam)
    HANDLE_ACTION_IF(
        toggle_ui_overlay, mUIOverlay, mUIOverlay->SetShowing(!mUIOverlay->Showing())
    )
    HANDLE_ACTION_IF(toggle_letterbox, mLetterbox, mLetterbox->ToggleBlacklightMode(false))
    HANDLE_ACTION_IF(
        toggle_letterbox_immediately, mLetterbox, mLetterbox->ToggleBlacklightMode(true)
    )
    HANDLE_EXPR(
        is_letterbox_in_transition,
        mLetterbox ? mLetterbox->InBlacklightTransition() : false
    )
    HANDLE_EXPR(is_blacklight_mode, IsBlacklightMode())
    HANDLE_ACTION(force_letterbox_off, ForceLetterboxOff())
    HANDLE_ACTION(force_letterbox_off_immediate, ForceLetterboxOffImmediate())
    HANDLE_ACTION(reset_snapshots, ResetSnapshots())
    HANDLE_EXPR(take_snapshot, TakeSnapshot())
    HANDLE_EXPR(num_snapshots, NumSnapshots())
    HANDLE_ACTION(
        apply_snapshot_to_mesh, ApplySnapshotToMesh(_msg->Int(2), _msg->Obj<RndMesh>(3))
    )
    HANDLE_EXPR(get_augmented_photo, mAugmentedPhoto)
    HANDLE_EXPR(has_overlay_panel, mOverlayPanel != nullptr)
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
    HANDLE_ACTION(store_depth_buff_at, StoreDepthBufferAt(_msg->Int(2)))
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
    HANDLE_MESSAGE(KinectGuideGestureMsg)
    HANDLE_SUPERCLASS(UIManager)
    HANDLE_MEMBER_PTR(mShellInput)
    HANDLE_MEMBER_PTR(mHelpBar)
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
        HamScreen *screen = dynamic_cast<HamScreen *>(&*it);
        if (!screen) {
            if (!it->TypeDef() || !strstr(it->TypeDef()->File(), "system/run")) {
                MILO_NOTIFY("UIScreen %s is not a HamScreen", it->Name());
            }
        }
        if (helpBarPanel && it->HasPanel(helpBarPanel)) {
            MILO_NOTIFY("Screen %s directly includes helpbar", it->Name());
        }
    }
    static Symbol ui("ui");
    mUIOverlay = RndOverlay::Find(ui, false);
    const char *photo = nullptr;
    if (SystemConfig("ui")->FindData("augmented_photo", photo, false) && photo) {
        ObjectDir *dir = DirLoader::LoadObjects(photo, nullptr, nullptr);
        mAugmentedPhoto = dynamic_cast<RndDir *>(dir);
        if (!mAugmentedPhoto && dir) {
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
    if (TheGame) {
        TheGame->CheckPauseRequest();
    }
    UIManager::Poll();
    mShellInput->Poll();
    TheProfileMgr.Poll();
    if (mHelpBar) {
        mHelpBar->Poll();
        if (mLetterbox) {
            mLetterbox->Poll();
        }
        mContentLoadingPanel->Poll();
        mEventDialogPanel->Poll();
        if (mOverlayPanel) {
            mOverlayPanel->Poll();
        }
        if (mEventScreen) {
            AttemptEventTranstion();
        }
        if (mButtonSpam) {
            static Timer t;
            if (!t.Running()) {
                t.Restart();
            }
            if (t.SplitMs() > 500.0f) {
                Handle(ButtonDownMsg(nullptr, kPad_Xbox_A, kAction_Confirm, 0), true);
                t.Stop();
            }
        }
        UpdateUIOverlay();
    }
}

void HamUI::Draw() {
    if (mHelpBar) {
        RndText::ClearBlacklight();
        UIPanel::SetFinalDrawPass(false);
        bool transitioning = TheUI->InTransition();
        if (TheUIEventMgr->HasActiveDialogEvent() && !transitioning) {
            mBackgroundPanel->Draw();
            mEventDialogPanel->Draw();
            mHelpBar->Draw();
            if (mLetterbox) {
                mLetterbox->Draw();
            }
            if (RndText::IsBlacklightModeEnabled()) {
                RndText::DrawBlacklight();
                RndText::ClearBlacklight();
            }
            if (mBlacklight) {
                mBlacklight->Draw();
            }
            DrawDebug();
            mShellInput->Draw();
        } else {
            UIManager::Draw();
            if (mOverlayPanel) {
                RndText::ClearBlacklight();
                mOverlayPanel->Draw();
            }
            mHelpBar->Draw();
            if (mLetterbox) {
                mLetterbox->Draw();
            }
            UIPanel::SetFinalDrawPass(true);
            UIManager::Draw();
            UIPanel::SetFinalDrawPass(false);
            if (RndText::IsBlacklightModeEnabled()) {
                RndText::DrawBlacklight();
                RndText::ClearBlacklight();
            }
            if (mBlacklight) {
                mBlacklight->Draw();
            }
            if (transitioning && mContentLoadingPanel->Showing()) {
                mContentLoadingPanel->Draw();
            }
            DrawDebug();
            mShellInput->Draw();
            static bool sPollAugmentedPhoto = true;
            if (sPollAugmentedPhoto) {
                sPollAugmentedPhoto = false;
                mAugmentedPhoto->Poll();
                mAugmentedPhoto->Draw();
            }
            if (unk_0xFC || unk_0xFD) {
                LiveCameraInput *cam = TheGestureMgr->GetLiveCameraInput();
                if (mAugmentedPhoto && cam) {
                    if (unk_0xFC) {
                        cam->IncrementSnapshotCount();
                        unk_0xFC = false;
                        unk_0xFD = true;
                    } else {
                        unk_0xFD = false;
                    }
                    RndTexRenderer *renderer =
                        mAugmentedPhoto->Find<RndTexRenderer>("TexRenderer.rndtex");
                    RndMat *snapshot = cam->GetSnapshot(cam->NumSnapshots() - 1);
                    renderer->SetOutputTexture(
                        snapshot ? snapshot->GetDiffuseTex() : nullptr
                    );
                    StreamRenderer *streamRenderer =
                        mAugmentedPhoto->Find<StreamRenderer>("StreamRendererColor.sr");
                    ShellInput *pShellInput = TheHamUI.GetShellInput();
                    MILO_ASSERT(pShellInput, 300);
                    SkeletonChooser *pSkeletonChooser = pShellInput->GetSkeletonChooser();
                    MILO_ASSERT(pSkeletonChooser, 0x12E);
                    for (int i = 0; i < 6; i++) {
                        streamRenderer->SetCrewPhotoPlayerDetected(
                            i, pSkeletonChooser->IsSkeletonValid(i)
                        );
                    }
                    HamPlayerData *player0 = TheGameData->Player(0);
                    HamPlayerData *player1 = TheGameData->Player(1);
                    streamRenderer->SetPinkPlayer(
                        TheGestureMgr->GetSkeletonIndexByTrackingID(
                            player0->GetSkeletonTrackingID()
                        )
                        + 1
                    );
                    streamRenderer->SetBluePlayer(
                        TheGestureMgr->GetSkeletonIndexByTrackingID(
                            player1->GetSkeletonTrackingID()
                        )
                        + 1
                    );
                    RndMesh *purpleL =
                        mAugmentedPhoto->Find<RndMesh>("ribbons_purple_l.mesh");
                    RndMesh *purpleR =
                        mAugmentedPhoto->Find<RndMesh>("ribbons_purple_r.mesh");
                    RndMesh *blueL =
                        mAugmentedPhoto->Find<RndMesh>("ribbons_blue_l.mesh");
                    RndMesh *blueR =
                        mAugmentedPhoto->Find<RndMesh>("ribbons_blue_r.mesh");
                    if ((player0->IsPlaying() && player0->Side() == kSkeletonLeft)
                        || (player1->IsPlaying() && player1->Side() == kSkeletonRight)) {
                        purpleL->SetShowing(true);
                        purpleR->SetShowing(false);
                        blueL->SetShowing(false);
                        blueR->SetShowing(true);
                    } else {
                        purpleL->SetShowing(false);
                        purpleR->SetShowing(true);
                        blueL->SetShowing(true);
                        blueR->SetShowing(false);
                    }
                    mAugmentedPhoto->Poll();
                    mAugmentedPhoto->DrawShowing();
                }
            }
        }
    }
}

bool HamUI::IsTimelineResetAllowed() const {
    if (!ThePassiveMessenger->HasMessages()
        && !ThePassiveMessenger->HasRecentlyDismissedMessage()
        && TheSkeletonIdentifier->GetIDStatus() == 0 &&
        (!mHelpBar || (!mHelpBar->IsWriteIconShowing() && !mHelpBar->IsAnimating()))) {
        return true;
    }
    return false;
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

void HamUI::ReloadStrings() {
    Message reload_string("reload_string");
    UIPanel *panels[] = { mHelpBar,          mLetterbox,       mBlacklight,
                          mEventDialogPanel, mBackgroundPanel, mContentLoadingPanel,
                          mOverlayPanel,     mGamePanel };
    for (int i = 0; i < DIM(panels); i++) {
        UIPanel *cur = panels[i];
        if (cur) {
            ObjectDir *dataDir = cur->DataDir();
            if (dataDir) {
                for (ObjDirItr<UILabel> it(dataDir, true); it != nullptr; ++it) {
                    it->Handle(reload_string, true);
                }
            }
        }
    }

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
        mGamePanel = ObjectDir::Main()->Find<GamePanel>("game_panel");
        mLetterbox = dynamic_cast<LetterboxPanel *>(FindPanel("letterbox"));
        mBlacklight = dynamic_cast<BlacklightPanel *>(FindPanel("blacklight"));
    }
}

bool HamUI::IsGameActive() const {
    if (!mGamePanel) {
        return false;
    } else {
        return mShellInput->IsGameplayPanel() && mGamePanel->GetState() == UIPanel::kUp
            && !mGamePanel->Paused();
    }
}

void HamUI::DrawDebug() {
    if (mBufferType < 4 || NumSnapshots() <= 0) {
        LiveCameraInput::BufferType bt = mBufferType;
        if (bt < 0) {
            bt = (LiveCameraInput::BufferType)4;
        }
        DrawGestureMgr(*TheGestureMgr, bt, mSkelRot);
    } else {
        DrawSnapshot(*TheGestureMgr, mBufferType - 4);
    }
    mShellInput->DrawDebug();
    TheHamDirector->DrawDebug();
}

Symbol HamUI::DisplayNextCameraOutput() {
    do {
        do {
            mBufferType = (LiveCameraInput::BufferType)(mBufferType + 1);
        } while (mBufferType == 1);
    } while (mBufferType == 2);
    if (mBufferType >= NumSnapshots() + 4) {
        mBufferType = (LiveCameraInput::BufferType)-1;
    }
    switch (mBufferType) {
    case LiveCameraInput::kBufferColor:
        return "color";
    case LiveCameraInput::kBufferDepth:
        return "depth";
    case LiveCameraInput::kBufferPlayer:
        return "player";
    case LiveCameraInput::kBufferPlayerColor:
        return "player color";
    case LiveCameraInput::kBufferOff:
        return "off";
    default:
        return MakeString("snapshot %d", mBufferType - 4);
    }
}

void HamUI::UpdateUIOverlay() {
    if (mUIOverlay && mUIOverlay->Showing()) {
        int lines = 0;
        mUIOverlay->Clear();
        std::vector<UIScreen *> screens;
        if (PushDepth() > 0) {
            screens.push_back(BottomScreen());
        }
        if (mCurrentScreen) {
            screens.push_back(mCurrentScreen);
        }
        FOREACH (it, screens) {
            *mUIOverlay << "screen " << (*it)->Name() << "\n";
            UIPanel *focusPanel = (*it)->FocusPanel();
            for (auto panelIt = (*it)->PanelList().begin();
                 panelIt != (*it)->PanelList().end();
                 ++panelIt, ++lines) {
                UIPanel *panel = panelIt->mPanel;
                *mUIOverlay << "panel " << panel->IsLoaded()
                            << (focusPanel == panel ? "* " : "  ") << panel->Name()
                            << "\n";
            }
        }
        if (mTransitionScreen) {
            *mUIOverlay << "going to screen " << mTransitionScreen->Name() << "\n";
            UIPanel *focusPanel = mTransitionScreen->FocusPanel();
            for (auto panelIt = mTransitionScreen->PanelList().begin();
                 panelIt != mTransitionScreen->PanelList().end();
                 ++panelIt, ++lines) {
                UIPanel *panel = panelIt->mPanel;
                *mUIOverlay << "panel " << panel->IsLoaded()
                            << (focusPanel == panel ? "* " : "  ") << panel->Name()
                            << "\n";
            }
        }
        if (lines != 0) {
            mUIOverlay->SetLines(lines);
        }
    }
}

DataNode HamUI::OnMsg(const UITransitionCompleteMsg &msg) {
    HAQManager::Print(HAQType::kHAQType_Screen);
    HAQManager::Print(HAQType::kHAQType_Focus);
    Symbol s60(gNullStr);
    UIScreen *newScreen = msg.GetNewScreen();
    if (newScreen) {
        s60 = newScreen->Name();
        const DataNode *prop = newScreen->Property("disable_screen_saver", false);
        bool disable = prop && prop->Int();
        ThePlatformMgr.SetScreenSaver(!disable);
    }
    if (TheUIEventMgr->HasActiveTransitionEvent()
        && TheUIEventMgr->IsTransitionEventFinished()) {
        TheUIEventMgr->DismissEvent(gNullStr);
    }
    if (mHelpBar) {
        mHelpBar->HandleType(msg);
    }
    mShellInput->SyncToCurrentScreen();
    CurrentScreenChangedMsg screenChangeMsg(s60);
    Export(screenChangeMsg, true);
    return DATA_UNHANDLED;
}

DataNode HamUI::OnMsg(const DiskErrorMsg &) {
    static Symbol disc_error("disc_error");
    TheUIEventMgr->TriggerEvent(disc_error, 0);
    return 1;
}

DataNode HamUI::OnMsg(const KinectGuideGestureMsg &msg) {
    if (IsGameActive() && !mGamePanel->IsGameOver()) {
        return DATA_UNHANDLED;
    } else {
        XShowNuiGuideUI(msg.TrackingID());
        return 1;
    }
}

DataNode HamUI::OnMsg(const ContentReadFailureMsg &msg) {
    static Message init("init", 0, 0);
    init[0] = msg.GetBool();
    init[1] = msg.GetStr();
    static Symbol data_error("data_error");
    TheUIEventMgr->TriggerEvent(data_error, init);
    return 1;
}

DataNode HamUI::OnMsg(const ButtonDownMsg &msg) {
    static ResetControllerModeTimeoutMsg resetControllerModeTimeoutMsg;
    Export(resetControllerModeTimeoutMsg, true);
    mPadNum = msg.GetPadNum();
    return DATA_UNHANDLED;
}

bool ToggleDrawSkeletons() {
    MILO_ASSERT(TheSkeletonViz, 0xe2);
    TheSkeletonViz->SetShowing(TheSkeletonViz->GetForceSubpartSelection());
    return TheSkeletonViz->GetForceSubpartSelection();
}
