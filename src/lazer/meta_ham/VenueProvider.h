#pragma once
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class VenueProvider : public UIListProvider, public Hmx::Object {
public:
    virtual Symbol DataSymbol(int) const;
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual int NumData() const;

    VenueProvider();
    void UpdateList();

    int unk30;
    std::vector<Symbol> mVenues; // 0x34
};
