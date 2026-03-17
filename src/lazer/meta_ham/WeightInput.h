#pragma once

#include "HamPanel.h"
#include "obj/Object.h"
#include "system/world/Instance.h"
#include "ProfileMgr.h"
#include "system/ui/UIListProvider.h"

class WeightInputProvider : public UIListProvider, public Hmx::Object {
public:
    WeightInputProvider();
    virtual ~WeightInputProvider() {}
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual int NumData() const { return TheProfileMgr.GetUnk4c() ? 73 : 80; };
    virtual DataNode Handle(DataArray *, bool);

    float GetWeight(int) const;
    int GetIndexForWeight(float) const;
    float GetKgForPounds(float) const;
    float GetPoundsForKgs(float) const;
};

class WeightInputPanel : public HamPanel {
public:
    WeightInputPanel() {}
    virtual ~WeightInputPanel() {}
    OBJ_CLASSNAME(WeightInputPanel)
    OBJ_SET_TYPE(WeightInputPanel)
    virtual DataNode Handle(DataArray *, bool);

    NEW_OBJ(WeightInputPanel)

    void SetWeight(float);
    float GetWeight();
    Symbol GetPreferredUnits();
    void SetPreferredUnits(Symbol);

private:
    WeightInputProvider mWeightInputProvider; // 0x3c
};
