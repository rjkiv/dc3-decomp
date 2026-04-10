#pragma once
#include "hamobj/Difficulty.h"
#include "lazer/meta_ham/Accomplishment.h"
#include "obj/Data.h"
#include "utl/Symbol.h"
#include <vector>

struct AccomplishmentCondition {
    Symbol mCondition; // 0x0
    int mValue; // 0x4
    Symbol mEra; // 0x8
    Difficulty mDifficulty; // 0xc
    Symbol mCharacter; // 0x10
    Symbol mMode; // 0x14
    bool mNoFlashcards; // 0x18
};

class AccomplishmentConditional : public Accomplishment {
public:
    virtual ~AccomplishmentConditional();
    virtual Difficulty GetRequiredDifficulty() const;
    virtual bool CanBeLaunched() const { return true; }

    AccomplishmentConditional(DataArray *, int);

protected:
    void Configure(DataArray *i_pConfig);
    void UpdateConditionOptionalData(
        AccomplishmentCondition &condition, DataArray *i_pConditionEntryArray
    );

    std::list<AccomplishmentCondition> m_lConditions; // 0x68
};
