#pragma once

#include "HamPanel.h"
#include "system/world/Instance.h"
#include "ProfileMgr.h"
#include "system/ui/UIListProvider.h"


class WeightInputProvider : public UIListProvider, Hmx::Object {
public:
    WeightInputProvider();
    virtual ~WeightInputProvider();
    virtual int NumData() const { return ((TheProfileMgr.GetUnk4c() != 0) & -7) + 80; }; // FIXME: needs to use subfic inst but uses subic
    virtual DataNode Handle(DataArray *, bool);
    virtual void Text(int, int, UIListLabel *, UILabel *) const;

    float GetWeight(int) const;
    int GetIndexForWeight(float) const;
    float GetKgForPounds(float) const;
    float GetPoundsForKgs(float) const;

};

class WeightInputPanel : public HamPanel {
public:
    WeightInputPanel();
    ~WeightInputPanel();
    virtual DataNode Handle(DataArray *, bool);

    void SetWeight(float);
    float GetWeight();
    Symbol GetPreferredUnits();
    void SetPreferredUnits(Symbol);

    WeightInputProvider mWeightInputProvider; // 0x3c

};