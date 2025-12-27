#include "net/WebSvcMgr.h"
#include "net/WebSvcReq.h"
#include "obj/Dir.h"
#include "os/Debug.h"
#include "os/NetworkSocket.h"

const char *kHMXDomain = "harmonixmusic.com";

WebSvcMgr::WebSvcMgr() {}

WebSvcMgr::~WebSvcMgr() { DeleteAll(mRequests); }

BEGIN_HANDLERS(WebSvcMgr)
END_HANDLERS

void WebSvcMgr::Init() { SetName("web_svc_mgr", ObjectDir::Main()); }

void WebSvcMgr::OnReqFinished(WebSvcRequest *req) {
    MILO_ASSERT(req, 0x181);
    MILO_ASSERT(req->IsDeleteReady(), 0x182);
    delete req;
}

int WebSvcMgr::NumRequestsStarted() {
    int numTotalReqs = mRequests.size();
    int numUnstartedReqs = 0;
    FOREACH (it, mRequests) {
        WebSvcRequest *cur = *it;
        if (cur->IsNotStarted()) {
            numUnstartedReqs++;
        }
    }
    return numTotalReqs - numUnstartedReqs;
}

int WebSvcMgr::NumRequests() { return mRequests.size(); }

void WebSvcMgr::Start(WebSvcRequest *req) { req->Start(); }

void WebSvcMgr::CancelOutstandingCalls() {
    FOREACH (it, mRequests) {
        WebSvcRequest *cur = *it;
        if (!cur->IsWebSvcRequest() && (cur->IsNotStarted() || cur->IsRunning())) {
            cur->Cancel(true);
        }
    }
}

bool WebSvcMgr::AddRequest(
    WebSvcRequest *req, unsigned int timeout_ms, bool immediate, bool resolve
) {
    MILO_ASSERT(req, 0xD7);
    if (resolve && !ResolveHostname(req)) {
        req->Cancel(true);
        return false;
    }
    req->SetTimeout(timeout_ms);
    if (immediate) {
        mRequests.push_front(req);
    } else {
        mRequests.push_back(req);
    }
    return true;
}

bool WebSvcMgr::ResolveHostname(WebSvcRequest *req) {
    unsigned int ip = req->GetIPAddr();
    if (ip == 0) {
        NetAddress addr = ResolveHostname(req->GetHostName(), kHMXDomain, 0x50);
        if (addr.mIP == 0) {
            addr = ResolveHostname(req->GetHostName(), nullptr, 0x50);
            if (addr.mIP == 0) {
                return false;
            }
        }
        req->UpdateIP(addr.mIP);
    }
    return true;
}

NetAddress
WebSvcMgr::ResolveHostname(const char *hostname, const char *domain, unsigned short port) {
    NetAddress ret;
    String str(hostname);
    if (domain) {
        str += ".";
        str += domain;
    }
    auto it = mHostCache.find(str.c_str());
    if (it != mHostCache.end()) {
        ret = it->second;
    } else {
        ret = NetworkSocket::SetIPPortFromHostPort(hostname, domain, port);
    }
    if (ret.mIP != 0 && it == mHostCache.end()) {
        mHostCache.insert(std::make_pair(str, ret));
    }
    return ret;
}
