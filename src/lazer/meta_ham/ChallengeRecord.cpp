#include "meta_ham/ChallengeRecord.h"
#include "meta_ham/Challenges.h"
#include "meta_ham/HamSongMgr.h"
#include "meta_ham/ProfileMgr.h"

ChallengeRecord::ChallengeRecord(ChallengeRow row) {
    mData = row;
    mSongShortName = TheHamSongMgr.GetShortNameFromSongID(mData.songId, false);
    if (mSongShortName.Null()) {
        if (TheChallenges->IsExportedSongDC1(mData.songId)) {
            mChallengeType = 2;
        } else if (TheChallenges->IsExportedSongDC2(mData.songId)) {
            mChallengeType = 3;
        } else {
            mChallengeType = 4;
        }
    } else if (TheProfileMgr.IsContentUnlocked(mSongShortName)) {
        mChallengeType = 0;
    } else {
        mChallengeType = 1;
    }
    mSongTitle = mData.songName.c_str();
    mChallenger = Symbol(mData.challengerUsername.c_str());
    mChallengee = Symbol(mData.challengeeUsername.c_str());
}
