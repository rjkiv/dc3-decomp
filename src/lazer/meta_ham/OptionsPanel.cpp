#include "meta_ham/OptionsPanel.h"
#include "OptionsPanel.h"
#include "ProfileMgr.h"
#include "meta/StoreOffer.h"
#include "meta/StorePurchaser.h"
#include "meta_ham/HamPanel.h"
#include "net_ham/RCJobDingo.h"
#include "net_ham/RockCentral.h"
#include "net_ham/TokenJobs.h"
#include "net_ham/WebLinkJobs.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/JobMgr.h"
#include "utl/Symbol.h"
#include "xdk/xapilibi/xbox.h"

OptionsPanel::OptionsPanel() : unk3c(), unk40(), unk50(), unk58() {}

OptionsPanel::~OptionsPanel() {}

bool OptionsPanel::OnRedeemToken(int i, char const *str) {
    unk3c = new RedeemTokenJob(this, i, str);
    TheRockCentral.ManageJob(unk3c);
    return true;
}

void OptionsPanel::OnPurchaseOfferByOfferString(int i, char const *c) {
    unsigned long long ID = StorePurchaseable::OfferStringToID(c);
    unk40 = new XboxPurchaser(i, ID, 0, 0, gNullStr, 0);
    unk48 = ID;
    unk50 = TheProfileMgr.GetProfileFromPad(i);
    unk40->Initiate();
}

bool OptionsPanel::OnGetLinkingCode(int i) {
    unk58 = new GetWebLinkCodeJob(this, i);
    TheRockCentral.ManageJob(unk58);
    return true;
}

void OptionsPanel::OnXboxTokenRedemption(int i) {
    MILO_LOG("XShowTokenRedemptionUI returned %d\n", XShowTokenRedemptionUI(i));
}

DataNode OptionsPanel::OnMsg(SingleItemEnumCompleteMsg const &msg) {
    if (msg.Success()) {
        if (msg.HasOfferID())
            msg.OfferID();
    }
    return 0;
}

BEGIN_HANDLERS(OptionsPanel)
    HANDLE_EXPR(redeem_token, OnRedeemToken(_msg->Int(2), _msg->Str(3)))
    HANDLE_ACTION(
        purchase_offer_by_offer_string,
        OnPurchaseOfferByOfferString(_msg->Int(2), _msg->Str(3))
    )
    HANDLE_EXPR(get_linking_code, OnGetLinkingCode(_msg->Int(2)))
    HANDLE_ACTION(xbox_token_redemption, OnXboxTokenRedemption(_msg->Int(2)))
    HANDLE_MESSAGE(RCJobCompleteMsg)
    HANDLE_MESSAGE(SingleItemEnumCompleteMsg)
    HANDLE_SUPERCLASS(HamPanel)
END_HANDLERS
