#include "game/SongSequence.h"
#include "flow/PropertyEventProvider.h"
#include "game/Game.h"
#include "game/GameMode.h"
#include "game/GamePanel.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamMaster.h"
#include "hamobj/HamPlayerData.h"
#include "macros.h"
#include "math/Easing.h"
#include "meta_ham/CampaignPerformer.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/MetaPerformer.h"
#include "midi/MidiParserMgr.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "rndobj/Group.h"
#include "rndobj/PropAnim.h"
#include "ui/UILabel.h"
#include "ui/UIPanel.h"
#include "utl/Symbol.h"
#include "utl/TimeConversion.h"

SongSequence TheSongSequence;

SongSequence::SongSequence() {}
SongSequence::~SongSequence() {}

BEGIN_HANDLERS(SongSequence)
    HANDLE_ACTION(clear, Clear())
    HANDLE_ACTION(add, Add(_msg))
    HANDLE_EXPR(do_next, DoNext(false, _msg->Int(2)))
    HANDLE_EXPR(done, Done())
    HANDLE_ACTION(on_rhythm_battle_combo_full, DoNext(false, false))
    HANDLE_ACTION(load_next_song_audio, LoadNextSongAudio())
    HANDLE_EXPR(get_intro_cam_shot, GetIntroCamShot())
    HANDLE_EXPR(get_outro_cam_shot, GetOutroCamShot())
    HANDLE_EXPR(
        loop_start,
        mCurrentIndex > mEntries.size() ? 0
                                        : BeatToMs(mEntries[mCurrentIndex].unk18 * 4.0f)
    )
    HANDLE_EXPR(empty, mEntries.size() == 0)
    HANDLE_EXPR(current_index, mCurrentIndex)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void SongSequence::Init() {
    mFileCache = nullptr;
    SetName("songseq", ObjectDir::Main());
    Clear();
}

bool SongSequence::Done() const {
    int numEntries = mEntries.size();
    return numEntries == 0 || mCurrentIndex >= numEntries;
}

void SongSequence::Add(const DataArray *a) {
    static Symbol babygotback("babygotback");
    static Symbol perform("perform");
    static Symbol holla_back_config_default("holla_back_config_default");
    if (a) {
        Entry entry;
        int aSize = a->Size();
        entry.unk4 = aSize > 2 ? a->Sym(2) : babygotback;
        entry.unk0 = aSize > 3 ? a->Sym(3) : babygotback;
        entry.unk8 = aSize > 4 ? a->Sym(4) : perform;
        entry.unkc = aSize > 5 ? a->Float(5) : -1;
        entry.unk10 = aSize > 6 ? a->Float(6) : -1;
        entry.unk14 = aSize > 7 ? a->Sym(7) : holla_back_config_default;
        entry.unk18 = aSize > 8 ? a->Float(8) : -1;
        entry.unk1c = aSize > 9 ? a->Float(9) : -1;
        entry.unk20 = aSize > 10 ? a->Int(10) : false;
        entry.unk21 = aSize > 11 ? a->Int(11) : entry.unk20;
        entry.mIntroCamShot = aSize > 12 ? a->Sym(12) : "";
        entry.mOutroCamShot = aSize > 13 ? a->Sym(13) : "";
        entry.unk2c = aSize > 14 ? a->Sym(14) : "";
        entry.unk30 = aSize > 15 ? a->Sym(15) : "";
        entry.unk34 = entry.unk38 = 0;
        mEntries.push_back(entry);
    }
}

void SongSequence::Clear() {
    mCurrentIndex = -1;
    mEntries.clear();
    unk18 = 0;
    unk1c = 0;
    if (mFileCache) {
        RELEASE(mFileCache);
    }
}

Symbol SongSequence::GetIntroCamShot() const {
    if (Done()) {
        return "";
    } else {
        return mEntries[mCurrentIndex].mIntroCamShot;
    }
}

Symbol SongSequence::GetOutroCamShot() const {
    if (Done()) {
        return "";
    } else {
        return mEntries[mCurrentIndex].mOutroCamShot;
    }
}

