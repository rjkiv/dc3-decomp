#pragma once
#include "obj/Object.h"
#include "stl/_vector.h"
#include "ui/UIListProvider.h"
#include "utl/Symbol.h"

class MainMenuProvider : public UIListProvider, public Hmx::Object {
public:
    // Hmx::Object
    virtual ~MainMenuProvider();

    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual Symbol DataSymbol(int) const;
    virtual int DataIndex(Symbol) const;
    virtual int NumData() const;

    MainMenuProvider();
    void UpdateList(UIListProvider *);

protected:
    std::vector<Symbol> mItems; // 0x30

private:
    bool HasNewDLC() const;
};
