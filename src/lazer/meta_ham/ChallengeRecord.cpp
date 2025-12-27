#include "meta_ham/ChallengeRecord.h"
#include "meta_ham/Challenges.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/ProfileMgr.h"

ChallengeRecord::ChallengeRecord(ChallengeRow row) {
    mRow = row;
    unk40 = TheHamSongMgr.GetShortNameFromSongID(mRow.mSongID, false);
    if (unk40.Null()) {
        if (TheChallenges->IsExportedSongDC1(mRow.mSongID)) {
            unk50 = 2;
        } else if (TheChallenges->IsExportedSongDC2(mRow.mSongID)) {
            unk50 = 3;
        } else {
            unk50 = 4;
        }
    } else if (TheProfileMgr.IsContentUnlocked(unk40)) {
        unk50 = 0;
    } else {
        unk50 = 1;
    }
    unk44 = mRow.mSongTitle.c_str();
    unk48 = Symbol(mRow.mGamertag.c_str());
    unk4c = Symbol(mRow.unk2c.c_str());
}
