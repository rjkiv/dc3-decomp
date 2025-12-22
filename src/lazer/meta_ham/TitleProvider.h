#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class TitleProvider : public UIListProvider, public Hmx::Object {
public:
    // Hmx::Object
    virtual ~TitleProvider();
    virtual DataNode Handle(DataArray *, bool);

    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual RndMat *Mat(int, int, UIListMesh *) const;
    virtual Symbol DataSymbol(int) const;

    TitleProvider();
    void UpdateList(bool);

protected:
    std::vector<Symbol> mItems; // 0x30
};
