#pragma once
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Mat.h"
#include "stl/_vector.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class CharacterProvider : public UIListProvider, public Hmx::Object {
public:
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual RndMat *Mat(int, int, UIListMesh *) const;
    virtual Symbol DataSymbol(int) const;
    virtual bool CanSelect(int) const;
    virtual int DataIndex(Symbol) const;

    CharacterProvider();
    bool IsCharacterAvailable(Symbol) const;
    String GetColorName() const;
    RndMat *GetMatForCharacter(Symbol) const;
    Symbol GetRandomAvailableCharacter() const;
    void UpdateList();

    int unk30;
    std::vector<Symbol> mCharacters; // 0x34
    ObjectDir *unk40; // not sure if ObjectDir
};
