#pragma once
#include "meta/FixedSizeSaveable.h"
#include "obj/Data.h"

enum SkillsAward {
    kSkillsAward_Unplayed = 0x0000,
    kSkillsAward_Fail = 0x0001,
    kSkillsAward_Complete = 0x0002,
    kSkillsAward_FastTrack = 0x0003
};

class SkillsAwardList : public FixedSizeSaveable {
public:
    class Key {
    public:
        bool operator<(const Key &key) const {
            if (mSongID != key.mSongID) {
                return mSongID < key.mSongID;
            } else {
                for (int i = 0; i < 4; i++) {
                    if (mMoves[i] != key.mMoves[i]) {
                        return mMoves[i] < key.mMoves[i];
                    }
                }
                return false;
            }
        }

        int mSongID; // 0x0
        Symbol mMoves[4]; // 0x4
    };
    SkillsAwardList() : mDirty(0) { mSaveSizeMethod = SaveSize; }
    virtual ~SkillsAwardList() {}
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    bool HasFailure() const;
    SkillsAward GetAward(DataArray *);
    SkillsAward GetAward(Symbol, DataArray *);
    void SetAward(DataArray *, SkillsAward);
    void SetAward(Symbol, DataArray *, SkillsAward);
    void Clear();
    int AwardCount(SkillsAward) const;
    bool HasAward(const Key &key) { return mMap.count(key) > 0; }

    static int SaveSize(int);

protected:
    std::map<Key, SkillsAward> mMap; // 0x8
    bool mDirty; // 0x20
};
