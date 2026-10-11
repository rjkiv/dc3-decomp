#pragma once
#include "hamobj/ScoreUtl.h"
#include "meta/FixedSizeSaveable.h"

// size 0x24
class MoveRatingHistory : public FixedSizeSaveable {
public:
    class Key {
    public:
        bool operator<(const Key &k) const { return mMove < k.mMove; }

        Symbol mMove; // 0x0
    };
    class RatingHistory {
    public:
        MoveRating mRating[4]; // 0x0
    };
    MoveRatingHistory() : mDirty(0) { mSaveSizeMethod = SaveSize; }
    virtual ~MoveRatingHistory() {}
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    void Clear();
    void AddHistory(Symbol, int);
    int GetRating(Symbol, int);
    bool HasRatingHistory(const Key &key) const { return mMap.count(key) > 0; }

    static int SaveSize(int);

    bool Dirty() { return mDirty; }

protected:
    std::map<Key, RatingHistory> mMap; // 0x8
    bool mDirty; // 0x20
};
