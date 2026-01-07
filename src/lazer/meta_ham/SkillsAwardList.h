#pragma once
#include "meta/FixedSizeSaveable.h"
#include "obj/Data.h"

enum SkillsAward {
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
                    if (unk4[i] != key.unk4[i]) {
                        return unk4[i] < key.unk4[i];
                    }
                }
                return false;
            }
        }

        int mSongID; // 0x0
        Symbol unk4[4];
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

    static int SaveSize(int);

protected:
    std::map<Key, SkillsAward> mAwardList; // 0x8
    bool mDirty; // 0x20
};
