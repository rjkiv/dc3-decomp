#include "net/DingoSvr.h"
#include "DingoJob.h"
#include "WebSvcReq.h"
#include "meta/ConnectionStatusPanel.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "os/System.h"
#include "utl/DataPointMgr.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include <cstring>

DingoServer::DingoServer() : mAuthState(kServerUnauthed), mPort(0), unk70(-1), unk74(-1) {
    for (int i = 0; i < DIM(unk78); i++) {
        unk78[i] = false;
    }
}

BEGIN_HANDLERS(DingoServer)
    HANDLE_MESSAGE(SigninChangedMsg)
    HANDLE_MESSAGE(ConnectionStatusChangedMsg)
    HANDLE_EXPR(is_authed, IsAuthenticated())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void DingoServer::Init() {
    SetName("server", ObjectDir::Main());
    mLocale = PlatformRegionToSymbol(ThePlatformMgr.GetRegion());
    ThePlatformMgr.AddSink(this, SigninChangedMsg::Type());
    ThePlatformMgr.AddSink(this, ConnectionStatusChangedMsg::Type());
    mLanguage = SystemLanguage();
}

void DingoServer::Logout() {
    unk40 = "";
    mAuthState = kServerUnauthed;
    unk74 = -1;
    for (int i = 0; i < DIM(unk78); i++) {
        unk78[i] = false;
    }
    mOnlineId.Clear();
}

void DingoServer::ManageJob(DingoJob *job) {
    MILO_ASSERT(job, 0xd0);
    bool isUrlDisabled = false;
    FOREACH (it, mDisabledUrls) {
        String cur(*it);
        if (strncmp(job->GetBaseURL(), cur.c_str(), cur.length()) == 0) {
            isUrlDisabled = true;
            break;
        }
    }
    bool shouldSendFailureCallback = true;
    bool authSucceeded = true;
    bool justAuthenticated = false;
    if (!isUrlDisabled) {
        if (!IsAuthenticated()) {
            MILO_NOTIFY("ManageJob without authentication.");
            if (ThePlatformMgr.IsConnected()) {
                authSucceeded = TheServer.Authenticate(unk74);
                justAuthenticated = true;
            } else {
                authSucceeded = false;
            }
        }
        if (authSucceeded && !job->GetHttpReq()) {
            shouldSendFailureCallback = !InitAndAddJob(job, false, justAuthenticated);
        }
    }
    if (shouldSendFailureCallback) {
        job->SendCallback(false, false);
        delete job;
    }
}

void DingoServer::FillAuthParams(DataPoint &point) {
    static Symbol locale("locale");
    point.AddPair(locale, mLocale.c_str());
    static Symbol language("language");
    point.AddPair(language, mLanguage.c_str());
}

void DingoServer::DoAdditionalLogin() {
    MILO_ASSERT(mAuthUrl.length() > 0, 0xa9);
    MILO_ASSERT(mAuthState == kServerAuthed, 0xAA);
    if (mAuthState == kServerAuthed) {
        if (mAuthUrl.length() != 0) {
            for (int i = 0; i < 4; i++) {
                if (!unk78[i]) {
                    DataPoint pt;
                    if (FillAuthParamsFromPadNum(pt, i)
                        && SendAuthenticateMsg(mAuthUrl.c_str(), pt, nullptr)) {
                        unk78[i] = true;
                    }
                }
            }
        }
    }
}

void DingoServer::DelayJob(DingoJob *job) { mDelayedJobs.push_back(job); }

void DingoServer::CancelDelayedCalls() {
    FOREACH (it, mDelayedJobs) {
        DingoJob *cur = *it;
        cur->Cancel(true);
        delete cur;
    }
    mDelayedJobs.clear();
}
