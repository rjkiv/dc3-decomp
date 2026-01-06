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
    OBJ_CLASSNAME(TitleProvider)
    OBJ_SET_TYPE(TitleProvider)
    virtual DataNode Handle(DataArray *, bool);
    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual RndMat *Mat(int, int, UIListMesh *) const;
    virtual Symbol DataSymbol(int) const;
    virtual int NumData() const { return mItems.size(); }

    void UpdateList(bool);
    NEW_OBJ(TitleProvider)

protected:
    std::vector<Symbol> mItems; // 0x30
};
