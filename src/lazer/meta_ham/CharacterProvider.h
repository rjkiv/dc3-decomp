#pragma once
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Mat.h"
#include "ui/PanelDir.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class CharacterProvider : public UIListProvider, public Hmx::Object {
public:
    CharacterProvider();
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual RndMat *Mat(int, int, UIListMesh *) const;
    virtual Symbol DataSymbol(int) const;
    virtual bool CanSelect(int) const;
    virtual int DataIndex(Symbol) const;
    virtual int NumData() const { return mCharacters.size(); }

    bool IsCharacterAvailable(Symbol) const;
    String GetColorName() const;
    RndMat *GetMatForCharacter(Symbol) const;
    Symbol GetRandomAvailableCharacter() const;
    void UpdateList();
    void SetPanelDir(PanelDir *);
    void SetPlayer(int player) { mPlayer = player; }

private:
    int mPlayer; // 0x30
    std::vector<Symbol> mCharacters; // 0x34
    PanelDir *mPanelDir; // 0x40
};
