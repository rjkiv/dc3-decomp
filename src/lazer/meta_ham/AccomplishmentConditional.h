#pragma once
#include "hamobj/Difficulty.h"
#include "lazer/meta_ham/Accomplishment.h"
#include "obj/Data.h"
#include "utl/Symbol.h"
#include <vector>

struct AccomplishmentCondition {
    Symbol m_symCondition; // 0x0
    int m_iValue; // 0x4
    Symbol m_symValue; // 0x8
    Difficulty m_eDifficulty; // 0xc
    Symbol m_symCharacter; // 0x10
    Symbol m_symMode; // 0x14
    bool m_bNoFlashcards; // 0x18
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
