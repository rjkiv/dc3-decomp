#include "game/SongSequence.h"
#include "flow/PropertyEventProvider.h"
#include "game/Game.h"
#include "game/GameMode.h"
#include "game/GamePanel.h"
#include "hamobj/Ham.h"
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
        m_Current > m_Entries.size() ? 0
                                     : BeatToMs(m_Entries[m_Current].loop_start * 4.0f)
    )
    HANDLE_EXPR(empty, m_Entries.size() == 0)
    HANDLE_EXPR(current_index, m_Current)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void SongSequence::Init() {
    sCache = nullptr;
    SetName("songseq", ObjectDir::Main());
    Clear();
}

bool SongSequence::Done() const {
    int num_Entries = m_Entries.size();
    return num_Entries == 0 || m_Current >= num_Entries;
}

void SongSequence::Add(const DataArray *a) {
    static Symbol babygotback("babygotback");
    static Symbol perform("perform");
    static Symbol holla_back_config_default("holla_back_config_default");
    if (a) {
        Entry entry;
        int aSize = a->Size();
        entry.song_audio = aSize > 2 ? a->Sym(2) : babygotback;
        entry.song_moves = aSize > 3 ? a->Sym(3) : babygotback;
        entry.mode = aSize > 4 ? a->Sym(4) : perform;
        entry.jump_start = aSize > 5 ? a->Float(5) : -1;
        entry.jump_end = aSize > 6 ? a->Float(6) : -1;
        entry.config = aSize > 7 ? a->Sym(7) : holla_back_config_default;
        entry.loop_start = aSize > 8 ? a->Float(8) : -1;
        entry.loop_end = aSize > 9 ? a->Float(9) : -1;
        entry.decoding = aSize > 10 ? a->Int(10) : false;
        entry.timey_wimey = aSize > 11 ? a->Int(11) : entry.decoding;
        entry.intro_cam_shot = aSize > 12 ? a->Sym(12) : "";
        entry.outro_cam_shot = aSize > 13 ? a->Sym(13) : "";
        entry.crew_0_override = aSize > 14 ? a->Sym(14) : "";
        entry.crew_1_override = aSize > 15 ? a->Sym(15) : "";
        entry.total_score = entry.stars = 0;
        m_Entries.push_back(entry);
    }
}

void SongSequence::Clear() {
    m_Current = -1;
    m_Entries.clear();
    m_PrevChangeTime = 0;
    m_TimeSinceLoad = 0;
    if (sCache) {
        RELEASE(sCache);
    }
}

Symbol SongSequence::GetIntroCamShot() const {
    if (Done()) {
        return "";
    } else {
        return m_Entries[m_Current].intro_cam_shot;
    }
}

Symbol SongSequence::GetOutroCamShot() const {
    if (Done()) {
        return "";
    } else {
        return m_Entries[m_Current].outro_cam_shot;
    }
}

