#include "net/WebSvcMgrCurl.h"
#include "WebSvcReq.h"
#include "os/Debug.h"

WebSvcMgrCurl::~WebSvcMgrCurl() {}

void WebSvcMgrCurl::Init() {}

bool WebSvcMgrCurl::DoRequest(
    ReqType,
    unsigned int,
    unsigned short,
    char const *,
    char const *,
    unsigned int,
    char const *,
    unsigned int
) {
    return false;
}

void WebSvcMgrCurl::Poll() {}

bool WebSvcMgrCurl::InitRequest(
    WebSvcRequest *, ReqType, unsigned int, unsigned short, char const *, unsigned int
) {
    return false;
}

bool WebSvcMgrCurl::InitRequest(
    WebSvcRequest *, ReqType, char const *, unsigned short, char const *, unsigned int
) {
    return false;
}

void WebSvcMgrCurl::InitCurl() {}

void WebSvcMgrCurl::Start(WebSvcRequest *req) { MILO_ASSERT(req->GetRequest(), 0x40); }

void WebSvcMgrCurl::FindAndFinish(void *, bool, unsigned int) {}

bool WebSvcMgrCurl::InitRequest(
    WebSvcRequest *,
    ReqType,
    char const *,
    unsigned int,
    unsigned short,
    char const *,
    unsigned int
) {
    return false;
}
