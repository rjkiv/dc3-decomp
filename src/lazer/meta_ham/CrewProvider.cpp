#include "meta_ham/CrewProvider.h"
#include "math/Rand.h"
#include "meta_ham/AppLabel.h"
#include "meta_ham/MetaPerformer.h"
#include "meta_ham/ProfileMgr.h"
#include "flow/PropertyEventProvider.h"
#include "game/GameMode.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamPlayerData.h"
#include "obj/Data.h"
#include "os/Debug.h"
#include "os/System.h"
#include "ui/PanelDir.h"
#include "utl/Symbol.h"
#include <cstdio>

CrewProvider::CrewProvider() : mPlayerIndex(0), mPanelDir(0) {}

void CrewProvider::Text(int, int data, UIListLabel *slot, UILabel *label) const {
    MILO_ASSERT_RANGE(data, 0, mCrews.size(), 0x69);
    AppLabel *app_label = dynamic_cast<AppLabel *>(label);
    MILO_ASSERT(app_label, 0x6C);
    Symbol crewSym = mCrews[data];
    if (slot->Matches("crew")) {
        static Symbol player_present("player_present");
        int otherPlayer = !mPlayerIndex;
        if (TheGameData->Player(mPlayerIndex)->Provider()->Property(player_present)->Int()
                != 1
            && TheGameData->Player(otherPlayer)->Provider()->Property(player_present)->Int()
                != 0) {
            char buffer[32];
            buffer[0] = 0;
            memset(&buffer[1], 0, 31);
            sprintf(buffer, "%s%s", crewSym.Str(), "_title");
            app_label->SetTextToken(buffer);
        } else {
            app_label->SetTextToken(crewSym);
        }
    } else if (slot->Matches("new")) {
        HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
        if (profile) {
            app_label->SetNew(profile->IsContentNew(crewSym));
        } else {
            app_label->SetTextToken(gNullStr);
        }
    } else if (slot->Matches("lock")) {
        if (!TheProfileMgr.IsContentUnlocked(crewSym)) {
            app_label->SetIcon('B');
        } else {
            app_label->SetTextToken(gNullStr);
        }
    } else if (slot->Matches("taken")) {
        if (!IsCrewAvailable(crewSym)) {
            app_label->SetIcon('i');
        } else {
            app_label->SetTextToken(gNullStr);
        }
    }
}

Symbol CrewProvider::DataSymbol(int idx) const {
    MILO_ASSERT_RANGE(idx, 0, mCrews.size(), 0xd3);
    return mCrews[idx];
}

bool CrewProvider::CanSelect(int idx) const {
    MILO_ASSERT_RANGE(idx, 0, NumData(), 0xf4);
    Symbol dataSym = DataSymbol(idx);
    if (!TheProfileMgr.IsContentUnlocked(dataSym))
        return false;
    else
        return IsCrewAvailable(dataSym) != false;
}

int CrewProvider::DataIndex(Symbol s) const {
    for (int i = 0; i < mCrews.size(); i++) {
        if (mCrews[i] == s)
            return i;
    }
    return -1;
}

bool CrewProvider::IsCrewAvailable(Symbol crewSym) const {
    static Symbol random_crew("random_crew");
    if (crewSym == random_crew) {
        return true;
    } else {
        MetaPerformer *performer = MetaPerformer::Current();
        MILO_ASSERT(performer, 0xe1);
        int index = mPlayerIndex == 0;
        HamPlayerData *pOtherPlayerData = TheGameData->Player(index);
        MILO_ASSERT(pOtherPlayerData, 0xe5);
        if (!TheGameMode->InMode("dance_battle", true)) {
            const DataNode *prop = TheHamProvider->Property("is_in_party_mode");
            if (prop->Int() == 0 && !TheGameData->IsSkeletonPresent(index))
                return true;
        }
        bool ret = false;
        if (pOtherPlayerData->Crew() != crewSym)
            ret = true;
        return ret;
    }
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

Symbol CrewProvider::GetRandomAvailableCrew() const {
    static Symbol random_crew("random_crew");
    std::vector<Symbol> vCrews;
    int num = NumData();
    for (int i = 0; i < num; i++) {
        Symbol dataSym = DataSymbol(i);
        if (CanSelect(i) && dataSym != random_crew) {
            vCrews.push_back(dataSym);
        }
    }
    return vCrews[RandomInt(0, vCrews.size())];
}

void CrewProvider::SetPanelDir(PanelDir *dir) { mPanelDir = dir; }
