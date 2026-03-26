#pragma once
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/PanelDir.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class CrewProvider : public UIListProvider, public Hmx::Object {
public:
    CrewProvider();
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual Symbol DataSymbol(int) const;
    virtual bool CanSelect(int) const;
    virtual int DataIndex(Symbol) const;
    virtual int NumData() const { return mCrews.size(); }

    bool IsCrewAvailable(Symbol) const;
    Symbol GetRandomAvailableCrew() const;
    void UpdateList();
    void SetPanelDir(PanelDir *);
    void SetPlayer(int player) { mPlayer = player; }

private:
    int mPlayer; // 0x30
    PanelDir *mPanelDir; // 0x34
    std::vector<Symbol> mCrews; // 0x38
};
