#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Mat.h"
#include "stl/_vector.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "ui/UIListMesh.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class ChooseModeProvider : public UIListProvider, public Hmx::Object {
public:
    // Hmx::Object
    virtual ~ChooseModeProvider();
    OBJ_CLASSNAME(ChooseModeProvider)
    virtual DataNode Handle(DataArray *, bool);
    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual RndMat *Mat(int, int, UIListMesh *) const;
    virtual Symbol DataSymbol(int) const;
    virtual int NumData() const { return mModes.size(); }

    void UpdateList(bool);
    NEW_OBJ(ChooseModeProvider)

private:
    std::vector<Symbol> mModes; // 0x30
};
