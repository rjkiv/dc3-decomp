#pragma once
#include "meta/FixedSizeSaveable.h"
#include "obj/Data.h"

enum SkillsAward {
};

class SkillsAwardList : public FixedSizeSaveable {
public:
    class Key {
    public:
        bool operator<(const Key &);
    };
    SkillsAwardList() : unk20(0) { mSaveSizeMethod = SaveSize; }
    virtual ~SkillsAwardList() {}
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);

    bool HasFailure() const;
    SkillsAward GetAward(DataArray *);
    SkillsAward GetAward(Symbol, DataArray *);
    void SetAward(DataArray *, SkillsAward);
    void Clear();
    int AwardCount(SkillsAward) const;

    static int SaveSize(int);

protected:
    std::map<Key, SkillsAward> unk8;
    bool unk20;
};
