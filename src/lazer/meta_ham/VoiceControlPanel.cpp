#include "meta_ham/VoiceControlPanel.h"
#include "HamPanel.h"
#include "MultiUserGesturePanel.h"
#include "ProfileMgr.h"
#include "flow/Flow.h"
#include "flow/FlowNode.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "gesture/SpeechMgr.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "meta/SongPreview.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/HamUI.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/MultiUserGesturePanel.h"
#include "meta_ham/OverlayPanel.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "synth/MetaMusic.h"
#include "ui/UIScreen.h"
#include "utl/Symbol.h"

VoiceControlPanel::VoiceControlPanel()
    : unk44(0), unk48(false), unk4c(3.4028235e+38), unk50(0), unk54(false), unk55(false),
      unk56(false), unk64(false), unk68(0), unk6c(0) {}

VoiceControlPanel::~VoiceControlPanel() {}

void VoiceControlPanel::Poll() { HamPanel::Poll(); }

bool VoiceControlPanel::DifficultyLocked() const {
    if (unk58 == gNullStr)
        return false;
    else {
        static Symbol practice("practice");
        if (unk60 == practice)
            return false;
        else
            return !TheProfileMgr.IsDifficultyUnlocked(
                unk58, DifficultyToSym(mDifficulty)
            );
    }
}

bool VoiceControlPanel::ReadyToStart() const {
    return unk58 != gNullStr && mDifficulty != kNumDifficulties && unk60 != gNullStr
        && !DifficultyLocked();
}

void VoiceControlPanel::WakeUpScreenSaver() const {
    bool b = ThePlatformMgr.ScreenSaver();
    ThePlatformMgr.SetScreenSaver(false);
    ThePlatformMgr.SetScreenSaver(b);
}

void VoiceControlPanel::EnterGame() {
    MILO_FAIL("VoiceControlPanel::EnterGame should never be called! (I hope)\n");
    if (ReadyToStart()) {
        TheGameMode->SetMode(unk60, "none");
        MetaPerformer::Current()->SetSong(unk58);
        TheGameData->Player(0)->SetDifficulty(mDifficulty);
        TheGameData->Player(1)->SetDifficulty(mDifficulty);
        static Symbol dance_battle("dance_battle");
        if (unk60 == dance_battle) {
            MetaPerformer::Current()->ClearCharacters();
            MultiUserGesturePanel *pMultiUserGesturePanel =
                ObjectDir::Main()->Find<MultiUserGesturePanel>("multiuser_panel");
            pMultiUserGesturePanel->UpdateProviderPlayerIndices();
            pMultiUserGesturePanel->UpdateProviders();
            pMultiUserGesturePanel->SetRandomCrew(0);
            pMultiUserGesturePanel->UpdateProviders();
            pMultiUserGesturePanel->SetRandomCrew(1);
        }
        static Symbol practice("practice");
        if (unk60 == practice) {
            MetaPerformer::Current()->SetSkipPracticeWelcome(true);
        }
        static Symbol byo_bid("byo_bid");
        TheHamProvider->SetProperty(byo_bid, false);
        Flow *pFlow = DataDir()->Find<Flow>("sound_dance.flow");
        pFlow->Activate();
        unk64 = true;
        SetRules(false);
        static Symbol enter_gameplay("enter_gameplay");
        static DataArrayPtr dataPtr(enter_gameplay);
        dataPtr->Execute();
        static Message microphone_activity("microphone_activity");
        TheHamProvider->Handle(microphone_activity, false);
    } else {
        Flow *pFlow = DataDir()->Find<Flow>("sound_error.flow");
        pFlow->Activate();
    }
}

void VoiceControlPanel::ContentMounted(const char *, const char *) {
    static Message refresh_album_art("refresh_album_art");
    Handle(refresh_album_art, true);
}

DataNode VoiceControlPanel::OnMsg(UITransitionCompleteMsg const &msg) {
    if (unk64) {
        Dismiss();
        SetRules(false);
    }
    return DataNode(6);
}

void VoiceControlPanel::Dismiss() {
    unk48 = false;
    Symbol lang;
    if (HongKongExceptionMet()) {
        lang = Symbol("eng");
    } else {
        lang = SystemLanguage();
    }
    if (lang == "eng" || lang != "jpn") {
        TheSpeechMgr->DisableAndUnloadGrammars();
        DataArray *arr = SystemConfig("kinect")->FindArray("speech");
        TheSpeechMgr->EnableAndLoadGrammars(arr);
    }
    SetRules(true);
    if (unk56) {
        TheMetaMusic->Start();
    }
    SongPreview *songPrev = ObjectDir::Main()->Find<SongPreview>("song_preview");
    songPrev->SetMusicVol(songPrev->PreviewDb());
    MILO_ASSERT(TheHamUI.GetOverlayPanel() == this, 0x98);
    TheHamUI.SetOverlayPanel(nullptr);
    OverlayPanel::Dismiss();
}

BEGIN_HANDLERS(VoiceControlPanel)
    HANDLE_MESSAGE(SpeechRecoMessage)
    HANDLE_MESSAGE(UITransitionCompleteMsg)
    HANDLE_SUPERCLASS(OverlayPanel)
END_HANDLERS
