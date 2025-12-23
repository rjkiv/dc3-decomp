#pragma once
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/PanelDir.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class CrewProvider : public UIListProvider, public Hmx::Object {
public:
    virtual ~CrewProvider();
    virtual int NumData() const;
    virtual bool CanSelect(int) const;
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual int DataIndex(Symbol) const;
    virtual Symbol DataSymbol(int) const;

    CrewProvider();
    bool IsCrewAvailable(Symbol) const;
    Symbol GetRandomAvailableCrew() const;
    void UpdateList();
    void SetPanelDir(PanelDir *);

    int unk30;
    int unk34;
    std::vector<Symbol> mCrews; // 0x38
};
