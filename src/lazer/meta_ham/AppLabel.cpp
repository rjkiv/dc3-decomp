#include "meta_ham/AppLabel.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamLabel.h"
#include "meta/StoreOffer.h"
#include "meta_ham/ChallengeSortMgr.h"
#include "meta_ham/Challenges.h"
#include "meta_ham/ContextChecker.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/HamSongMetadata.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/HamStoreFilterProvider.h"
#include "meta_ham/Instarank.h"
#include "meta_ham/NavListNode.h"
#include "meta_ham/Playlist.h"
#include "meta_ham/PracticeChoosePanel.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/SkeletonIdentifier.h"
#include "meta_ham/SongStatusMgr.h"
#include "meta_ham/Utl.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "ui/UIListSlot.h"
#include "utl/Locale.h"
#include "utl/Symbol.h"

BEGIN_HANDLERS(AppLabel)
    HANDLE(set_user_name, OnSetUserName)
    HANDLE_ACTION(set_instarank, SetInstarank(_msg->Obj<Instarank>(2)))
    HANDLE_ACTION(set_song_name, SetSongName(_msg->Sym(2), -1, false))
    HANDLE_ACTION(
        set_playlist_song_name,
        SetPlaylistSongName(_msg->Sym(2), _msg->Int(3), _msg->Int(4))
    )
    HANDLE_ACTION(set_album_name, SetAlbumName(_msg->Sym(2)))
    HANDLE_ACTION(set_artist_name, SetArtistName(_msg->Sym(2), false))
    HANDLE_ACTION(set_dancer, SetDancer(_msg->Sym(2)))
    HANDLE_ACTION(
        set_from_song_select_node, SetFromSongSelectNode(_msg->Obj<NavListNode>(2))
    )
    HANDLE_ACTION(set_last_played_time, SetLastPlayedTime(_msg->Int(2)))
    HANDLE_ACTION(set_last_played_score, SetLastPlayedScore(_msg->Int(2)))
    HANDLE_ACTION(set_last_practice_time, SetLastPracticeTime(_msg->Int(2)))
    HANDLE_ACTION(set_best_score, SetBestScore(_msg->Int(2)))
    HANDLE_ACTION(set_best_coop_score, SetBestCoopScore(_msg->Int(2)))
    HANDLE_ACTION(
        set_best_perform_percent,
        SetBestPerformPercent(_msg->Int(2), (Difficulty)_msg->Int(3))
    )
    HANDLE_ACTION(
        set_best_perform_perfect,
        SetBestPerformPerfect(_msg->Int(2), (Difficulty)_msg->Int(3))
    )
    HANDLE_ACTION(
        set_best_perform_nice, SetBestPerformNice(_msg->Int(2), (Difficulty)_msg->Int(3))
    )
    HANDLE_ACTION(
        set_best_battle_score, SetBestBattleScore(_msg->Obj<HamProfile>(2), _msg->Int(3))
    )
    HANDLE_ACTION(
        set_total_battle_wins, SetTotalBattleWins(_msg->Obj<HamProfile>(2), _msg->Int(3))
    )
    HANDLE_ACTION(
        set_total_battle_losses,
        SetTotalBattleLosses(_msg->Obj<HamProfile>(2), _msg->Int(3))
    )
    HANDLE_ACTION(set_locked, SetLocked(_msg->Int(2)))
    HANDLE_ACTION(set_checked, SetChecked(_msg->Int(2)))
    HANDLE_ACTION(set_new, SetNew(_msg->Int(2)))
    HANDLE_ACTION(set_diff_score, SetDiffScore(_msg->Int(2), (Difficulty)_msg->Int(3)))
    HANDLE_ACTION(set_fitness_time, SetFitnessTime(_msg->Obj<HamProfile>(2)))
    HANDLE_ACTION(set_fitness_calories, SetFitnessCalories(_msg->Obj<HamProfile>(2)))
    HANDLE_ACTION(
        set_fitness_total_calories, SetFitnessTotalCalories(_msg->Obj<HamProfile>(2))
    )
    HANDLE_ACTION(set_fitness_time_num, SetFitnessTimeNum(_msg->Obj<HamProfile>(2)))
    HANDLE_ACTION(
        set_fitness_total_calories_num,
        SetFitnessTotalCaloriesNum(_msg->Obj<HamProfile>(2))
    )
    HANDLE_ACTION(set_random_tip, SetRandomTip())
    HANDLE_ACTION(set_offer_name, SetStoreOfferName(_msg->Obj<StoreOffer>(2)))
    HANDLE_ACTION(set_offer_artist, SetStoreOfferArtist(_msg->Obj<StoreOffer>(2)))
    HANDLE_ACTION(set_offer_album, SetStoreOfferAlbum(_msg->Obj<StoreOffer>(2)))
    HANDLE_ACTION(set_player_high_score, SetPlayerHighScore(_msg->Int(2)))
    HANDLE_ACTION(set_player_challenge_score, SetPlayerChallengeScore(_msg->Int(2)))
    HANDLE_ACTION(set_challenge_exp, SetChallengeExp(_msg->Int(2)))
    HANDLE_ACTION(set_potential_challenge_exp, SetPotentialChallengeExp(_msg->Int(2)))
    HANDLE_ACTION(set_challenger_gamertag, SetChallengerGamertag(_msg->Int(2)))
    HANDLE_ACTION(set_challenge_score, SetChallengeScore(_msg->Int(2)))
    HANDLE_ACTION(set_expire_time, SetExpireTime())
    HANDLE_ACTION(set_modal_count, SetMedalCount(_msg->Int(2)))
    HANDLE_SUPERCLASS(HamLabel)
