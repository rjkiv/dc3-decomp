#include "WeightInput.h"

#include "HamProfile.h"
#include "ui/UIListLabel.h"
#include "lazer/meta_ham/ProfileMgr.h"

BEGIN_HANDLERS(WeightInputProvider)
    HANDLE_EXPR(get_weight, GetWeight(_msg->Int(2)))
    HANDLE_EXPR(get_index_for_weight, GetIndexForWeight(_msg->Float(2)))
    HANDLE_EXPR(get_kg_for_pounds, GetKgForPounds(_msg->Float(2)))
    HANDLE_EXPR(get_pounds_for_kgs, GetPoundsForKgs(_msg->Float(2)))
END_HANDLERS

WeightInputProvider::WeightInputProvider() {
    SetName("weight_input_provider", ObjectDir::Main());
}

int WeightInputProvider::GetIndexForWeight(float f1) const {
    for (int i = 0; i < NumData(); i++) {
        if (GetWeight(i) == f1) {
            return i;
        }
    }
    return 0;
}

float WeightInputProvider::GetWeight(int i_iIndex) const {
    MILO_ASSERT((0) <= (i_iIndex) && (i_iIndex) < (NumData()), 0x76);
    if (TheProfileMgr.GetUnk4c() != 0) {
        return i_iIndex * 5.0f + 45.0f;
    } else {
        return i_iIndex * 2.5f + 20.0f;
    }
}

float WeightInputProvider::GetPoundsForKgs(float kgs) const {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0xa7);

    float result = 45.0f;
    float pounds = pProfile->GetPoundsFromKgs(kgs);
    if (pounds > 45.0f) {
        for (int i = 0; i < 80; i++) {
            result = i * 5.0f + 45.0f;
            if (fabs(pounds - result) <= 2.5f) {
                return result;
            }
        }
        result = 440.0;
    }
    return result;
}

void WeightInputProvider::Text(
    int i1, int i2, UIListLabel *listlabel, UILabel *label
) const {
    static Symbol weight_done("weight_done");
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x22);
    int units = TheProfileMgr.GetUnk4c();
    float weight = GetWeight(i2);
    if (listlabel->Matches("label")) {
        if (units == 0) {
            static Symbol weight_pounds("weight_pounds");
            label->SetTokenFmt(weight_pounds, weight);
        } else {
            static Symbol weight_kgs("weight_kgs");
            label->SetTokenFmt(weight_kgs, weight);
        }
    } else if (listlabel->Matches("checkbox")) {
        float pounds = pProfile->GetFitnessPounds();
        if (units == 1) {
            pounds = GetKgForPounds(pounds);
        }
        if (pounds == weight) {
            label->SetIcon('b');
        } else {
            label->SetTextToken(gNullStr);
        }
    }
}

WeightInputPanel::WeightInputPanel() {};

BEGIN_HANDLERS(WeightInputPanel)
    HANDLE_ACTION(set_preferred_units, SetPreferredUnits(_msg->Sym(2)))
    HANDLE_EXPR(get_preferred_units, GetPreferredUnits())
    HANDLE_EXPR(get_weight, GetWeight())
    HANDLE_ACTION(set_weight, SetWeight(_msg->Float(2)))
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS

void WeightInputPanel::SetWeight(float weight) {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    if (!pProfile) {
        MILO_ASSERT(pProfile, 0xec);
    }
    if (TheProfileMgr.GetUnk4c() == 1) {
        weight = mWeightInputProvider.GetPoundsForKgs(weight);
    }
    pProfile->SetFitnessPounds(weight);
}

float WeightInputPanel::GetWeight() {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0xdc);
    float weight = pProfile->GetFitnessPounds();

    if (TheProfileMgr.GetUnk4c() == 1) {
        weight = mWeightInputProvider.GetKgForPounds(weight);
    }

    return weight;
}

Symbol WeightInputPanel::GetPreferredUnits() {
    static Symbol pounds("pounds");
    static Symbol kilograms("kilograms");
    if (TheProfileMgr.GetUnk4c() == 1) {
        return kilograms;
    }
    return pounds;
}

void WeightInputPanel::SetPreferredUnits(Symbol units) {
    static Symbol pounds("pounds");
    TheProfileMgr.SetGlobalOptionsDirty(true);
    int i;
    if (units == pounds)
        i = 1;
    TheProfileMgr.SetUnk4c(i);
}
