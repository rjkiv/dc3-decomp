#pragma once
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Mat.h"
#include "stl/_vector.h"
#include "ui/PanelDir.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class CharacterProvider : public UIListProvider, public Hmx::Object {
public:
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual RndMat *Mat(int, int, UIListMesh *) const;
    virtual Symbol DataSymbol(int) const;
    virtual bool CanSelect(int) const;
    virtual int DataIndex(Symbol) const;
    virtual int NumData() const;

    CharacterProvider();
    bool IsCharacterAvailable(Symbol) const;
    String GetColorName() const;
    RndMat *GetMatForCharacter(Symbol) const;
    Symbol GetRandomAvailableCharacter() const;
    void UpdateList();
    void SetPanelDir(PanelDir *);
    void SetPlayerIndex(int i) { unk30 = i; }

    int unk30;
    std::vector<Symbol> mCharacters; // 0x34
    ObjectDir *unk40; // not sure if ObjectDir
};