void SongSequence::LoadNextSongAudio() {
    if (!Done() && mCurrentIndex <= mEntries.size() - 2) {
        Hmx::Object *game = ObjectDir::Main()->Find<Hmx::Object>("game");
        if (game) {
            game->Handle(Message("set_realtime", true), true);
        }
        Entry &curEntry = mEntries[mCurrentIndex + 1];
        TheGame->LoadNewSongAudio(curEntry.unk4);
        bool b3 = false;
        if (*curEntry.unk2c.Str() != '\0') {
            b3 = true;
            HamPlayerData *hpd = TheGameData->Player(0);
            hpd->SetOutfit("");
            hpd->SetCrew(curEntry.unk2c);
            if (curEntry.unk8 == Symbol("mind_control")) {
                hpd->SetOutfit("lima06");
            }
        }
        if (*curEntry.unk30.Str() != '\0') {
            b3 = true;
            HamPlayerData *hpd = TheGameData->Player(1);
            hpd->SetOutfit("");
            hpd->SetCrew(curEntry.unk30);
            if (curEntry.unk8 == Symbol("mind_control")) {
                hpd->SetOutfit("rasa06");
            }
        }
        if (b3) {
            TheHamDirector->LoadCrew(
                TheGameData->Player(0)->Crew(), TheGameData->Player(1)->Crew()
            );
        }
    }
}

bool SongSequence::DoNext(bool b1, bool b2) {
    static Symbol midi_player("midi_player");
    static Symbol holla_back_config("holla_back_config");
    static Symbol active("active");
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol perform("perform");
    static Symbol in_campaign_era_intro("in_campaign_era_intro");
    static Symbol holla_back("holla_back");
    static Symbol mind_control("mind_control");
    unk28 = false;
    if (mEntries.size() == 0)
        return true;
    bool isLoaded = TheGame->IsLoaded();
    if (!b1 && !isLoaded) {
        return mEntries.size() <= mCurrentIndex;
    }
    if (!b1) {
        float old = unk18;
        unk18 = TheTaskMgr.UISeconds();
        if (unk18 - old < 0.5f) {
            return mEntries.size() <= mCurrentIndex;
        }
    }
    if (!b2 && mCurrentIndex >= 0) {
        if (TheHamProvider->Property(holla_back_config)->Int()) {
            static Symbol num_stars("num_stars");
            const DataNode *prop = TheGamePanel->Property(num_stars, false);
            int stars;
            if (prop) {
                stars = prop->Float();
            } else {
                stars = 0;
            }
            mEntries[mCurrentIndex].unk38 = stars;
            HamPlayerData *p0 = TheGameData->Player(0);
            HamPlayerData *p1 = TheGameData->Player(1);
            static Symbol score("score");
            int p0Score = p0->Provider()->Property(score)->Int();
            int p1Score = p1->Provider()->Property(score)->Int();
            mEntries[mCurrentIndex].unk34 = p0Score + p1Score;
            CampaignPerformer *cp = static_cast<CampaignPerformer *>(MetaPerformer::Current());
            Entry &entry = mEntries[mCurrentIndex];
            cp->UpdateEraSong(cp->GetDifficulty(), entry.unk4, entry.unk4, stars);
            cp->TriggerSongCompletion(entry.unk34, (float)entry.unk38);
        }
    }
    if (!b2 && mEntries[mCurrentIndex].unk8 == mind_control) {
        CampaignPerformer *cp = static_cast<CampaignPerformer *>(MetaPerformer::Current());
        cp->SetCampaignMindControlComplete(true);
    }
    mCurrentIndex++;
    if (mCurrentIndex >= mEntries.size()) {
        goto end_sequence;
    }
    if (!b2) {
        Entry &nextEntry = mEntries[mCurrentIndex];
        bool loadCrew = false;
        if (*nextEntry.unk2c.Str() != '\0') {
            loadCrew = true;
            HamPlayerData *hpd = TheGameData->Player(0);
            hpd->SetOutfit(Symbol(""));
            hpd->SetCrew(nextEntry.unk2c);
            if (nextEntry.unk8 == mind_control) {
                hpd->SetOutfit(Symbol("lima06"));
            }
        }
        if (*nextEntry.unk30.Str() != '\0') {
            loadCrew = true;
            HamPlayerData *hpd = TheGameData->Player(1);
            hpd->SetOutfit(Symbol(""));
            hpd->SetCrew(nextEntry.unk30);
            if (nextEntry.unk8 == mind_control) {
                hpd->SetOutfit(Symbol("rasa06"));
            }
        }
        if (loadCrew && isLoaded) {
            TheHamDirector->LoadCrew(
                TheGameData->Player(0)->Crew(), TheGameData->Player(1)->Crew()
            );
        }
        static Symbol hud_panel("hud_panel");
        static Symbol clear_flash_cards("clear_flash_cards");
        static Symbol clear_all_flashcard_campaign_states(
            "clear_all_flashcard_campaign_states"
        );
        TheMidiParserMgr->GetParser(midi_player)->SetProperty(active, nextEntry.unk14);
        TheHamProvider->SetProperty(holla_back_config, nextEntry.unk14);
        if (isLoaded) {
            ObjectDir *hudPanel = DataVariable(hud_panel).Obj<ObjectDir>();
            if (hudPanel) {
                hudPanel->Handle(Message(clear_flash_cards, 0), true);
                hudPanel->Handle(Message(clear_flash_cards, 1), true);
                hudPanel->Handle(Message(clear_all_flashcard_campaign_states), true);
            }
        }
        if (nextEntry.unk8 == "holla_back") {
            TheHamDirector->StartStopVisualizer(true, 0);
        }
        TheGameMode->SetGameplayMode(nextEntry.unk8, nextEntry.unk8 == perform);
        TheGame->LoadNewSong(nextEntry.unk4, nextEntry.unk0);
        unk24 = TheTaskMgr.UISeconds();
        if (isLoaded) {
            static Symbol deinit("deinit");
            UIPanel *gamePanel = ObjectDir::Main()->Find<UIPanel>("game_panel", true);
            gamePanel->Handle(Message(deinit), true);
        }
        if (nextEntry.unk8 == "holla_back") {
            static Symbol hide_venue("hide_venue");
            TheHamProvider->SetProperty(hide_venue, true);
        }
        return false;
    }
end_sequence:
    MILO_LOG("SongSequence::DoNext: terminating (skipped=%s)\n", b2 ? "T" : "F");
    Symbol mode = TheGameMode->Property(gameplay_mode)->Sym();
    if (mode == holla_back) {
        RndGroup *grp = TheHamDirector->GetVenueWorld()->Find<RndGroup>("bid.grp", true);
        if (grp) {
            grp->SetShowing(false);
        }
        RndPropAnim *anim =
            TheHamDirector->GetVenueWorld()->Find<RndPropAnim>("set_performance.anim", true);
        if (anim) {
            anim->Animate(0, false, 0, nullptr, kEaseLinear, 0, false);
        }
    }
    TheGameMode->SetGameplayMode(perform, true);
    Clear();
    mCurrentIndex = -1;
    return true;
}

