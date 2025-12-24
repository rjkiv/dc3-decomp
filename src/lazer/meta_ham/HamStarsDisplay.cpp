#include "meta_ham/HamStarsDisplay.h"
#include "hamobj/Difficulty.h"
#include "hamobj/StarsDisplay.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"

void HamStarsDisplay::SetSongChallenge(Difficulty diff) {
    MILO_ASSERT(diff != kNumDifficulties, 0x2f);
    mDiffLabel->SetShowing(true);
    char const *s = MakeString("%s_short", DifficultyToSym(diff));
    mDiffLabel->SetTextToken(s);
    mStarsLabel->SetShowing(false);
    mNoFlashcardsLabel->SetShowing(false);
}

void HamStarsDisplay::SetSong(int i) {
    SetSongImpl(i, kNumDifficulties, (StarDisplayMode)0);
}

void HamStarsDisplay::SetSongCampaign(int i) {
    SetSongImpl(i, kNumDifficulties, (StarDisplayMode)4);
}

void HamStarsDisplay::SetSongWithDifficulty(int i, Difficulty d, bool b) {
    SetSongImpl(i, d, b ? (StarDisplayMode)2 : (StarDisplayMode)1);
}

BEGIN_HANDLERS(HamStarsDisplay)
    HANDLE_ACTION(
        set_song, SetSongImpl(_msg->Int(2), kNumDifficulties, (StarDisplayMode)0)
    )
    HANDLE_ACTION(
        set_song_with_diff,
        SetSongImpl(_msg->Int(2), (Difficulty)_msg->Int(3), (StarDisplayMode)1)
    )
    HANDLE_ACTION(
        set_song_with_diff_always,
        SetSongImpl(_msg->Int(2), (Difficulty)_msg->Int(3), (StarDisplayMode)2)
    )
    HANDLE_ACTION(
        set_song_last_played,
        SetSongImpl(_msg->Int(2), kNumDifficulties, (StarDisplayMode)3)
    )
    HANDLE_SUPERCLASS(StarsDisplay)
END_HANDLERS
