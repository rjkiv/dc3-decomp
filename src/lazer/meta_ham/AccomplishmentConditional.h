#pragma once

#include "hamobj/Difficulty.h"
#include "lazer/meta_ham/Accomplishment.h"
#include "obj/Data.h"
#include "stl/_vector.h"
#include "utl/Symbol.h"

struct AccomplishmentCondition {
    u32 unk0;
    int unk4;
    Symbol unk8; // 0x8
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
