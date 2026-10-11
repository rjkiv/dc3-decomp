#include "meta_ham/VoiceControlPanel.h"
#include "HamPanel.h"
#include "HamSongMetadata.h"
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
#include "meta_ham/SongRecord.h"
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
#include "utl/Std.h"
#include "utl/Symbol.h"

VoiceControlPanel::VoiceControlPanel()
    : mEnterTime(0), mEntered(false), mPickRandomSongTimer(3.4028235e+38),
      mCycleTipTimer(0), mHasSelectedDifficulty(false), mHasSelectedMode(false),
      mMustTurnMusicBackOn(false), mDismissOnTransitionComplete(false), mNumShellCmds(0),
      mNumBidCmds(0) {}

VoiceControlPanel::~VoiceControlPanel() {}

BEGIN_HANDLERS(VoiceControlPanel)
    HANDLE_MESSAGE(SpeechRecoMessage)
    HANDLE_MESSAGE(UITransitionCompleteMsg)
    HANDLE_SUPERCLASS(OverlayPanel)
END_HANDLERS

void VoiceControlPanel::Poll() {
    HamPanel::Poll();
    static float sFloats[] = { 0.25f, 2.0f, 4.5f };
    if (mEntered && mEnterTime < sFloats[0]) {
        mEnterTime = Min(sFloats[0], mEnterTime + TheTaskMgr.DeltaUISeconds());
    }
    if (!mEntered && mEnterTime > 0) {
        mEnterTime = Max(0.0f, mEnterTime - TheTaskMgr.DeltaUISeconds());
    }
    if (mSong == gNullStr && mEntered) {
        float set = mPickRandomSongTimer + TheTaskMgr.DeltaUISeconds();
        float cmp = sFloats[1];
        mPickRandomSongTimer = set;
        if (mPickRandomSongTimer > cmp) {
            mPickRandomSongTimer = 0;
            Symbol song = TheHamSongMgr.GetRandomSong();
            DisplaySong(song);
        }
    }
    if (mEntered) {
        float set = mCycleTipTimer + TheTaskMgr.DeltaUISeconds();
        float cmp = sFloats[2];
        mCycleTipTimer = set;
        if (mCycleTipTimer > cmp) {
            CycleTip();
        }
    }
}