END_HANDLERS

void AppLabel::SetCreditsText(DataArray *arr, UIListSlot *slot) {
    static Symbol heading("heading");
    static Symbol image("image");
    static Symbol blank("blank");
    static Symbol heading2("heading2");
    static Symbol title_name("title_name");
    static Symbol centered("centered");
    Symbol sym = blank;
    if (arr->Size() != 0) {
        sym = arr->Sym(0);
    }
    if (sym == blank) {
        SetTextToken(gNullStr);
    } else if (heading == sym) {
        if (slot->Matches("heading")) {
            SetDisplayText(arr->Str(1), true);
        } else
            SetTextToken(gNullStr);
    } else if (heading2 == sym) {
        if (slot->Matches("slot_heading")) {
            SetDisplayText(arr->Str(1), true);
        } else
            SetTextToken(gNullStr);
    } else if (title_name == sym) {
        if (slot->Matches("slot_left")) {
            SetDisplayText(arr->Str(1), true);
        } else if (slot->Matches("slot_right")) {
            SetDisplayText(arr->Str(2), true);
        } else
            SetTextToken(gNullStr);
    } else if (centered == sym) {
        if (slot->Matches("slot_centered")) {
            SetDisplayText(arr->Str(1), true);
        } else
            SetTextToken(gNullStr);
    } else if (image == sym) {
        SetTextToken(gNullStr);
    } else
        SetTextToken(gNullStr);
}

void AppLabel::SetLocked(bool locked) {
    if (locked) {
        SetIcon('B');
    } else {
        SetDisplayText(gNullStr, true);
    }
}

void AppLabel::SetChecked(bool checked) { SetIcon(checked ? 'b' : 'a'); }
void AppLabel::SetUserName(int x) { SetDisplayText(ThePlatformMgr.GetName(x), true); }
void AppLabel::SetUserName(const User *user) { SetDisplayText(user->UserName(), true); }

void AppLabel::SetAlbumName(Symbol shortname) {
    int songID = TheHamSongMgr.GetSongIDFromShortName(shortname);
    SetDisplayText(TheHamSongMgr.Data(songID)->Album(), true);
}

void AppLabel::SetInstarank(const Instarank *rank) { SetDisplayText(rank->Str(), true); }

void AppLabel::SetFromSongSelectNode(const NavListNode *node) {
    const DateTime *dt = node->GetDateTime();
    if (dt) {
        static Symbol year_format("year_format");
        SetDateTime(*dt, year_format);
    } else if (node->LocalizeToken()) {
        SetTextToken(node->GetToken());
    } else {
        SetDisplayText(node->GetToken().Str(), true);
    }
}

void AppLabel::SetFromGeneralSelectNode(const NavListNode *node) {
    if (node->LocalizeToken()) {
        SetTextToken(node->GetToken());
    } else {
        SetDisplayText(node->GetToken().Str(), true);
    }
}