void SongSequence::LoadNextSongAudio() {
    if (!Done() && m_Current <= m_Entries.size() - 2) {
        Hmx::Object *game = ObjectDir::Main()->Find<Hmx::Object>("game");
        if (game) {
            game->Handle(Message("set_realtime", true), true);
        }
        Entry &curEntry = m_Entries[m_Current + 1];
        TheGame->LoadNewSongAudio(curEntry.song_audio);
        bool b3 = false;
        if (*curEntry.crew_0_override.Str() != '\0') {
            b3 = true;
            HamPlayerData *hpd = TheGameData->Player(0);
            hpd->SetOutfit("");
            hpd->SetCrew(curEntry.crew_0_override);
            if (curEntry.mode == Symbol("mind_control")) {
                hpd->SetOutfit("lima06");
            }
        }
        if (*curEntry.crew_1_override.Str() != '\0') {
            b3 = true;
            HamPlayerData *hpd = TheGameData->Player(1);
            hpd->SetOutfit("");
            hpd->SetCrew(curEntry.crew_1_override);
            if (curEntry.mode == Symbol("mind_control")) {
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

bool SongSequence::DoNext(bool even_if_worlds_not_loaded, bool forced) {
    static Symbol midi_player("midi_player");
    static Symbol holla_back_config("holla_back_config");
    static Symbol active("active");
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol perform("perform");
    static Symbol in_campaign_era_intro("in_campaign_era_intro");
    static Symbol holla_back("holla_back");
    static Symbol mind_control("mind_control");
    m_HaveReenteredCharacters = false;
    int num_Entries = m_Entries.size();
    if (num_Entries == 0)
        return true;
    bool isLoaded = TheGame->IsLoaded();
    if (!even_if_worlds_not_loaded && !isLoaded) {
        return num_Entries <= m_Current;
    }
    if (!even_if_worlds_not_loaded) {
        float ui = TheTaskMgr.UISeconds();
        float old = m_PrevChangeTime;
        m_PrevChangeTime = ui;
        if (m_PrevChangeTime - old < 0.5f) {
            return num_Entries <= m_Current;
        }
    }
    if (!forced && m_Current >= 0) {
        if (TheHamProvider->Property(in_campaign_era_intro)->Int()) {
            static Symbol num_stars("num_stars");
            const DataNode *prop = TheGamePanel->Property(num_stars, false);
            int stars;
            if (prop) {
                stars = prop->Float();
            } else {
                stars = 0;
            }
            m_Entries[m_Current].stars = stars;
            PropertyEventProvider *p0 = TheGameData->Player(0)->Provider();
            PropertyEventProvider *p1 = TheGameData->Player(1)->Provider();
            m_Entries[m_Current].total_score =
                p0->Property("score")->Int() + p1->Property("score")->Int();
            CampaignPerformer *campaignPerf =
                static_cast<CampaignPerformer *>(MetaPerformer::Current());
            campaignPerf->UpdateEraSong(
                campaignPerf->GetDifficulty(),
                campaignPerf->Era(),
                m_Entries[0].song_audio,
                m_Entries[0].stars
            );
            campaignPerf->TriggerSongCompletion(
                m_Entries[0].total_score, m_Entries[0].stars
            );
        }
    }
    if (!forced && m_Entries[m_Current].mode == mind_control) {
        CampaignPerformer *campaignPerf =
            static_cast<CampaignPerformer *>(MetaPerformer::Current());
        campaignPerf->SetCampaignMindControlComplete(true);
    }

    if (++m_Current >= (int)m_Entries.size() || forced) {
        MILO_LOG("SongSequence::DoNext: terminating. forced=%s\n", forced ? "T" : "F");
        static Symbol holla_back("holla_back");
        Symbol mode = TheGameMode->Property(gameplay_mode)->Sym();
        if (mode == holla_back) {
            RndGroup *grp = TheHamDirector->GetVenueWorld()->Find<RndGroup>("bid.grp");
            if (grp) {
                grp->SetShowing(false);
            }
            RndPropAnim *anim = TheHamDirector->GetVenueWorld()->Find<RndPropAnim>(
                "set_performance.anim", true
            );
            if (anim) {
                anim->Animate(0, false, 0);
            }
        }
        TheGameMode->SetGameplayMode(perform, true);
        Clear();
        m_Current = -1;
        return true;
    } else {
        Entry &nextEntry = m_Entries[m_Current];
        bool loadCrew = false;
        if (*nextEntry.crew_0_override.Str() != '\0') {
            loadCrew = true;
            HamPlayerData *hpd = TheGameData->Player(0);
            hpd->SetOutfit("");
            hpd->SetCrew(nextEntry.crew_0_override);
            if (nextEntry.mode == Symbol("mind_control")) {
                hpd->SetOutfit("lima06");
            }
        }
        if (*nextEntry.crew_1_override.Str() != '\0') {
            loadCrew = true;
            HamPlayerData *hpd = TheGameData->Player(1);
            hpd->SetOutfit("");
            hpd->SetCrew(nextEntry.crew_1_override);
            if (nextEntry.mode == Symbol("mind_control")) {
                hpd->SetOutfit("rasa06");
            }
        }
        if (loadCrew && isLoaded) {
            TheHamDirector->LoadCrew(
                TheGameData->Player(0)->Crew(), TheGameData->Player(1)->Crew()
            );
        }
        static Symbol hud_panel("hud_panel");
        static Symbol clear_flash_cards("clear_flash_cards");
        static Symbol clear_all_flashcard_campaign_status(
            "clear_all_flashcard_campaign_status"
        );
        TheMidiParserMgr->GetParser(midi_player)->SetProperty(active, 0);
        TheHamProvider->SetProperty(holla_back_config, nextEntry.config);
        if (isLoaded) {
            ObjectDir *hudPanel = DataVariable(hud_panel).Obj<ObjectDir>();
            if (hudPanel) {
                hudPanel->Handle(Message(clear_flash_cards, 0), true);
                hudPanel->Handle(Message(clear_flash_cards, 1), true);
                hudPanel->Handle(Message(clear_all_flashcard_campaign_status), true);
            }
        }
        if (nextEntry.mode == "holla_back") {
            TheHamDirector->StartStopVisualizer(true, 0);
        }
        TheGameMode->SetGameplayMode(nextEntry.mode, nextEntry.mode == perform);
        TheGame->LoadNewSong(nextEntry.song_audio, nextEntry.song_moves);
        m_BeginLoadTime = TheTaskMgr.UISeconds();
        static Symbol deinit("deinit");
        UIPanel *gamePanel = ObjectDir::Main()->Find<UIPanel>("game_panel");
        gamePanel->Handle(Message(deinit), true);
        if (nextEntry.mode == "holla_back") {
            TheHamProvider->SetProperty("hide_venue", true);
        }
        return false;
    }
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
        MILO_LOG(
            "Time to advance song sequence = %.3f\n",
            TheTaskMgr.UISeconds() - m_BeginLoadTime
        );
        Entry &curEntry = m_Entries[m_Current];
        UIPanel *gamePanel = ObjectDir::Main()->Find<UIPanel>("game_panel");
        int hasIntro = TheGame->HasIntro();
        Symbol gameMode = TheGameMode->Property(gameplay_mode)->Sym();
        bool inHollaback = gameMode == holla_back;
        bool inMindControl = gameMode == mind_control;
        TheHamDirector->SetProperty("disabled", false);
        gamePanel->Handle(Message(set_type, curEntry.mode), true);
        gamePanel->Handle(Message(init), true);
        for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
            HamPlayerData *hpd = TheGameData->Player(i);
            if (hpd->IsPlaying()) {
                hpd->Provider()->SetProperty(start_score_move_index, -1);
            } else {
                hpd->Provider()->SetProperty(start_score_move_index, 1000);
            }
        }
        if (m_Current != 0 || !inHollaback) {
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
        if (curEntry.jump_start >= 0 && curEntry.jump_end >= 0) {
            float startBeat = curEntry.jump_start * 4.0f;
            float endBeat = curEntry.jump_end * 4.0f;
            BeatToMs(startBeat);
            BeatToMs(endBeat);
            TheMaster->GetAudio()->SetLoop(endBeat, startBeat);
        }
        if (curEntry.loop_start >= 0 && curEntry.loop_end >= 1) {
            TheMaster->GetAudio()->SetLoop(
                curEntry.loop_start * 4.0f, curEntry.loop_end * 4.0f
            );
            TheGame->Jump(BeatToMs(curEntry.loop_start * 4.0f), true);
        }
        if (curEntry.decoding) {
            ObjectDir *hudPanel = DataVariable("hud_panel").Obj<ObjectDir>();
            hudPanel->Find<UILabel>("song_name.lbl")->SetPrelocalizedString(String(""));
            hudPanel->Find<UILabel>("song_artist.lbl")->SetPrelocalizedString(String(""));
        }
        if (sCache) {
            sCache->Clear();
            if (m_Current < m_Entries.size() - 1) {
                char song_path[256];
                sCache->StartSet(0);
                Symbol song_shortname = m_Entries[m_Current + 1].song_moves;
                const char *song_str = song_shortname.Str();
                int s0len = strlen(song_str);
                strcpy(song_path, TheHamSongMgr.SongPath(song_shortname, 0));
                song_path[strlen(song_path) - s0len] = 0;
                const char *milo = MakeString("%s%s.milo", song_path, song_str);
                const char *moves = MakeString("%s%s.milo", song_path, "moves");
                const char *clips = MakeString("%s%s.milo", song_path, "clips");
                const char *mogg = MakeString("%s%s.mogg", song_path, song_str);
                sCache->Add(milo, 1, milo);
                sCache->Add(moves, 1, moves);
                sCache->Add(clips, 1, clips);
                sCache->Add(mogg, 1, mogg);
                sCache->EndSet();
            }
        }
        m_TimeSinceLoad = 0;
        if (!curEntry.timey_wimey) {
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
        if (!inHollaback && !inMindControl) {
            TheHamDirector->PlayIntroShot();
            static Message songseq_intro("songseq_intro");
            TheHamProvider->Export(songseq_intro, true);
        }
        if (inMindControl) {
            TheGame->Handle(Message("set_realtime", false), true);
            TheHamProvider->SetProperty("game_stage", Symbol("playing"));
            m_HaveReenteredCharacters = true;
            TheGame->Jump(0, true);
            TheHamDirector->Handle(Message("set_suppress_next_shot", 120), true);
        }
    }
}
