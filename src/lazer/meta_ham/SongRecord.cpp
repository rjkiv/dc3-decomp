#include "SongRecord.h"
#include "lazer/meta_ham/HamSongMgr.h"

SongRecord::SongRecord(const HamSongMetadata *meta) : mShortName(), mMetadata(meta) {
    mShortName = TheHamSongMgr.GetShortNameFromSongID(meta->ID(), 1);
    mRankTier = TheHamSongMgr.RankTier(meta->Rank());
}