void VoiceControlPanel::Dismiss() {
    mEntered = false;
    Symbol lang = HongKongExceptionMet() ? "eng" : SystemLanguage();
    if (lang != "eng" && lang != "jpn") {
        TheSpeechMgr->DisableAndUnloadGrammars();
        DataArray *arr = SystemConfig("kinect")->FindArray("speech");
        TheSpeechMgr->EnableAndLoadGrammars(arr);
    }
    SetRules(true);
    if (mMustTurnMusicBackOn) {
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
    if (mSong == gNullStr)
        return false;
    else {
        static Symbol practice("practice");
        if (mMode == practice)
            return false;
        else
            return !TheProfileMgr.IsDifficultyUnlocked(mSong, DifficultyToSym(mDiff));
    }
}

bool VoiceControlPanel::ReadyToStart() const {
    return mSong != gNullStr && mDiff != kNumDifficulties && mMode != gNullStr
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
        TheGameMode->SetMode(mMode, "none");
        MetaPerformer::Current()->SetSong(mSong);
        TheGameData->Player(0)->SetDifficulty(mDiff);
        TheGameData->Player(1)->SetDifficulty(mDiff);
        static Symbol dance_battle("dance_battle");
        if (mMode == dance_battle) {
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
        if (mMode == practice) {
            MetaPerformer::Current()->SetSkipPracticeWelcome(true);
        }
        static Symbol byo_bid("byo_bid");
        TheHamProvider->SetProperty(byo_bid, false);
        Flow *pFlow = DataDir()->Find<Flow>("sound_dance.flow");
        pFlow->Activate();
        mDismissOnTransitionComplete = true;
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
    if (b1 != s8a0 || see0 != mEntered) {
        see0 = mEntered;
        bool oldRecognizing = TheSpeechMgr->Recognizing();
        s8a0 = b1;
        TheSpeechMgr->SetRecognizing(false);
        TheSpeechMgr->SetRule("voice_shell", "voice_control", !mEntered && b1);
        if (TheSpeechMgr->HasGrammar("play_song_grammar")) {
            TheSpeechMgr->SetRule("play_song_grammar", "play_song", mEntered && b1);
        }
        TheSpeechMgr->SetRule("voice_shell", "random_song", mEntered && b1);
        TheSpeechMgr->SetRule("voice_shell", "difficulty", mEntered && b1);
        TheSpeechMgr->SetRule("voice_shell", "mode", mEntered && b1);
        TheSpeechMgr->SetRule("voice_shell", "dance", mEntered && b1);
        TheSpeechMgr->SetRule("voice_shell", "back", mEntered && b1);
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

    const std::map<Symbol, SongRecord> &songSortMembers =
        TheSongSortMgr->GetSongRecords();
    FOREACH (it, songSortMembers) {
        const SongRecord &record = it->second;
        std::vector<String> pronunciations = record.GetData()->Pronunciations();
        const std::vector<PronunciationsLoc> &pronunciationsLoc =
            record.GetData()->PronunciationsLocalized();
        for (int i = 0; i < pronunciationsLoc.size(); i++) {
            if (pronunciationsLoc[i].mLanguage == GetSongTitlePronunciationLanguage()) {
                pronunciations = pronunciationsLoc[i].mPronunciations;
            }
        }

        for (int i = 0; i < pronunciations.size(); i++) {
            String pronunciation = pronunciations[i];
            TheSpeechMgr->AddDynamicRuleWord(
                "play_song_grammar",
                pronunciation.c_str(),
                record.ShortName().Str(),
                &v8c,
                nullptr
            );
        }
    }
    TheSpeechMgr->CommitGrammar("play_song_grammar");
    TheSpeechMgr->SetRecognizing(true);
}

void VoiceControlPanel::CycleTip() {
    mCycleTipTimer = 0;
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
        if (!mHasSelectedMode) {
            syms.push_back("voicecontrol_mode_instruction");
        }
        if (!mHasSelectedDifficulty) {
            syms.push_back("voicecontrol_difficulty_instruction");
        }
        if (!mHasSelectedMode || !mHasSelectedDifficulty) {
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
        mEntered = true;
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
        mMustTurnMusicBackOn = TheMetaMusic->IsStarted();
        TheMetaMusic->Stop();
        SongPreview *preview = ObjectDir::Main()->Find<SongPreview>("song_preview");
        preview->SetMusicVol(SongPreview::kSilenceVal);
        mSong = gNullStr;
        mDiff = DefaultDifficulty();
        mMode = "perform";
        mHasSelectedDifficulty = false;
        mHasSelectedMode = false;
        DataDir()->Find<RndDrawable>("song_dimmer.mesh")->SetShowing(true);
        DataDir()->Find<RndAnimatable>("selected_diff.anim")->SetFrame(0, 1);
        DataDir()->Find<RndAnimatable>("selected_mode.anim")->SetFrame(0, 1);
        CycleTip();
        WakeUpScreenSaver();
        mDismissOnTransitionComplete = false;
    }
}

DataNode VoiceControlPanel::OnMsg(const UITransitionCompleteMsg &msg) {
    if (mDismissOnTransitionComplete) {
        Dismiss();
        SetRules(false);
    }
    return DATA_UNHANDLED;
}

DataNode VoiceControlPanel::OnMsg(const SpeechRecoMessage &msg) {
    if (!TheProfileMgr.GetDisableVoiceCommander()) {
        float thresh = TheSpeechMgr->SpeechConfThresh();
        if (msg.Confidence() >= thresh) {
            if (mMode == "practice") {
                mNumBidCmds++;
            } else {
                mNumShellCmds++;
            }
            if (mEntered) {
                WakeUpScreenSaver();
            }
            DataArray *tags = msg.Tags();
            Symbol rulename = msg.RuleName();
            if (rulename == "voice_control" && !mEntered) {
                Symbol arrSym = tags->Sym(0);
                if (arrSym == "voice_control" && !TheHamUI.InTransition()
                    && !TheContentMgr.RefreshInProgress()) {
                    PopUp();
                } else if (arrSym == "xbox") {
                    static Message microphoneActivityMsg("microphone_activity");
                    TheHamProvider->Handle(microphoneActivityMsg, false);
                    static Message voice_commander_help_bump("voice_commander_help_bump");
                    TheHamProvider->Handle(voice_commander_help_bump, false);
                    static Message voice_commander_help("voice_commander_help");
                    TheHamProvider->Handle(voice_commander_help, false);
                }
                return 0;
            } else if (rulename == "back" && mEntered) {
                DataDir()->Find<Flow>("sound_back.flow")->Activate();
                Dismiss();
                return 0;
            } else if (rulename == "dance" && mEntered) {
                EnterGame();
                return 0;
            } else if (rulename == "play_song" && mEntered) {
                Symbol arrSym = tags->Sym(0);
                static Symbol random_song("random_song");
                if (arrSym == random_song) {
                    arrSym = TheHamSongMgr.GetRandomSong();
                }
                int songID = TheHamSongMgr.GetSongIDFromShortName(arrSym, false);
                if (songID != 0) {
                    mSong = tags->Sym(0);
                    TheHamSongMgr.Data(songID);
                    DisplaySong(mSong);
                    DataDir()->Find<RndDrawable>("song_dimmer.mesh")->SetShowing(false);
                    DataDir()->Find<Flow>("sound_selection.flow")->Activate();
                    DataDir()->Find<Flow>("song_selected.flow")->Activate();
                    static Message microphoneActivityMsg("microphone_activity");
                    TheHamProvider->Handle(microphoneActivityMsg, false);
                }
                CycleTip();
                return 0;
            } else if (rulename == "random_song" && mEntered) {
                Symbol song = TheHamSongMgr.GetRandomSong();
                int songID = TheHamSongMgr.GetSongIDFromShortName(song, false);
                if (songID != 0) {
                    mSong = song;
                    TheHamSongMgr.Data(songID);
                    DisplaySong(mSong);
                    DataDir()->Find<RndDrawable>("song_dimmer.mesh")->SetShowing(false);
                    DataDir()->Find<Flow>("sound_selection.flow")->Activate();
                    DataDir()->Find<Flow>("song_selected.flow")->Activate();
                    static Message microphoneActivityMsg("microphone_activity");
                    TheHamProvider->Handle(microphoneActivityMsg, false);
                }
                CycleTip();
                return 0;
            } else if (rulename == "mode" && mEntered && mSong != gNullStr) {
                mMode = tags->Sym(0);
                float frame = 0;
                if (mMode == "practice") {
                    frame = 1;
                }
                if (mMode == "dance_battle") {
                    frame = 2;
                }
                DataDir()->Find<RndAnimatable>("selected_mode.anim")->SetFrame(frame, 1);
                mHasSelectedMode = true;
                DataDir()->Find<Flow>("sound_selection.flow")->Activate();
                static Message microphoneActivityMsg("microphone_activity");
                TheHamProvider->Handle(microphoneActivityMsg, false);
                CycleTip();
                return 0;
            } else if (rulename == "difficulty" && mEntered && mSong != gNullStr) {
                Symbol arrSym = tags->Sym(0);
                if (arrSym == "beginner") {
                    mDiff = kDifficultyBeginner;
                } else if (arrSym == "easy") {
                    mDiff = kDifficultyEasy;
                } else if (arrSym == "medium") {
                    mDiff = kDifficultyMedium;
                } else if (arrSym == "hard") {
                    mDiff = kDifficultyExpert;
                } else {
                    MILO_ASSERT(false, 0x14D);
                }
                DataDir()->Find<RndAnimatable>("selected_diff.anim")->SetFrame(mDiff, 1);
                mHasSelectedDifficulty = true;
                DataDir()->Find<Flow>("sound_selection.flow")->Activate();
                static Message microphoneActivityMsg("microphone_activity");
                TheHamProvider->Handle(microphoneActivityMsg, false);
                CycleTip();
                return 0;
            } else {
                return DATA_UNHANDLED;
            }
        }
    }
    return DATA_UNHANDLED;
}
