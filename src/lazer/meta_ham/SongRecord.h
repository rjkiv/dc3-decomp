#pragma once

#include "lazer/meta_ham/HamSongMetadata.h"

class SongRecord {
public:
    SongRecord(const HamSongMetadata *meta);
    virtual ~SongRecord() {}

protected:
    Symbol mShortName;
    int mRankTier;
    const HamSongMetadata *mMetadata;
};
