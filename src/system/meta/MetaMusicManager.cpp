#include "meta/MetaMusicManager.h"
#include "obj/Dir.h"
#include "utl/Symbol.h"

MetaMusicManager *TheMetaMusicManager;

void MetaMusicManager::Cleanup() {
    for (std::map<Symbol, MetaMusicScene *>::iterator it = m_mapScenes.begin();
         it != m_mapScenes.end();
         it++) {
        delete it->second;
        it->second = 0;
    }
    m_mapScenes.clear();
    m_mapScreenToScene.clear();
}

MetaMusicManager::~MetaMusicManager() {
    TheMetaMusicManager = 0;
    Cleanup();
}

MetaMusicManager::MetaMusicManager(DataArray *da) {
    MILO_ASSERT(!TheMetaMusicManager, 0x13);
    TheMetaMusicManager = this;
    SetName("metamusic_mgr", ObjectDir::Main());
    Init(da);
}

void MetaMusicManager::ConfigureMetaMusicSceneData(DataArray *i_pConfig) {
    MILO_ASSERT(m_mapScenes.empty(), 0x38);
    for (int i = 1; i < i_pConfig->Size(); i++) {
        DataArray *pSceneArray = i_pConfig->Array(i);
        MILO_ASSERT(pSceneArray, 0x3E);
        MetaMusicScene *pScene = new MetaMusicScene(pSceneArray);
        MILO_ASSERT(pScene, 0x42);
        Symbol symScene = pScene->GetName();
        if (HasScene(symScene)) {
            MILO_NOTIFY("%s scene already exists, skipping", symScene.Str());
            delete pScene;
        } else {
            m_mapScenes[symScene] = pScene;
            const std::list<Symbol> &screenlist = pScene->GetScreenList();
            FOREACH (it, screenlist) {
                Symbol symScreen = *it;
                if (IsScreenInSceneMap(symScreen)) {
                    MILO_NOTIFY(
                        "%s screen already referenced in a scene, skipping",
                        symScreen.Str()
                    );
                } else {
                    m_mapScreenToScene[symScreen] = symScene;
                }
            }
        }
    }
}

void MetaMusicManager::Init(DataArray *da) {
    ConfigureMetaMusicSceneData(da->FindArray("scenes"));
}

Symbol MetaMusicManager::GetSceneForScreen(Symbol sym) const {
    auto s = m_mapScreenToScene.find(sym);
    if (s != m_mapScreenToScene.end()) {
        return s->second;
    }
    return gNullStr;
}

bool MetaMusicManager::IsScreenInSceneMap(Symbol sym) const {
    if (GetSceneForScreen(sym) == gNullStr) {
        return false;
    }
    return true;
}
