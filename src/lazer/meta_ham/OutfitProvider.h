#pragma once
#include "obj/Object.h"
#include "ui/UIListLabel.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class OutfitProvider : public UIListProvider, public Hmx::Object {
public:
    OutfitProvider();
    // Hmx::Object
    virtual ~OutfitProvider();
    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual Symbol DataSymbol(int) const;
    virtual bool CanSelect(int) const;
    virtual int NumData() const { return mOutfits.size(); }

    Symbol GetRandomAvailableOutfit() const;
    void UpdateList();
    void SetPlayer(int player) { mPlayer = player; }

private:
    int mPlayer; // 0x30
    std::vector<Symbol> mOutfits; // 0x34
};
