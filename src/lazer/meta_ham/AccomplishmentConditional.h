#pragma once

#include "hamobj/Difficulty.h"
#include "lazer/meta_ham/Accomplishment.h"
#include "obj/Data.h"
#include "stl/_vector.h"

struct AccomplishmentCondition {
    u32 unk0;
    u32 unk4;
    u32 unk8;
    Difficulty mDifficulty; // 0xc
    Symbol mCharacter; // 0x10
    Symbol mMode; // 0x14
    bool mFlashcards; // 0x18
};

class AccomplishmentConditional : public Accomplishment {
public:
    virtual ~AccomplishmentConditional();
    virtual Difficulty GetRequiredDifficulty() const;

    AccomplishmentConditional(DataArray *, int);

    std::list<AccomplishmentCondition> m_lConditions; // 0x68

protected:
    void Configure(DataArray *);
    void UpdateConditionOptionalData(AccomplishmentCondition &, DataArray *);
};
