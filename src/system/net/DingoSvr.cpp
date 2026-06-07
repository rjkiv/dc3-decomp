#include "net/DingoSvr.h"
#include "DingoJob.h"
#include "DingoSvr.h"
#include "WebSvcReq.h"
#include "meta/ConnectionStatusPanel.h"
#include "net/DingoAuthJob.h"
#include "net/HttpReq.h"
#include "net/WebSvcMgr.h"
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
    bool b5 = false;
    FOREACH (it, mDisabledUrls) {
        String cur(*it);
        if (strncmp(job->GetBaseURL(), cur.c_str(), cur.length()) == 0) {
            b5 = true;
            break;
        }
    }
    bool u7 = true;
    bool c4 = true;
    bool b8 = false;
    if (!b5) {
        if (!IsAuthenticated()) {
            MILO_NOTIFY("ManageJob without authentication.");
            if (ThePlatformMgr.IsConnected()) {
                c4 = TheServer.Authenticate(unk74);
                b8 = true;
            } else {
                c4 = false;
            }
        }
        HttpReq *req = job->GetHttpReq();
        if (c4 && !req) {
            u7 = !InitAndAddJob(job, false, b8);
        }
    }
    if (u7) {
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

void DingoServer::AddDelayedCalls() {
    FOREACH (it, mDelayedJobs) {
        DingoJob *req = *it;
        bool addReq = TheWebSvcMgr.AddRequest(req, req->GetTimeoutMs(), false, false);
        if (!addReq) {
            MILO_NOTIFY("Unable to add delayed job!");
            req->Cancel(true);
            delete req;
        }
    }
    mDelayedJobs.clear();
}

bool DingoServer::InitAndAddJob(DingoJob *job, bool immediate, bool delay) {
    ReqType type = kHttpReqType_POST;
    unsigned short port = GetPort();
    if (GetSSLEnable()) {
        type = kHttpReqType_PUT;
        port = 0x1bb;
    }
    bool success;
    if (GetIPAddr() != 0) {
        success = TheWebSvcMgr.InitRequest(job, type, GetIPAddr(), port, 0, 0);
    } else {
        success = TheWebSvcMgr.InitRequest(job, type, GetHostName(), port, 0, 0);
    }
    if (success) {
        job->SetUserAgent(mUserAgent.c_str());
        if (delay) {
            DelayJob(job);
            return true;
        }
        return TheWebSvcMgr.AddRequest(job, job->GetTimeoutMs(), immediate, false);
    }
    MILO_NOTIFY("InitAndAddJob failed.");
    return false;
}

bool DingoServer::Authenticate(int padnum, const char *url) {
    if (mAuthState != kServerUnauthed) {
        return true;
    }
    mAuthState = kServerAuthenticating;
    mAuthUrl = url;
    DataPoint dataP;
    if (padnum < 0) {
        FillAuthParams(dataP);
    } else if (!FillAuthParamsFromPadNum(dataP, padnum)) {
        return false;
    }
    return SendAuthenticateMsg(url, dataP, this);
}

bool DingoServer::SendAuthenticateMsg(
    const char *url, DataPoint &pt, Hmx::Object *callback
) {
    return InitAndAddJob(new AuthenticateReqJob(url, pt, callback), true, false);
}

DataNode DingoServer::OnMsg(const ConnectionStatusChangedMsg &msg) {
    if (msg.Connected()) {
        return DataNode(kDataInt, 0);
    } else {
        Disconnect();
        return DataNode(kDataInt, 1);
    }
}

DataNode DingoServer::OnMsg(const SigninChangedMsg &msg) {
    int mask = msg.GetMask();
    int changedMask = msg.GetChangedMask();

    if (!IsAuthenticated()) {
        return 0;
    } else {
        if (((1 << unk74) & mask) == 0) {
            Logout();
        } else {
            for (int i = 0; i < 4; i++) {
                if ((i != unk74) && ((1 << i) & changedMask) != 0) {
                    unk78[i] = false;
                }
            }
            DoAdditionalLogin();
        }
    }
    return 1;
}

DataNode DingoServer::OnMsg(const DingoJobCompleteMsg &msg) {
    if (mAuthState != kServerAuthenticating) {
        int state = mAuthState;
        MILO_NOTIFY("Got auth response in wrong state: %d.", state);
    } else {
        if (msg.GetVal3() != 0) {
            AuthenticateReqJob *job = dynamic_cast<AuthenticateReqJob *>(msg.GetJob());
            MILO_ASSERT(job, 0x14e);
            job->ParseResponse();
            if (job->GetResult() == 1) {
                unk40 = job->SessionID();
                mAuthState = kServerAuthed;
                OnAuthSuccess();
                AddDelayedCalls();
                ServerStatusChangedMsg msg(kServerStatusConnected);
                Export(msg, false);
                DoAdditionalLogin();
            } else {
                DataPoint dataP("svr_sent_non_success_on_auth");
                dataP.AddPair("location", "DingoSvr::OnMsg1");
                dataP.AddPair("result", job->GetResult());
                dataP.AddPair("response_str", job->GetResponseString());
                dataP.AddPair("mBaseUrl", job->GetBaseURL());
                dataP.AddPair("mResponseStatusCode", (int)job->GetResponseStatusCode());
                dataP.AddPair("severity", "warn");
                dataP.AddPair("project", "sync");
                TheDataPointMgr.RecordDebugDataPoint(dataP);
                CancelDelayedCalls();
                TheWebSvcMgr.CancelOutstandingCalls();
                Logout();
                ServerStatusChangedMsg msg(kServerStatusDisconnected);
                Export(msg, false);
            }
        } else {
            SendDebugDataPoint(
                "auth_msg_send_failure",
                "location",
                "DingoSvr::OnMsg2",
                "severity",
                "warn",
                "project",
                "sync"
            );
            CancelDelayedCalls();
            TheWebSvcMgr.CancelOutstandingCalls();
            Logout();
            ServerStatusChangedMsg msg(kServerStatusDisconnected);
            Export(msg, false);
        }
        return 0;
    }
    return 1;
}
