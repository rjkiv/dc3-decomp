#include "meta_ham/CrewProvider.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/ProfileMgr.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/Symbol.h"

CrewProvider::CrewProvider() : unk30(0), unk34(0) {}

CrewProvider::~CrewProvider() {}

int CrewProvider::NumData() const { return mCrews.size(); }

Symbol CrewProvider::DataSymbol(int idx) const {
    MILO_ASSERT_RANGE(idx, 0, mCrews.size(), 0xd3);
    return mCrews[idx];
}

int CrewProvider::DataIndex(Symbol s) const {
    for (int i = 0; i < mCrews.size(); i++) {
        if (mCrews[i] == s)
            return i;
    }
    return -1;
}

bool CrewProvider::IsCrewAvailable(Symbol s) const {
    static Symbol random_crew("random_crew");
    if (s == random_crew) {
        return true;
    } else {
        MetaPerformer *performer = MetaPerformer::Current();
        MILO_ASSERT(performer, 0xe1);
        int index = unk30 == 0;
        HamPlayerData *pOtherPlayerData = TheGameData->Player(index);
        MILO_ASSERT(pOtherPlayerData, 0xe5);
        if (!TheGameMode->InMode("dance_battle", true)) {
            const DataNode *prop = TheHamProvider->Property("is_in_party_mode", true);
            if (prop->Int() == 0 && !TheGameData->IsSkeletonPresent(index))
                return true;
        }
        if (pOtherPlayerData->Crew() != s)
            return true;
    }
}

bool CrewProvider::CanSelect(int idx) const {
    MILO_ASSERT_RANGE(idx, 0, mCrews.size(), 0xd4);
    if (TheProfileMgr.IsContentUnlocked(mCrews[idx].Str()))
        return IsCrewAvailable(mCrews[idx]);
    return false;
}

void CrewProvider::UpdateList() {
    mCrews.clear();
    DataArray *crewArray = SystemConfig()->FindArray("crews", false);
    if (crewArray) {
        static Symbol random_crew("random_crew");
        mCrews.push_back(random_crew);
        int size = crewArray->Size();
        for (int i = 1; i < size; i++) {
            DataArray *arr = crewArray->Node(i).Array(crewArray);
            static Symbol always_hidden("always_hidden");
            bool checkAlwaysHidden = false;
            arr->FindData(always_hidden, checkAlwaysHidden, false);
            if (!checkAlwaysHidden) {
                static Symbol is_hidden("is_hidden");
                bool checkIsHidden = false;
                arr->FindData(is_hidden, checkIsHidden, false);
                Symbol crew = arr->Sym(0);
                if (TheProfileMgr.IsContentUnlocked(crew) || !checkIsHidden) {
                    mCrews.push_back(crew);
                }
            }
        }
    }
}
