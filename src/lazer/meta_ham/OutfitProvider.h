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
    virtual int NumData() const { return m_vOutfits.size(); }

    Symbol GetRandomAvailableOutfit() const;
    void UpdateList();
    void SetPlayerIndex(int player) { mPlayerIndex = player; }

private:
    int mPlayerIndex; // 0x30
    std::vector<Symbol> m_vOutfits; // 0x34
};
