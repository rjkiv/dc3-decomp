#include "meta_ham/FitnessProvider.h"
#include "FitnessProvider.h"
#include "HamProfile.h"
#include "ProfileMgr.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

FitnessProvider::~FitnessProvider() {}

void FitnessProvider::Text(
    int, int i_iData, UIListLabel *uiListLabel, UILabel *uiLabel
) const {
    MILO_ASSERT(i_iData < NumData(), 0x53);
    Symbol dataSym = DataSymbol(i_iData);
    if (uiListLabel->Matches("label")) {
        uiLabel->SetTextToken(dataSym);
    } else if (uiListLabel->Matches("checkbox")) {
        HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
        if (pProfile) {
            static Symbol fitness_option("fitness_option");
            if (dataSym == fitness_option) {
                if (pProfile->InFitnessMode())
                    uiLabel->SetIcon('b');
                else
                    uiLabel->SetIcon('a');
                return;
            }
        }
        uiLabel->SetTextToken(gNullStr);
    } else {
        uiLabel->SetTextToken(uiListLabel->GetDefaultText());
    }
}

Symbol FitnessProvider::DataSymbol(int i_iData) const {
    MILO_ASSERT_RANGE(i_iData, 0, NumData(), 0x92);
    return mFitnessOptions[i_iData];
}

void FitnessProvider::UpdateList() {
    mFitnessOptions.clear();
    static Symbol fitness_option("fitness_option");
    static Symbol fitness_set_fitness_goals("fitness_set_fitness_goals");
    static Symbol fitness_body_profile("fitness_body_profile");
    static Symbol playlists("playlists");
    mFitnessOptions.push_back(fitness_option);
    mFitnessOptions.push_back(fitness_set_fitness_goals);
    mFitnessOptions.push_back(fitness_body_profile);
    mFitnessOptions.push_back(playlists);
}
BEGIN_HANDLERS(FitnessProvider)
    HANDLE_ACTION(update_list, UpdateList())
    HANDLE_SUPERCLASS(UIListProvider)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
