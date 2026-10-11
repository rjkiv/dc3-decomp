#include "meta_ham/OptionsPanel.h"
#include "OptionsPanel.h"
#include "ProfileMgr.h"
#include "macros.h"
#include "meta/StoreOffer.h"
#include "meta/StorePurchaser.h"
#include "meta_ham/HamPanel.h"
#include "net_ham/RCJobDingo.h"
#include "net_ham/RockCentral.h"
#include "net_ham/TokenJobs.h"
#include "net_ham/WebLinkJobs.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "ui/UIPanel.h"
#include "utl/JobMgr.h"
#include "utl/Locale.h"
#include "utl/Symbol.h"
#include "xdk/xapilibi/xbox.h"

OptionsPanel::OptionsPanel() {
    mPurchaser = nullptr;
    mPendingPurchase.first = 0;
    mPendingPurchase.second = nullptr;
    mTokenJob = nullptr;
    mGetLinkingCodeJob = nullptr;
}

OptionsPanel::~OptionsPanel() {}

void OptionsPanel::Poll() {
    UIPanel::Poll();
    if (mPurchaser) {
        mPurchaser->Poll();
        if (!mPurchaser->IsPurchasing()) {
            if (mPurchaser->IsSuccess() && !mPurchaser->PurchaseMade()
                && mPurchaser->NeedsEnum() && mPendingPurchase.second) {
                ThePlatformMgr.QueueEnumJob(new PostPurchaseEnumJob(
                    this,
                    mPendingPurchase.second->GetPadNum(),
                    mPendingPurchase.first,
                    mPurchaser->Source(),
                    mPurchaser->PID()
                ));
            }
            RELEASE(mPurchaser);
        }
    }
}

bool OptionsPanel::OnRedeemToken(int i, char const *str) {
    mTokenJob = new RedeemTokenJob(this, i, str);
    TheRockCentral.ManageJob(mTokenJob);
    return true;
}

void OptionsPanel::OnPurchaseOfferByOfferString(int i, char const *c) {
    unsigned long long ID = StorePurchaseable::OfferStringToID(c);
    mPurchaser = new XboxPurchaser(i, ID, 0, 0, gNullStr, 0);
    mPendingPurchase.first = ID;
    mPendingPurchase.second = TheProfileMgr.GetProfileFromPad(i);
    mPurchaser->Initiate();
}

bool OptionsPanel::OnGetLinkingCode(int i) {
    mGetLinkingCodeJob = new GetWebLinkCodeJob(this, i);
    TheRockCentral.ManageJob(mGetLinkingCodeJob);
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
    if (msg.Job() == mTokenJob) {
        MILO_LOG("Token: server response: %s\n", mTokenJob->GetResponseString());
        int res;
        String offer;
        static Symbol token_redemption_ready("token_redemption_ready");
        static Symbol token_redemption_error("token_redemption_error");
        static Symbol token_redemption_not_found("token_redemption_not_found");
        static Symbol token_redemption_other_player("token_redemption_other_player");
        static Symbol token_redemption_purchased("token_redemption_purchased");
        static Symbol token_redemption_too_early("token_redemption_too_early");
        static Symbol token_redemption_too_late("token_redemption_too_late");
        static Symbol leaderboard_no_net("leaderboard_no_net");
        mTokenJob->GetRedeemTokenData(res, offer);
        bool success;
        Symbol error;
        switch (res) {
        case 0xA0002:
            error = token_redemption_ready;
            success = true;
            break;
        case 0xA0005:
            error = token_redemption_ready;
            success = true;
            break;
        case 0xA0006:
            error = token_redemption_purchased;
            success = true;
            break;
        case 0xA0007:
            error = token_redemption_ready;
            success = true;
            break;
        case 0x800A0003:
            error = token_redemption_not_found;
            success = false;
            break;
        case 0x800A0005:
            error = token_redemption_other_player;
            success = false;
            break;
        case 0x800A0008:
            error = token_redemption_too_late;
            success = false;
            break;
        case 0x800A0009:
            error = token_redemption_too_early;
            success = false;
            break;
        default:
            bool online = TheRockCentral.IsOnline();
            success = false;
            if (!online) {
                error = leaderboard_no_net;
            } else {
                error = token_redemption_error;
            }
            break;
        }
        static TokenRedeemedMsg msg(true, String(""), token_redemption_ready);
        msg.SetSuccess(success);
        msg.SetOfferString(offer);
        msg.SetError(error);
        UIPanel *panel = ObjectDir::Main()->Find<UIPanel>("store_redeem_token_panel");
        panel->HandleType(msg);
        mTokenJob = nullptr;
    } else if (msg.Job() == mGetLinkingCodeJob) {
        String wlcData;
        String offer;
        bool webData = mGetLinkingCodeJob->GetWebLinkCodeData(wlcData);
        static LinkingCodeRetrievedMsg msg(true, String(""));
        bool success = webData && wlcData != "N/A";
        static Symbol linking_code_desc("linking_code_desc");
        static Symbol linking_code_failure("linking_code_failure");
        if (success) {
            offer = Localize(linking_code_desc, false, TheLocale);
            offer += "\n\n";
            offer += wlcData;
        } else {
            offer = Localize(linking_code_failure, false, TheLocale);
        }
        msg.SetSuccess(success);
        msg.SetOfferString(offer);
        UIPanel *panel = ObjectDir::Main()->Find<UIPanel>("options_panel");
        if (panel->GetState() == UIPanel::kUp) {
            panel->HandleType(msg);
        }
        mGetLinkingCodeJob = nullptr;
    }
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
