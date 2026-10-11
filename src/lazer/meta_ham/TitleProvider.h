#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
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
    virtual int NumData() const { return m_vEntries.size(); }

    void UpdateList(bool);
    NEW_OBJ(TitleProvider)

private:
    std::vector<Symbol> m_vEntries; // 0x30
};
