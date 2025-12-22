#pragma once

#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"
class FitnessProvider : public UIListProvider, public Hmx::Object {
public:
    // Hmx::Object
    virtual ~FitnessProvider();
    virtual DataNode Handle(DataArray *, bool);

    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual Symbol DataSymbol(int) const;

    FitnessProvider();
    void UpdateList();

protected:
    std::vector<Symbol> mFitnessOptions; // 0x30
};
