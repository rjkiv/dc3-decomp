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
#include "hamobj/HamLabel.h"
#include "hamobj/HamPlayerData.h"
#include "math/Rand.h"
#include "math/Utl.h"
#include "meta/SongPreview.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/HamUI.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/MultiUserGesturePanel.h"
#include "meta_ham/OverlayPanel.h"
#include "meta_ham/SongSortMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/ContentMgr.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "synth/MetaMusic.h"
#include "ui/UI.h"
#include "ui/UIColor.h"
#include "ui/UIScreen.h"
#include "utl/Locale.h"
#include "utl/Symbol.h"

VoiceControlPanel::VoiceControlPanel()
    : unk44(0), unk48(false), unk4c(3.4028235e+38), unk50(0), unk54(false), unk55(false),
      unk56(false), unk64(false), unk68(0), unk6c(0) {}

VoiceControlPanel::~VoiceControlPanel() {}

BEGIN_HANDLERS(VoiceControlPanel)
    HANDLE_MESSAGE(SpeechRecoMessage)
    HANDLE_MESSAGE(UITransitionCompleteMsg)
    HANDLE_SUPERCLASS(OverlayPanel)
END_HANDLERS

void VoiceControlPanel::Poll() {
    HamPanel::Poll();
    static float sFloats[] = { 0.25f, 2.0f, 4.5f };
    if (unk48 && unk44 < sFloats[0]) {
        unk44 = Min(sFloats[0], unk44 + TheTaskMgr.DeltaUISeconds());
    }
    if (!unk48 && unk44 > 0) {
        unk44 = Max(0.0f, unk44 - TheTaskMgr.DeltaUISeconds());
    }
    if (unk58 == gNullStr && unk48) {
        float set = unk4c + TheTaskMgr.DeltaUISeconds();
        float cmp = sFloats[1];
        unk4c = set;
        if (unk4c > cmp) {
            unk4c = 0;
            Symbol song = TheHamSongMgr.GetRandomSong();
            DisplaySong(song);
        }
    }
    if (unk48) {
        float set = unk50 + TheTaskMgr.DeltaUISeconds();
        float cmp = sFloats[2];
        unk50 = set;
        if (unk50 > cmp) {
            CycleTip();
        }
    }
}

