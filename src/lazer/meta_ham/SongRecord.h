#pragma once

#include "lazer/meta_ham/HamSongMetadata.h"

class SongRecord {
public:
    SongRecord(const HamSongMetadata *meta);
    virtual ~SongRecord() {}

    const Symbol &ShortName() const { return mShortName; }
    const HamSongMetadata *GetData() const { return mData; }
    int GetTier() const;

private:
    Symbol mShortName; // 0x4
    int mTier; // 0x8
    const HamSongMetadata *mData; // 0xc
};