void AppLabel::SetFromPlaylistSelectNode(const NavListNode *node) {
    if (node->LocalizeToken()) {
        SetTextToken(node->GetToken());
    } else {
        SetDisplayText(node->GetToken().Str(), true);
    }
}

void AppLabel::SetChallengerName(const char *name) { SetDisplayText(name, true); }

void AppLabel::SetLastPlayedScore(int x) {
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    if (profile) {
        SongStatusMgr *mgr = profile->GetSongStatusMgr();
        bool bref;
        int score = mgr->GetLastScore(x, bref);
        static Symbol no_flashcards_icon("no_flashcards_icon");
        String localized(LocalizeSeparatedInt(score, TheLocale));
        if (bref) {
            localized += " <alt>";
            localized += Localize(no_flashcards_icon, nullptr, TheLocale);
            localized += "</alt>";
        }
        const char *text;
        if (score) {
            text = localized.c_str();
        } else {
            text = gNullStr;
        }
        SetDisplayText(text, true);
    } else {
        SetDisplayText(gNullStr, true);
    }
}

void AppLabel::SetBestPerformPerfect(int songID, Difficulty diff) {
    int num = 0;
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    if (profile) {
        num = profile->GetSongStatusMgr()->GetNumPerfectForDifficulty(songID, diff);
    }
    SetInt(num, false);
}

void AppLabel::SetBestPerformNice(int songID, Difficulty diff) {
    int num = 0;
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    if (profile) {
        num = profile->GetSongStatusMgr()->GetNumNiceForDifficulty(songID, diff);
    }
    SetInt(num, false);
}

void AppLabel::SetTotalBattleWins(HamProfile *profile, int songID) {
    int num = 0;
    if (profile) {
        num = profile->GetSongStatusMgr()->GetTotalBattleWins(songID);
    }
    SetInt(num, false);
}

void AppLabel::SetTotalBattleLosses(HamProfile *profile, int songID) {
    int num = 0;
    if (profile) {
        num = profile->GetSongStatusMgr()->GetTotalBattleLosses(songID);
    }
    SetInt(num, false);
}

void AppLabel::SetNew(bool isNew) {
    static Symbol new_content("new_content");
    if (isNew) {
        SetTextToken(new_content);
    } else {
        SetDisplayText(gNullStr, true);
    }
}

void AppLabel::SetBuy(bool buy) {
    static Symbol buy_icon("buy_icon");
    if (buy) {
        SetTextToken(buy_icon);
    } else {
        SetDisplayText(gNullStr, true);
    }
}

void AppLabel::SetDownload(bool download) {
    if (download) {
        SetIcon('G');
    } else {
        SetDisplayText(gNullStr, true);
    }
}

void AppLabel::SetRandomTip() {
    Symbol item = RandomContextSensitiveItem(SystemConfig("tips"));
    SetTextToken(item);
}

void AppLabel::SetEnrolledPlayerName(int x) {
    SetDisplayText(TheSkeletonIdentifier->GetPlayerName(x).c_str(), true);
}

void AppLabel::SetStoreOfferName(const StoreOffer *offer) {
    SetDisplayText(offer->OfferName(), true);
}

void AppLabel::SetStoreOfferArtist(const StoreOffer *offer) {
    SetDisplayText(offer->ArtistName(), true);
}

void AppLabel::SetStoreOfferAlbum(const StoreOffer *offer) {
    SetDisplayText(offer->AlbumName(), true);
}

void AppLabel::SetStoreOfferCost(const StoreOffer *offer) {
    SetDisplayText(offer->CostStr(), true);
}

void AppLabel::SetPlayerHighScore(int i1) {
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    if (profile) {
        bool bref;
        int score =
            profile->GetSongStatusMgr()->GetBestScore(i1, bref, kDifficultyBeginner);
        SetDisplayText(LocalizeSeparatedInt(score, TheLocale), true);
    } else {
        SetDisplayText(gNullStr, true);
    }
}

void AppLabel::SetPlayerChallengeScore(int i1) {
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    if (profile) {
        int score = TheChallengeSortMgr->GetOwnerChallengeScore(i1);
        SetDisplayText(LocalizeSeparatedInt(score, TheLocale), true);
    } else {
        SetDisplayText(gNullStr, true);
    }
}

