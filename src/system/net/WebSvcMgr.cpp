#include "net/WebSvcMgr.h"
#include "net/WebSvcReq.h"

WebSvcMgr::WebSvcMgr() {}

WebSvcMgr::~WebSvcMgr() {
    unk2c.clear();
    unk34.clear();
}

void WebSvcMgr::Init() {}

void WebSvcMgr::OnReqFinished(WebSvcRequest *) {}

void WebSvcMgr::Poll() {}

int WebSvcMgr::NumRequestsStarted() { return 1; }

int WebSvcMgr::NumRequests() { return 1; }

void WebSvcMgr::CancelOutstandingCalls() {}

bool WebSvcMgr::AddRequest(WebSvcRequest *, unsigned int, bool, bool) { return false; }

void WebSvcMgr::Start(WebSvcRequest *) {}

bool WebSvcMgr::ResolveHostname(WebSvcRequest *) { return false; }
