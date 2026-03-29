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
#include "os/PlatformMgr.h"
#include "ui/UIPanel.h"
#include "utl/JobMgr.h"
#include "utl/Symbol.h"
#include "xdk/xapilibi/xbox.h"

OptionsPanel::OptionsPanel() : unk48(0), unk50(), unk58() {
    unk3c = nullptr;
    unk40 = nullptr;
}

OptionsPanel::~OptionsPanel() {}

void OptionsPanel::Poll() {
    UIPanel::Poll();
    if (unk40) {
        unk40->Poll();
        if (!unk40->IsPurchasing()) {
            if (unk40->IsSuccess() && !unk40->PurchaseMade() && unk40->NeedsEnum()
                && unk50) {
                ThePlatformMgr.QueueEnumJob(new PostPurchaseEnumJob(
                    this, unk50->GetPadNum(), unk48, unk40->unk4, unk40->unk8
                ));
            }
            RELEASE(unk40);
        }
    }
}

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

DataNode OptionsPanel::OnMsg(RCJobCompleteMsg const &msg) {
    int i;
    if (msg.Job() == unk3c) {
        MILO_LOG("Token: server response: %s\n", unk3c->GetResponseString());
        String str;
        static Symbol token_redemption_ready("token_redemption_ready");
        static Symbol token_redemption_error("token_redemption_error");
        static Symbol token_redemption_not_found("token_redemption_not_found");
        static Symbol token_redemption_other_player("token_redemption_other_player");
        static Symbol token_redemption_purchased("token_redemption_purchased");
        static Symbol token_redemption_too_early("token_redemption_too_early");
        static Symbol token_redemption_too_late("token_redemption_too_late");
        static Symbol leaderboard_no_net("leaderboard_no_net");
        unk3c->GetRedeemTokenData(i, str);
    } else if (msg.Job() != unk58) {
        return 1;
    }
    String temp1;
    String temp2;
    unk58->GetWebLinkCodeData(temp1);

    return 1;
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