void AppLabel::SetSongDuration(Symbol shortname) {
    String str;
    if (TheHamSongMgr.HasSong(shortname, false)) {
        const HamSongMetadata *pData =
            TheHamSongMgr.Data(TheHamSongMgr.GetSongIDFromShortName(shortname, false));
        MILO_ASSERT(pData, 0x7C);
        str = FormatTimeMS(TheHamSongMgr.GetDuration(shortname));
    } else {
        str = "";
    }
    SetDisplayText(str.c_str(), true);
}

void AppLabel::SetArtistName(Symbol shortname, bool b2) {
    const HamSongMetadata *pMetaData =
        TheHamSongMgr.Data(TheHamSongMgr.GetSongIDFromShortName(shortname));
    MILO_ASSERT(pMetaData, 0xF1);
    if (pMetaData->IsCover() && !b2) {
        static Symbol as_made_famous_by("as_made_famous_by");
        SetDisplayText(
            MakeString(
                Localize(as_made_famous_by, nullptr, TheLocale), pMetaData->Artist()
            ),
            true
        );
    } else {
        SetDisplayText(pMetaData->Artist(), true);
    }
}

void AppLabel::SetChallengeScoreLabel(int i1) {
    const char *label = MakeString("%i", i1);
    SetDisplayText(label, true);
}

void AppLabel::SetBestPracticeDifficulty(int songID) {
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    if (profile) {
        Difficulty d = profile->GetSongStatusMgr()->GetPracticeDifficulty(songID);
        if (profile->GetSongStatusMgr()->GetPracticeScore(songID) > 0) {
            SetTextToken(MakeString("%s_short", DifficultyToSym(d)));
            return;
        }
    }
    SetDisplayText(gNullStr, true);
}

void AppLabel::SetFitnessTimeNum(HamProfile *profile) {
    MILO_ASSERT(profile, 0x2E4);
    float f1, f2, f3;
    profile->GetFitnessStats(f1, f2, f3);
    SetDisplayText(FormatTimeHMS(f1), true);
}

void AppLabel::SetFitnessTotalCaloriesNum(HamProfile *profile) {
    MILO_ASSERT(profile, 0x2ED);
    float f1, f2, f3;
    profile->GetFitnessStats(f1, f2, f3);
    SetDisplayText(LocalizeFloat("%5.2f", f2), true);
}

void AppLabel::SetPlaylistName(const Playlist *playlist, bool b2, bool b3) {
    MILO_ASSERT(playlist, 0x2FB);
    String str(Localize(playlist->GetName(), nullptr, TheLocale));
    if (!b3) {
        unsigned int idx = str.find("<altb>");
        if (idx < str.length()) {
            str.replace(idx, 6, "");
            idx = str.find("</alt>");
            if (idx < str.length()) {
                str.replace(idx, 6, "");
            }
            idx = str.find("</altb>");
            if (idx < str.length()) {
                str.replace(idx, 7, "");
            }
        }
    }
    if (b2) {
        const char *time = FormatTimeMS(playlist->GetDuration());
        static Symbol songname_duration("songname_duration");
        str = MakeString(
            Localize(songname_duration, nullptr, TheLocale), str.c_str(), time
        );
    }
    SetDisplayText(str.c_str(), true);
}

void AppLabel::SetPackSongName(const DataArray *a1) {
    static Symbol name("name");
    SetDisplayText(a1->FindStr(name), true);
}

void AppLabel::SetSongName(Symbol s1, int i2, bool b3) {
    if (streq(s1.Str(), "blank")) {
        SetDisplayText("", true);
        return;
    }
    String str;
    if (TheHamSongMgr.HasSong(s1, false)) {
        int songID = TheHamSongMgr.GetSongIDFromShortName(s1, false);
        const HamSongMetadata *data = TheHamSongMgr.Data(songID);
        str = data->Title();
        if (b3) {
            static Symbol songname_duration("songname_duration");
            Symbol shortnameFromSongID = TheHamSongMgr.GetShortNameFromSongID(songID);
            const char *time =
                FormatTimeMS(TheHamSongMgr.GetDuration(shortnameFromSongID));
            str = MakeString(
                Localize(songname_duration, nullptr, TheLocale), str.c_str(), time
            );
        }
    } else {
        static Symbol song_unknown("song_unknown");
        str = Localize(song_unknown, nullptr, TheLocale);
    }
    if (i2 >= 0) {
        static Symbol songname_numbered("songname_numbered");
        SetTokenFmt(songname_numbered, i2, str.c_str());
    } else {
        SetDisplayText(str.c_str(), true);
    }
}

