#include "WeightInput.h"
#include "HamProfile.h"
#include "os/Debug.h"
#include "ui/UIListLabel.h"
#include "meta_ham/ProfileMgr.h"
#include <cmath>

// static const float sFloats[6] = { 45.0f, 440.0f, 5.0f, 20.0f, 200.0f, 2.5f };
static const float sFloats0 = 45.0f;
static const float sFloats1 = 440.0f;
static const float sFloats2 = 5.0f;
static const float sFloats3 = 20.0f;
static const float sFloats4 = 200.0f;
static const float sFloats5 = 2.5f;

#pragma region WeightInputProvider

WeightInputProvider::WeightInputProvider() {
    SetName("weight_input_provider", ObjectDir::Main());
}

void WeightInputProvider::Text(
    int i1, int data, UIListLabel *listlabel, UILabel *label
) const {
    static Symbol weight_done("weight_done");
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x22);
    int units = TheProfileMgr.GetUnk4c();
    float weight = GetWeight(data);
    if (listlabel->Matches("label")) {
        if (units == 0) {
            static Symbol weight_pounds("weight_pounds");
            label->SetTokenFmt(weight_pounds, (int)weight);
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

BEGIN_HANDLERS(WeightInputProvider)
    HANDLE_EXPR(get_weight, GetWeight(_msg->Int(2)))
    HANDLE_EXPR(get_index_for_weight, GetIndexForWeight(_msg->Float(2)))
    HANDLE_EXPR(get_kg_for_pounds, GetKgForPounds(_msg->Float(2)))
    HANDLE_EXPR(get_pounds_for_kgs, GetPoundsForKgs(_msg->Float(2)))
END_HANDLERS

int WeightInputProvider::GetIndexForWeight(float f1) const {
    int idx = 0;
    for (int i = 0; i < NumData(); i++) {
        if (GetWeight(i) == f1) {
            idx = i;
            break;
        }
    }
    return idx;
}

float WeightInputProvider::GetWeight(int i_iIndex) const {
    MILO_ASSERT_RANGE(i_iIndex, 0, NumData(), 0x76);
    if (TheProfileMgr.GetUnk4c() == 0) {
        return i_iIndex * sFloats2 + sFloats0;
    } else {
        return i_iIndex * sFloats5 + sFloats3;
    }
}

float WeightInputProvider::GetPoundsForKgs(float kgs) const {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0xa7);

    float result;
    float pounds = pProfile->GetPoundsFromKgs(kgs);
    if (pounds < sFloats0) {
        return sFloats0;
    }
    for (int i = 0; i < 80; i++) {
        result = i * sFloats2 + sFloats0;
        if (fabs(pounds - result) <= sFloats5) {
            return result;
        }
    }
    return sFloats1;
}

float WeightInputProvider::GetKgForPounds(float lbs) const {
    HamProfile *pProfile = TheProfileMgr.GetActiveProfile(true);
    MILO_ASSERT(pProfile, 0x89);

    float result;
    float kgs = pProfile->GetKgFromPounds(lbs);
    if (kgs < sFloats3) {
        return sFloats3;
    }

    for (int i = 0; i < 73; i++) {
        result = sFloats3 + (i * sFloats5);
        if (fabs(kgs - result) <= 1.25f) {
            return result;
        }
    }

    return sFloats4;
}

#pragma endregion
#pragma region WeightInputPanel

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
    Symbol units = pounds;
    if (TheProfileMgr.GetUnk4c() == 1) {
        units = kilograms;
    }
    return units;
}

void WeightInputPanel::SetPreferredUnits(Symbol units) {
    static Symbol pounds("pounds");
    if (units == pounds) {
        TheProfileMgr.SetUnk4c(0);
        TheProfileMgr.SetGlobalOptionsDirty(true);
    } else {
        TheProfileMgr.SetUnk4c(1);
        TheProfileMgr.SetGlobalOptionsDirty(true);
    }
}
