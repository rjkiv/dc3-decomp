#pragma once
#include "obj/Object.h"
#include "meta/MetaMusicScene.h"
#include <map>

class MetaMusicManager : public Hmx::Object {
public:
    MetaMusicManager(DataArray *);
    virtual ~MetaMusicManager();
    virtual DataNode Handle(DataArray *, bool);

    Symbol GetSceneForScreen(Symbol i_symScreen) const;
    MetaMusicScene *GetScene(Symbol i_symScene) const;

private:
    void Cleanup();
    void Init(DataArray *i_pConfig);
    bool IsScreenInSceneMap(Symbol i_symScreen) const;
    void ConfigureMetaMusicSceneData(DataArray *i_pConfig);

    bool HasScene(Symbol i_symScene) const { return GetScene(i_symScene); }

    std::map<Symbol, MetaMusicScene *> m_mapScenes; // 0x2c
    std::map<Symbol, Symbol> m_mapScreenToScene; // 0x44
};

extern MetaMusicManager *TheMetaMusicManager;