void AppLabel::SetBlacklightSongName(Symbol s1, int i2, bool b3) {
    if (streq(s1.Str(), "blank")) {
        SetDisplayText("", true);
        return;
    }
    String str;
    if (TheHamSongMgr.HasSong(s1, false)) {
        int songID = TheHamSongMgr.GetSongIDFromShortName(s1, false);
        const HamSongMetadata *data = TheHamSongMgr.Data(songID);
        str = MakeString("%s%s%s", "<altb>", data->Title(), "</altb>");
        if (b3) {
            static Symbol songname_duration("songname_duration");
            Symbol shortnameFromSongID = TheHamSongMgr.GetShortNameFromSongID(songID);
            const char *time =
                FormatTimeMS(TheHamSongMgr.GetDuration(shortnameFromSongID));
            str = MakeString(
                Localize(songname_duration, nullptr, TheLocale), str.c_str(), time
            );
        }
    } else {
        static Symbol song_unknown("song_unknown");
        str = Localize(song_unknown, nullptr, TheLocale);
    }
    if (i2 >= 0) {
        static Symbol songname_numbered("songname_numbered");
        SetTokenFmt(songname_numbered, i2, str.c_str());
    } else {
        SetDisplayText(str.c_str(), true);
    }
}

void AppLabel::SetPlaylistSongName(Symbol s1, int i2, int i3) {
    int songID = TheHamSongMgr.GetSongIDFromShortName(s1);
    static Symbol playlist_song_name("playlist_song_name");
    const HamSongMetadata *data = TheHamSongMgr.Data(songID);
    SetTokenFmt(playlist_song_name, data->Title(), i2, i3);
}

void AppLabel::SetDancer(Symbol s1) {
    static Symbol defaultcharacter_label("defaultcharacter_label");
    int songID = TheHamSongMgr.GetSongIDFromShortName(s1);
    const HamSongMetadata *data = TheHamSongMgr.Data(songID);
    Symbol charSym = data->Character();
    SetTokenFmt(defaultcharacter_label, Localize(charSym, nullptr, TheLocale));
}

void AppLabel::SetLastPracticeTime(int i1) {
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    unsigned int time = 0;
    if (profile) {
        time = profile->GetSongStatusMgr()->GetLastPlayedPractice(i1);
    }
    SetTimeElapsedSince(time);
}

void AppLabel::SetBestScore(int i1) {
    static Symbol best_score("best_score");
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    if (profile) {
        bool bref = false;
        int score = profile->GetSongStatusMgr()->GetScore(i1, bref);
        if (score > 0) {
            SetTokenFmt(best_score, LocalizeSeparatedInt(score, TheLocale));
            return;
        }
    }
    SetDisplayText(gNullStr, true);
}

void AppLabel::SetBestCoopScore(int i1) {
    static Symbol best_score("best_score");
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    if (profile) {
        int score = profile->GetSongStatusMgr()->GetCoopScore(i1);
        if (score > 0) {
            SetTokenFmt(best_score, LocalizeSeparatedInt(score, TheLocale));
            return;
        }
    }
    SetDisplayText(gNullStr, true);
}

void AppLabel::SetBestPerformPercent(int i1, Difficulty d) {
    int pct = 0;
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    if (profile) {
        pct = profile->GetSongStatusMgr()->GetPercentForDifficulty(i1, d);
    }
    static Symbol percentage("percentage");
    SetTokenFmt(percentage, pct);
}

void AppLabel::SetBestBattleScore(HamProfile *profile, int i2) {
    if (profile) {
        static Symbol best_score("best_score");
        int score = profile->GetSongStatusMgr()->GetBestBattleScore(i2);
        SetTokenFmt(best_score, LocalizeSeparatedInt(score, TheLocale));
    } else {
        SetInt(0, false);
    }
}

