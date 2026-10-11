#include "meta_ham/ChooseModeProvider.h"
#include "game/GameMode.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Mat.h"
#include "ui/UILabel.h"
#include "ui/UIListLabel.h"
#include "ui/UIListMesh.h"
#include "utl/Symbol.h"

BEGIN_HANDLERS(ChooseModeProvider)
    HANDLE_ACTION(update_list, UpdateList(_msg->Int(2)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

ChooseModeProvider::~ChooseModeProvider() {}

void ChooseModeProvider::Text(
    int, int i_iData, UIListLabel *listLabel, UILabel *uiLabel
) const {
    MILO_ASSERT(i_iData < NumData(), 0x4e);
    Symbol dataSym = DataSymbol(i_iData);
    if (listLabel->Matches("label")) {
        uiLabel->SetTextToken(dataSym);
    } else {
        uiLabel->SetTextToken(listLabel->GetDefaultText());
    }
}

Symbol ChooseModeProvider::DataSymbol(int i_iData) const {
    MILO_ASSERT_RANGE(i_iData, 0, NumData(), 0xaf);
    return m_vEntries[i_iData];
}

void ChooseModeProvider::UpdateList(bool b) {
    m_vEntries.clear();
    static Symbol perform("perform");
    static Symbol practice("practice");
    static Symbol dance_battle("dance_battle");
    static Symbol custom_party("custom_party");
    static Symbol crew_showdown("crew_showdown");
    static Symbol rtnbldrproto("rtnbldrproto");
    static Symbol namethatdance("namethatdance");
    static Symbol cascade("cascade");
    static Symbol concentration("concentration");
    static Symbol rhythm_battle("rhythm_battle");
    static Symbol holla_back("holla_back");
    static Symbol holla_back_70s_craze("holla_back_70s_craze");
    static Symbol perform_legacy("perform_legacy");
    static Symbol leaderboards("leaderboards");
    static Symbol bustamove("bustamove");
    static Symbol mind_control("mind_control");

    m_vEntries.push_back(perform);
    m_vEntries.push_back(practice);
    if (!TheGameMode->InMode("campaign", true)) {
        m_vEntries.push_back(dance_battle);
        m_vEntries.push_back(custom_party);
        m_vEntries.push_back(crew_showdown);
        if (b) {
            m_vEntries.push_back(perform_legacy);
            m_vEntries.push_back(rtnbldrproto);
            m_vEntries.push_back(namethatdance);
            m_vEntries.push_back(concentration);
            m_vEntries.push_back(rhythm_battle);
            m_vEntries.push_back(holla_back_70s_craze);
            m_vEntries.push_back(dance_battle);
            m_vEntries.push_back(bustamove);
            m_vEntries.push_back(mind_control);
        }
    }
}

RndMat *ChooseModeProvider::Mat(int, int i_iData, UIListMesh *mesh) const {
    MILO_ASSERT_RANGE(i_iData, 0, NumData(), 0x5d);
    static Symbol perform("perform");
    static Symbol perform_legacy("perform_legacy");
    static Symbol practice("practice");
    static Symbol dance_battle("dance_battle");
    static Symbol custom_party("custom_party");
    static Symbol crew_showdown("crew_showdown");
    static Symbol start_the_party("start_the_party");
    static Symbol rtnbldrproto("rtnbldrproto");
    static Symbol namethatdance("namethatdance");
    static Symbol concentration("concentration");
    static Symbol rhythm_battle("rhythm_battle");
    static Symbol holla_back("holla_back");
    static Symbol holla_back_70s_craze("holla_back_70s_craze");
    static Symbol bustamove("bustamove");
    Symbol dataSym = DataSymbol(i_iData);
    if (mesh->Matches("icon_1p")) {
        if (dataSym != practice) {
            return nullptr;
        }
    } else if (mesh->Matches("icon_1p_plus")) {
        if (dataSym == custom_party) {
            return mesh->DefaultMat();
        } else if (dataSym != start_the_party) {
            return nullptr;
        }
    } else if (mesh->Matches("icon_2p")) {
        if (dataSym == namethatdance || dataSym == dance_battle
            || dataSym == concentration || dataSym == rhythm_battle) {
            return mesh->DefaultMat();
        } else if (dataSym != bustamove) {
            return nullptr;
        }
    } else if (mesh->Matches("icon_1por2p")) {
        if (dataSym == perform || dataSym == perform_legacy || dataSym == rtnbldrproto
            || dataSym == holla_back) {
            return mesh->DefaultMat();
        } else if (dataSym != holla_back_70s_craze) {
            return nullptr;
        }
    } else if (mesh->Matches("icon_2p_plus")) {
        if (dataSym != crew_showdown) {
            return nullptr;
        }
    }
    return mesh->DefaultMat();
}