void VoiceControlPanel::Dismiss() {
    unk48 = false;
    Symbol lang = HongKongExceptionMet() ? "eng" : SystemLanguage();
    if (lang != "eng" && lang != "jpn") {
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

void VoiceControlPanel::ContentMounted(const char *, const char *) {
    static Message refreshAlbumArtMsg("refresh_album_art");
    Handle(refreshAlbumArtMsg, true);
}

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

void VoiceControlPanel::SetRules(bool b1) {
    static bool s8a0 = false;
    static bool see0 = true;
    if (b1 != s8a0 || see0 != unk48) {
        see0 = unk48;
        bool oldRecognizing = TheSpeechMgr->Recognizing();
        s8a0 = b1;
        TheSpeechMgr->SetRecognizing(false);
        TheSpeechMgr->SetRule("voice_shell", "voice_control", !unk48 && b1);
        if (TheSpeechMgr->HasGrammar("play_song_grammar")) {
            TheSpeechMgr->SetRule("play_song_grammar", "play_song", unk48 && b1);
        }
        TheSpeechMgr->SetRule("voice_shell", "random_song", unk48 && b1);
        TheSpeechMgr->SetRule("voice_shell", "difficulty", unk48 && b1);
        TheSpeechMgr->SetRule("voice_shell", "mode", unk48 && b1);
        TheSpeechMgr->SetRule("voice_shell", "dance", unk48 && b1);
        TheSpeechMgr->SetRule("voice_shell", "back", unk48 && b1);
        TheSpeechMgr->SetRecognizing(oldRecognizing);
    }
}

void VoiceControlPanel::DisplaySong(Symbol song) {
    const char *content = TheHamSongMgr.ContentName(song);
    if (content) {
        TheContentMgr.MountContent(content);
    }
    static Message setSongMsg("set_song", 0);
    setSongMsg[0] = song;
    Handle(setSongMsg, true);
}

void VoiceControlPanel::CreatePlaySongGrammar() const {
    TheSpeechMgr->SetRecognizing(false);
    if (TheSpeechMgr->HasGrammar("play_song_grammar")) {
        TheSpeechMgr->UnloadGrammar("play_song_grammar");
    }
    TheSpeechMgr->CreateGrammar("play_song_grammar");
    void *v90;
    TheSpeechMgr->AddDynamicRule("play_song_grammar", "play_song", &v90);
    static Symbol voice_command_song("voice_command_song");
    void *v8c;
    TheSpeechMgr->AddDynamicRuleWord(
        "play_song_grammar",
        Localize(voice_command_song, nullptr, TheLocale),
        gNullStr,
        &v90,
        &v8c
    );
    if (TheUI->FocusPanel() != ObjectDir::Main()->Find<UIPanel>("song_select_panel")) {
        TheSongSortMgr->OnEnter();
    }
    // SongSortMgr member iteration
    TheSpeechMgr->CommitGrammar("play_song_grammar");
    TheSpeechMgr->SetRecognizing(true);
}

void VoiceControlPanel::CycleTip() {
    unk50 = 0;
    HamLabel *lbl = DataDir()->Find<HamLabel>("instructions.lbl");
    lbl->LStyle(0).mColorOverride = DataDir()->Find<UIColor>("instructions.color");
    std::vector<Symbol> syms;
    if (DifficultyLocked()) {
        lbl->LStyle(0).mColorOverride = DataDir()->Find<UIColor>("red.color");
        syms.push_back("voicecontrol_difficulty_locked");
    } else if (!ReadyToStart()) {
        syms.push_back("voicecontrol_song_instruction");

    } else if (ReadyToStart()) {
        syms.push_back("voicecontrol_dance_instruction");
        if (!unk55) {
            syms.push_back("voicecontrol_mode_instruction");
        }
        if (!unk54) {
            syms.push_back("voicecontrol_difficulty_instruction");
        }
        if (!unk55 || !unk54) {
            syms.push_back("voicecontrol_random_song_instruction");
        }
    }
    for (int i = 0; i < 5; i++) {
        int randIdx = RandomInt(0, syms.size());
        if (lbl->TextToken() != syms[randIdx]) {
            lbl->SetTextToken(syms[randIdx]);
            break;
        }
    }
}

void VoiceControlPanel::PopUp() {
    if (!TheHamUI.GetOverlayPanel()) {
        unk48 = true;
        TheHamUI.SetOverlayPanel(this);
        Symbol lang = HongKongExceptionMet() ? "eng" : SystemLanguage();
        if (lang != "eng" && lang != "jpn") {
            TheSpeechMgr->DisableAndUnloadGrammars();
            TheSpeechMgr->Enable(true);
            SystemConfig("kinect")->FindArray("speech")->FindArray("grammars");
            String fullDir = String("grammar/") + TheSpeechMgr->GetSpeechLanguageDir()
                + "/" + "voice_shell_en-US.cfgp";
            TheSpeechMgr->LoadGrammar("voice_shell", fullDir.c_str(), true);
        }
        CreatePlaySongGrammar();
        SetRules(true);
        unk56 = TheMetaMusic->IsStarted();
        TheMetaMusic->Stop();
        SongPreview *preview = ObjectDir::Main()->Find<SongPreview>("song_preview");
        preview->SetMusicVol(SongPreview::kSilenceVal);
        unk58 = gNullStr;
        mDifficulty = DefaultDifficulty();
        unk60 = "perform";
        unk54 = false;
        unk55 = false;
        DataDir()->Find<RndDrawable>("song_dimmer.mesh")->SetShowing(true);
        DataDir()->Find<RndAnimatable>("selected_diff.anim")->SetFrame(0, 1);
        DataDir()->Find<RndAnimatable>("selected_mode.anim")->SetFrame(0, 1);
        CycleTip();
        WakeUpScreenSaver();
        unk64 = false;
    }
}

DataNode VoiceControlPanel::OnMsg(const UITransitionCompleteMsg &msg) {
    if (unk64) {
        Dismiss();
        SetRules(false);
    }
    return DATA_UNHANDLED;
}