void SongSequence::OnSongLoaded() {
    if (!Done()) {
        static Symbol reset("reset");
        static Symbol init("init");
        static Symbol set_type("set_type");
        static Symbol start_song_now("start_song_now");
        static Symbol show_hud("show_hud");
        static Symbol hide_hud("hide_hud");
        static Symbol start_score_move_index("start_score_move_index");
        static Symbol gameplay_mode("gameplay_mode");
        static Symbol freestyle_enabled("freestyle_enabled");
        static Symbol holla_back("holla_back");
        static Symbol mind_control("mind_control");
        MILO_LOG("Time to advance song sequence = %.3f\n", TheTaskMgr.UISeconds() - unk24);
        Entry &curEntry = mEntries[mCurrentIndex];
        UIPanel *gamePanel = ObjectDir::Main()->Find<UIPanel>("game_panel");
        bool hasIntro = TheGame->HasIntro();
        Symbol gameMode = TheGameMode->Property(gameplay_mode)->Sym();
        bool inHollaback = gameMode == holla_back;
        bool inMindControl = gameMode == mind_control;
        TheHamDirector->SetProperty("disabled", false);
        gamePanel->Handle(Message(set_type, curEntry.unk8), true);
        gamePanel->Handle(Message(init), true);
        for (int i = 0; i < 2; i++) {
            HamPlayerData *hpd = TheGameData->Player(i);
            if (hpd->IsPlaying()) {
                hpd->Provider()->SetProperty(start_score_move_index, -1);
            } else {
                hpd->Provider()->SetProperty(start_score_move_index, 1000);
            }
        }
        if (mCurrentIndex != 0 || !inHollaback) {
            gamePanel->Handle(Message(reset), true);
        }
        if (!inMindControl) {
            TheHamProvider->SetProperty("game_stage", Symbol("intro"));
        }
        if (!hasIntro) {
            gamePanel->Handle(Message(start_song_now), true);
            TheMaster->GetAudio()->ClearLoop();
        }
        TheHamDirector->SetProperty(freestyle_enabled, false);
        if (curEntry.unkc >= 0 && curEntry.unk10 >= 0) {
            TheMaster->GetAudio()->SetLoop(
                BeatToMs(curEntry.unk10 * 4.0f), BeatToMs(curEntry.unkc * 4.0f)
            );
        }
        if (curEntry.unk18 >= 0 && curEntry.unk1c >= 1) {
            TheMaster->GetAudio()->SetLoop(curEntry.unk18 * 4.0f, curEntry.unk1c * 4.0f);
            TheGame->Jump(curEntry.unk18 * 4.0f, true);
        }
        if (curEntry.unk20) {
            ObjectDir *hudPanel = DataVariable("hud_panel").Obj<ObjectDir>();
            hudPanel->Find<UILabel>("song_name.lbl")->SetPrelocalizedString(String(""));
            hudPanel->Find<UILabel>("song_artist.lbl")->SetPrelocalizedString(String(""));
        }
        if (mFileCache) {
            mFileCache->Clear();
            if (mCurrentIndex < mEntries.size() - 1) {
                char buffer[256];
                mFileCache->StartSet(0);
                Symbol s0 = mEntries[mCurrentIndex + 1].unk0;
                int s0len = strlen(s0.Str());
                strcpy(buffer, TheHamSongMgr.SongPath(s0, 0));
                buffer[strlen(buffer) + s0len] = 0;
                const char *milo = MakeString("%s%s.milo", buffer, s0.Str());
                const char *moves = MakeString("%s%s.milo", buffer, "moves");
                const char *clips = MakeString("%s%s.milo", buffer, "clips");
                const char *mogg = MakeString("%s%s.milo", buffer, s0);
                mFileCache->Add(milo, 1, milo);
                mFileCache->Add(moves, 1, moves);
                mFileCache->Add(clips, 1, clips);
                mFileCache->Add(mogg, 1, mogg);
                mFileCache->EndSet();
            }
        }
        unk1c = 0;
        if (!curEntry.unk21) {
            TheHamDirector->StartStopVisualizer(false, 1);
            RndGroup *grp = TheHamDirector->GetVenueWorld()->Find<RndGroup>("bid.grp");
            if (grp) {
                grp->SetShowing(false);
            }
        } else {
            TheHamDirector->StartStopVisualizer(false, 2);
        }
        if (inHollaback) {
            RndGroup *grp = TheHamDirector->GetVenueWorld()->Find<RndGroup>("bid.grp");
            if (grp) {
                grp->SetShowing(true);
            }
            RndPropAnim *anim =
                TheHamDirector->GetVenueWorld()->Find<RndPropAnim>("set_bid.anim");
            if (anim) {
                anim->Animate(0, false, 0, nullptr, kEaseLinear, 0, false);
            }
        }
        if (hasIntro) {
            TheGame->Handle(Message("set_realtime", hasIntro), true);
        }
        if (!inHollaback) {
            if (inMindControl)
                goto next;
            static Message songseq_intro("songseq_intro");
            TheHamProvider->Export(songseq_intro, true);
        }
        if (!inMindControl)
            return;
    next:
        TheGame->Handle(Message("set_realtime", false), true);
        TheHamProvider->SetProperty("game_stage", Symbol("playing"));
        unk28 = true;
        TheGame->Jump(0, true);
        TheHamDirector->Handle(Message("set_suppress_next_shot", 0x78), true);
    }
}
