#include "meta_ham/HamStarsDisplay.h"
#include "hamobj/Difficulty.h"
#include "hamobj/StarsDisplay.h"
#include "meta/SongMgr.h"
#include "meta_ham/CampaignPerformer.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/ProfileMgr.h"
#include "meta_ham/Utl.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/MakeString.h"
#include "utl/Symbol.h"

BEGIN_HANDLERS(HamStarsDisplay)
    HANDLE_ACTION(set_song, SetSong(_msg->Int(2)))
    HANDLE_ACTION(
        set_song_with_diff,
        SetSongWithDifficulty(_msg->Int(2), (Difficulty)_msg->Int(3), false)
    )
    HANDLE_ACTION(
        set_song_with_diff_always,
        SetSongWithDifficulty(_msg->Int(2), (Difficulty)_msg->Int(3), true)
    )
    HANDLE_ACTION(set_song_last_played, SetSongLastPlayed(_msg->Int(2)))
    HANDLE_SUPERCLASS(StarsDisplay)
END_HANDLERS

void HamStarsDisplay::SetSongChallenge(Difficulty diff) {
    MILO_ASSERT(diff != kNumDifficulties, 0x2f);
    mDiffLabel->SetShowing(true);
    char const *s = MakeString("%s_short", DifficultyToSym(diff));
    mDiffLabel->SetTextToken(s);
    mStarsLabel->SetShowing(false);
    mNoFlashcardsLabel->SetShowing(false);
}

void HamStarsDisplay::SetSong(int i) {
    SetSongImpl(i, kNumDifficulties, kStarDisplayStandard);
}

void HamStarsDisplay::SetSongLastPlayed(int i) {
    SetSongImpl(i, kNumDifficulties, kStarDisplayLastPlayed);
}

void HamStarsDisplay::SetSongCampaign(int i) {
    SetSongImpl(i, kNumDifficulties, kStarDisplayCampaign);
}

void HamStarsDisplay::SetSongWithDifficulty(int i, Difficulty d, bool always) {
    SetSongImpl(i, d, always ? kStarDisplayWithDiffAlways : kStarDisplayWithDiff);
}

void HamStarsDisplay::SetSongImpl(int songID, Difficulty diff, StarDisplayMode mode) {
    bool u11 = false;
    int stars = 0;
    bool c10 = false;
    HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
    if (profile) {
        if (profile->GetSongStatusMgr()->HasSongStatus(songID)) {
            const SongStatus &status = profile->GetSongStatusMgr()->GetSongStatus(songID);
            switch (mode) {
            case 0: {
                MILO_ASSERT(diff == kNumDifficulties, 0x48);
                const SongStatusData &data = status.GetBestSongStatusData();
                if (data.mScore > 0) {
                    u11 = true;
                    for (int i = 0; i < kNumDifficulties; i++) {
                        if (&data == &status.mStatusData[i]) {
                            diff = (Difficulty)i;
                        }
                    }
                    stars = data.mStars;
                    c10 = data.unk10;
                }
                break;
            }
            case 1:
            case 2: {
                MILO_ASSERT(diff < kNumDifficulties, 0x5C);
                if (status.mStatusData[diff].mScore > 0) {
                    stars = status.mStatusData[diff].mStars;
                    u11 = true;
                    c10 = status.mStatusData[diff].unk10;
                }
                break;
            }
            case 3: {
                MILO_ASSERT(diff == kNumDifficulties, 0x67);
                if (status.unk7c != kNumDifficulties) {
                    stars = status.unk78;
                    u11 = true;
                    c10 = status.unk79;
                    diff = status.unk7c;
                }
                break;
            }
            case 4: {
                MILO_ASSERT(diff == kNumDifficulties, 0x73);
                CampaignPerformer *pPerformer =
                    dynamic_cast<CampaignPerformer *>(MetaPerformer::Current());
                MILO_ASSERT(pPerformer, 0x76);
                Symbol era = pPerformer->Era();
                Symbol shortname = TheSongMgr.GetShortNameFromSongID(songID);
                HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
                MILO_ASSERT(pProfile, 0x7B);
                const CampaignProgress &prog =
                    pProfile->GetCampaignProgress(pPerformer->GetDifficulty());
                stars = prog.GetSongStarsEarned(era, shortname);
                u11 = stars > 0;
                diff = pPerformer->GetDifficulty();
                break;
            }
            }
        }
    }
    if (u11) {
        mDiffLabel->SetShowing(true);
        mDiffLabel->SetTextToken(MakeString("%s_short", DifficultyToSym(diff)));
        mStarsLabel->SetShowing(true);
        mStarsLabel->SetTextToken(GetStarsToken(stars));
        if (c10) {
            mNoFlashcardsLabel->SetShowing(true);
        } else {
            mNoFlashcardsLabel->SetShowing(false);
        }
    } else {
        if (mShowUnplayedSong) {
            if (mode == 2) {
                mDiffLabel->SetShowing(true);
                mDiffLabel->SetTextToken(MakeString("%s_short", DifficultyToSym(diff)));
            } else {
                mDiffLabel->SetShowing(false);
            }
            mStarsLabel->SetShowing(true);
            mStarsLabel->SetTextToken(GetStarsToken(0));
        } else {
            mDiffLabel->SetShowing(false);
            mStarsLabel->SetShowing(false);
        }
        mNoFlashcardsLabel->SetShowing(false);
    }
}
