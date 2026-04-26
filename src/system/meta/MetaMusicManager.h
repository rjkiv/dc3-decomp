#pragma once
#include "obj/Object.h"
#include "meta/MetaMusicScene.h"
#include <map>

class MetaMusicManager : public Hmx::Object {
public:
    MetaMusicManager(DataArray *);
    virtual ~MetaMusicManager();
    virtual DataNode Handle(DataArray *, bool);

    Symbol GetSceneForScreen(Symbol) const;
    MetaMusicScene *GetScene(Symbol) const;

    bool SceneExists(Symbol s) const { return GetScene(s); }
    bool SceneForScreenExists(Symbol s) const { return GetSceneForScreen(s) != gNullStr; }

private:
    void Init(DataArray *);
    void Cleanup();
    bool IsScreenInSceneMap(Symbol) const;
    void ConfigureMetaMusicSceneData(DataArray *);

    std::map<Symbol, MetaMusicScene *> m_mapScenes; // 0x1c
    std::map<Symbol, Symbol> m_mapScreenToScene; // 0x34
};

extern MetaMusicManager *TheMetaMusicManager;
