#pragma once
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/UIListLabel.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class OutfitProvider : public UIListProvider, public Hmx::Object {
public:
    // Hmx::Object
    virtual ~OutfitProvider();

    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual Symbol DataSymbol(int) const;
    virtual bool CanSelect(int) const;
    virtual int NumData() const;

    OutfitProvider();
    Symbol GetRandomAvailableOutfit() const;
    void UpdateList();

    int unk30;
    std::vector<Symbol> mOutfits; // 0x34
};
