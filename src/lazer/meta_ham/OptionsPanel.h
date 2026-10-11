#pragma once
#include "meta/StorePurchaser.h"
#include "meta_ham/HamPanel.h"
#include "meta_ham/HamProfile.h"
#include "net_ham/RCJobDingo.h"
#include "net_ham/TokenJobs.h"
#include "net_ham/WebLinkJobs.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "stl/_pair.h"
#include "utl/JobMgr.h"
#include "utl/Symbol.h"
#include "xdk/win_types.h"

class OptionsPanel : public HamPanel {
public:
    virtual ~OptionsPanel();
    OBJ_CLASSNAME(OptionsPanel)
    OBJ_SET_TYPE(OptionsPanel)
    virtual DataNode Handle(DataArray *, bool);
    virtual void Poll();

    NEW_OBJ(OptionsPanel)

    OptionsPanel();
    bool OnRedeemToken(int, char const *);
    void OnPurchaseOfferByOfferString(int, char const *);
    bool OnGetLinkingCode(int);
    void OnXboxTokenRedemption(int);

protected:
    DataNode OnMsg(RCJobCompleteMsg const &);

private:
    DataNode OnMsg(SingleItemEnumCompleteMsg const &);

    RedeemTokenJob *mTokenJob; // 0x3c
    XboxPurchaser *mPurchaser; // 0x40
    std::pair<unsigned long long, const HamProfile *> mPendingPurchase; // 0x48
    GetWebLinkCodeJob *mGetLinkingCodeJob;
};

DECLARE_MESSAGE(TokenRedeemedMsg, "token_redeemed")
TokenRedeemedMsg(bool b, const String &s1, const Symbol &s2)
    : Message(Type(), b, s1, s2) {}
void SetSuccess(bool success) { mData->Node(2) = success; }
void SetOfferString(const String &s) { mData->Node(3) = s; }
void SetError(const Symbol &error) { mData->Node(4) = error; }
END_MESSAGE

DECLARE_MESSAGE(LinkingCodeRetrievedMsg, "linking_code_retrieved")
LinkingCodeRetrievedMsg(bool b, const String &s) : Message(Type(), b, s) {}
void SetSuccess(bool success) { mData->Node(2) = success; }
void SetOfferString(const String &s) { mData->Node(3) = s; }
END_MESSAGE
