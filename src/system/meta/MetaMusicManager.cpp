#include "meta/MetaMusicManager.h"
#include "obj/Dir.h"

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

void MetaMusicManager::ConfigureMetaMusicSceneData(DataArray *da) {
    MILO_ASSERT(m_mapScenes.empty(), 0x38);
    for (int i = 1; i < da->Size(); i++) {
        DataArray *pSceneArray = da->Array(i);
        MILO_ASSERT(pSceneArray, 0x3E);
        MetaMusicScene *pScene = new MetaMusicScene(pSceneArray);
        MILO_ASSERT(pScene, 0x42);
        Symbol scene_name = pScene->GetName();
        if (SceneExists(scene_name)) {
            MILO_NOTIFY("%s scene already exists, skipping", scene_name.Str());
            delete pScene;
        } else {
            m_mapScenes[scene_name] = pScene;
            const std::list<Symbol> &screenlist = pScene->GetScreenList();
            for (std::list<Symbol>::const_iterator it = screenlist.begin();
                 it != screenlist.end();
                 it++) {
                Symbol theSymbol = *it;
                if (IsScreenInSceneMap(theSymbol)) {
                    MILO_NOTIFY(
                        "%s screen already referenced in a scene, skipping",
                        theSymbol.Str()
                    );
                } else {
                    m_mapScreenToScene[theSymbol] = scene_name;
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