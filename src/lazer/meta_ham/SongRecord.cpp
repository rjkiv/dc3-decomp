#include "SongRecord.h"
#include "lazer/meta_ham/HamSongMgr.h"

SongRecord::SongRecord(const HamSongMetadata *meta) : mShortName(), mData(meta) {
    mShortName = TheHamSongMgr.GetShortNameFromSongID(meta->ID(), 1);
    mTier = TheHamSongMgr.RankTier(meta->Rank());
}

int SongRecord::GetTier() const { return mTier; }
