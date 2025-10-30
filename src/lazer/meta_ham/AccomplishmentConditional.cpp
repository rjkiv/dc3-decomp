#include "lazer/meta_ham/AccomplishmentConditional.h"
#include "AccomplishmentConditional.h"
#include "hamobj/Difficulty.h"
#include "lazer/meta_ham/Accomplishment.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

AccomplishmentConditional::AccomplishmentConditional(DataArray *d, int i)
    : Accomplishment(d, i) {
    Configure(d);
}

AccomplishmentConditional::~AccomplishmentConditional() {}

Difficulty AccomplishmentConditional::GetRequiredDifficulty() const {
    MILO_ASSERT(!m_lConditions.empty(), 0x7b);

    Difficulty requiredDifficulty = (Difficulty)3;

    return requiredDifficulty;
}

void AccomplishmentConditional::Configure(DataArray *i_pConfig) {
    MILO_ASSERT(i_pConfig, 0x48);

    static Symbol conditions("conditions");
    DataArray *pConditionArray = i_pConfig->FindArray(conditions);
    if (pConditionArray) {
        for (int i = 1; i < pConditionArray->Size(); i++) {
            DataArray *pConditionEntryArray = pConditionArray->Node(i).Array();
            MILO_ASSERT(pConditionEntryArray, 0x54);

            AccomplishmentCondition condition;

            if (pConditionEntryArray->Size() >= 2) {
                if (pConditionEntryArray->Node(1).Type() == kDataInt) {
                } else if (pConditionEntryArray->Node(1).Type() == kDataSymbol) {
                } else {
                    MILO_ASSERT(false, 0x66);
                }
                UpdateConditionOptionalData(condition, pConditionEntryArray);
            }
            m_lConditions.push_back(condition);
        }
    }
}

void AccomplishmentConditional::UpdateConditionOptionalData(
    AccomplishmentCondition &condition, DataArray *i_pConditionEntryArray
) {
    MILO_ASSERT(i_pConditionEntryArray->Size() >= 2, 0x1c);

    for (int i = 2; i < i_pConditionEntryArray->Size(); i++) {
        static Symbol character("character");
        static Symbol instrument("instrument");
        static Symbol difficulty("difficulty");
        static Symbol no_flashcards("no_flashcards");
        static Symbol mode("mode");

        DataArray *pEntry = i_pConditionEntryArray->Node(i).Array();
        MILO_ASSERT(pEntry, 0x28);
        Symbol name = Accomplishment::GetName();
        if (pEntry->Size() != 2) {
            MILO_FAIL("Invalid condition entry in %s.", name);
        }

        Symbol s = pEntry->Node(0).Sym();
        if (s == difficulty) {
            condition.mDifficulty = (Difficulty)pEntry->Node(1).Int();
        } else if (s == character) {
            condition.mCharacter = pEntry->Node(1).Sym();
        } else if (s == mode) {
            condition.mMode = pEntry->Node(1).Sym();
        } else if (s == no_flashcards) {
            condition.mFlashcards = pEntry->Node(1).Int();
        } else {
            MILO_ASSERT(false, 0x40);
        }
    }
}
