#pragma once
#include "obj/Data.h"
#include "utl/Symbol.h"
#include <list>

class MetaMusicScene {
private:
    Symbol m_symName; // 0x4
    std::list<Symbol> m_lScreens; // 0x8
    DataArray *m_FxConfig; // 0x10
    void Configure(DataArray *);

public:
    MetaMusicScene(DataArray *);
    virtual ~MetaMusicScene();

    Symbol GetName() const;
    const std::list<Symbol> &GetScreenList() const;
    DataArray *GetFxConfig() { return m_FxConfig; }
};
