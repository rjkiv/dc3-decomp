#pragma once
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class VenueProvider : public UIListProvider, public Hmx::Object {
public:
    VenueProvider();
    virtual ~VenueProvider();
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual Symbol DataSymbol(int) const;
    virtual int NumData() const;

    void UpdateList();

    void SetPlayer(int i) { mPlayer = i; }

private:
    int mPlayer; // 0x30
    std::vector<Symbol> mVenues; // 0x34
};
