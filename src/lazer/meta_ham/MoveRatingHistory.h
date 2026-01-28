#pragma once
#include "hamobj/ScoreUtl.h"
#include "meta/FixedSizeSaveable.h"

// size 0x24
class MoveRatingHistory : public FixedSizeSaveable {
public:
    class Key {
    public:
        bool operator<(const Key &k) const { return unk0 < k.unk0; }

        Symbol unk0;
    };
    class RatingHistory {
    public:
        MoveRating unk0[4];
    };
    MoveRatingHistory() : unk20(0) { mSaveSizeMethod = SaveSize; }
    virtual ~MoveRatingHistory() {}
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    void Clear();
    void AddHistory(Symbol, int);
    int GetRating(Symbol, int);
    bool HasRatingHistory(const Key &key) const { return unk8.find(key) != unk8.end(); }

    static int SaveSize(int);

    bool Unk20() const { return unk20; }

private:
    std::map<Key, RatingHistory> unk8;
    bool unk20;
};
