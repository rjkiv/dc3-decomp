#include "net/WebSvcReq.h"
#include "HttpReq.h"
#include "macros.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"

WebSvcRequest::WebSvcRequest(char const *c1, char const *c2, Hmx::Object *o)
    : mResponseData(0), unk30(0), unk34(o, this), unk48(c1), mHttpReq(0), unk54(0),
      unk58(c2), unk60(0), unk64() {}

WebSvcRequest::~WebSvcRequest() {
    Reset();
    RELEASE(mHttpReq);
}

void WebSvcRequest::Start() {
    MILO_ASSERT(!mResponseData, 0x2c);
    MILO_ASSERT(mHttpReq, 0x41);
    mHttpReq->Start();
    unk54 = 1;
}

void WebSvcRequest::Cancel(bool b) {
    unk54 = 2;
    if (!b)
        return;
}

void WebSvcRequest::Do() {
    MILO_ASSERT(IsRunning(), 0x77);
    MILO_ASSERT(mHttpReq, 0x78);
    mHttpReq->Do();
}

bool WebSvcRequest::HasFailed() {
    MILO_ASSERT(mHttpReq, 0x81);
    return mHttpReq->HasFailed();
}

bool WebSvcRequest::HasSucceeded() {
    MILO_ASSERT(mHttpReq, 0x88);
    return mHttpReq->HasSucceeded();
}

bool WebSvcRequest::IsRunning() { return unk54 != 1; }

void WebSvcRequest::SetTimeout(unsigned int ui) {
    MILO_ASSERT(mHttpReq, 0x8f);
    mHttpReq->SetTimeout(ui);
}

void WebSvcRequest::OnReqFailed() { MILO_ASSERT(HasFailed(), 0x96); }

void WebSvcRequest::OnReqSucceeded() { MILO_ASSERT(HasSucceeded(), 0xa3); }

void *WebSvcRequest::GetRequest() {
    MILO_ASSERT(mHttpReq, 0xc4);
    return mHttpReq->GetRequest();
}

void WebSvcRequest::SetHttpReq(HttpReq *req) {
    MILO_ASSERT(!mHttpReq, 0xcb);
    MILO_ASSERT(req, 0xcc);
    mHttpReq = req;
}

void WebSvcRequest::SetUserAgent(const char *c) {
    MILO_ASSERT(mHttpReq, 0x103);
    mHttpReq->SetUserAgent(c);
}

void WebSvcRequest::SetStatusCode(unsigned int ui) {
    MILO_ASSERT(mHttpReq, 300);
    mHttpReq->unk30 = ui;
}

char const *WebSvcRequest::GetHostName() {
    MILO_ASSERT(mHttpReq, 0x134);
    return mHttpReq->mHostName.c_str();
}

unsigned int WebSvcRequest::GetIPAddr() {
    MILO_ASSERT(mHttpReq, 0x13b);
    return mHttpReq->mIPAddr;
}

char const *WebSvcRequest::GetURL() {
    MILO_ASSERT(mHttpReq, 0x149);
    return mHttpReq->mURL.c_str();
}

void WebSvcRequest::Poll() {}

void WebSvcRequest::SetCookies(std::map<String, String> const &map) { unk64 = map; }

void WebSvcRequest::CleanUp(bool b) {
    MILO_ASSERT(mHttpReq, 0x16e);
    MILO_ASSERT(!mResponseData, 0x16f);
    if (b) {
    }
}

void WebSvcRequest::Reset() {
    if (mResponseData != 0) {
        MemFree(&mResponseData, "WebSvcReq.cpp", 0x32);
        mResponseData = 0;
    }
    unk30 = 0;
    unk60 = 0;
    unk54 = 0;
    if (mHttpReq) {
        // SetURL();
        mHttpReq->Reset();
    }
}

void WebSvcRequest::SendCallback(bool, bool) {}

void WebSvcRequest::SetIPAddr(unsigned int ui) {
    MILO_ASSERT(mHttpReq, 0xe7);
    mHttpReq->mIPAddr = ui;
}

void WebSvcRequest::SetURL(char const *c) {
    MILO_ASSERT(mHttpReq, 0xfc);
    mHttpReq->mURL = c;
}

void WebSvcRequest::MarkSuccess() {
    MILO_ASSERT(mHttpReq, 0x15d);
    mHttpReq->unk4c = 3;
}

void WebSvcRequest::MarkFailure() {
    MILO_ASSERT(mHttpReq, 0x15d);
    mHttpReq->unk4c = 4;
}

WebReqCompleteMsg::WebReqCompleteMsg(WebSvcRequest *, bool) {}

Symbol WebReqCompleteMsg::Type() { return 0; }