bool AppLabel::SetPracticeScore(int i1, Difficulty d) {
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    if (profile) {
        int score;
        if (d == kNumDifficulties) {
            score = profile->GetSongStatusMgr()->GetPracticeScore(i1);
        } else {
            score = profile->GetSongStatusMgr()->GetPracticeScore(i1, d);
        }
        static Symbol percentage("percentage");
        if (score > 0) {
            SetTokenFmt(percentage, score);
            return true;
        }
    }
    SetDisplayText(gNullStr, true);
    return false;
}

void AppLabel::SetDiffScore(int i1, Difficulty d) {
    static Symbol best_score_diff("best_score_diff");
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    if (profile) {
        bool bref;
        int score = profile->GetSongStatusMgr()->GetScoreForDifficulty(i1, d, bref);
        if (score > 0) {
            const char *cc = bref ? "Q" : " ";
            SetTokenFmt(best_score_diff, LocalizeSeparatedInt(score, TheLocale), cc);
            return;
        }
    }
    SetDisplayText(gNullStr, true);
}

void AppLabel::SetFitnessTime(HamProfile *profile) {
    MILO_ASSERT(profile, 0x2C0);
    static Symbol calories_time("calories_time");
    float f1, f2, f3;
    profile->GetFitnessStats(f1, f2, f3);
    char str[40];
    GetTimeString(f1, str);
    SetTokenFmt(calories_time, str);
}

void AppLabel::SetFitnessCalories(HamProfile *profile) {
    MILO_ASSERT(profile, 0x2CD);
    static Symbol calories_song("calories_song");
    float f1, f2, f3;
    profile->GetFitnessStats(f1, f2, f3);
    SetTokenFmt(calories_song, (int)f3);
}

void AppLabel::SetFitnessTotalCalories(HamProfile *profile) {
    MILO_ASSERT(profile, 0x2D8);
    static Symbol calories_total("calories_total");
    float f1, f2, f3;
    profile->GetFitnessStats(f1, f2, f3);
    SetTokenFmt(calories_total, (int)f2);
}

void AppLabel::SetLastPlayedTime(int x) {
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    unsigned int last = 0;
    if (profile) {
        last = profile->GetSongStatusMgr()->GetLastPlayed(x);
    }
    SetTimeElapsedSince(last);
}

void AppLabel::SetStepMoveName(const StepMoves &moves) {
    SetDisplayText(moves.GetDisplayName(false).c_str(), true);
}

void AppLabel::SetChallengeExp(int i1) {
    SetDisplayText(
        LocalizeSeparatedInt(TheChallengeSortMgr->GetChallengeExp(i1), TheLocale), true
    );
}

void AppLabel::SetPotentialChallengeExp(int i1) {
    SetDisplayText(
        LocalizeSeparatedInt(TheChallengeSortMgr->GetPotentialChallengeExp(i1), TheLocale),
        true
    );
}

void AppLabel::SetChallengerGamertag(int i1) {
    SetDisplayText(TheChallengeSortMgr->GetChallengerGamertag(i1), true);
}

void AppLabel::SetChallengeScore(int i1) {
    SetDisplayText(
        LocalizeSeparatedInt(TheChallengeSortMgr->GetChallengeScore(i1), TheLocale), true
    );
}

void AppLabel::SetMedalCount(int i1) {
    SetDisplayText(
        LocalizeSeparatedInt(TheChallenges->GetMedalCount(i1), TheLocale), true
    );
}

void AppLabel::SetExpireTime() {
    static Symbol challenge_expire_time_int_fmt("challenge_expire_time_int_fmt");
    static Symbol challenge_expire_time_invalid("challenge_expire_time_invalid");
    int i1 = 0;
    int i2 = 0;
    int i3 = 0;
    int i4 = 0;
    if (TheChallenges->GetExpireTime(i1, i2, i3, i4)) {
        SetTokenFmt(challenge_expire_time_int_fmt, i1, i2, i3, i4);
    } else {
        SetTextToken(challenge_expire_time_invalid);
    }
}

DataNode AppLabel::OnSetUserName(const DataArray *a) {
    const DataNode &n = a->Evaluate(2);
    if (n.Type() == kDataInt) {
        SetUserName(n.Int());
    } else {
        User *user = n.Obj<User>();
        if (user) {
            SetUserName(user);
        } else {
            MILO_NOTIFY("Could not set user name, unknown class");
        }
    }
    return 1;
}
