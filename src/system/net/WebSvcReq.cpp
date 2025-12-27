#include "net/WebSvcReq.h"
#include "HttpReq.h"
#include "macros.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"

WebSvcRequest::WebSvcRequest(
    const char *url, const char *additional_hdr, Hmx::Object *callback
)
    : mResponseData(0), mResponseDataLength(0), mCallback(this, callback), mBaseUrl(url),
      mHttpReq(0), mState(kNotStarted), mAdditionalHdr(additional_hdr),
      mResponseStatusCode(0) {}

WebSvcRequest::~WebSvcRequest() {
    Reset();
    RELEASE(mHttpReq);
}

void WebSvcRequest::Start() {
    MILO_ASSERT(!mResponseData, 0x40);
    MILO_ASSERT(mHttpReq, 0x41);
    mHttpReq->Start();
    mState = kRunning;
}

void WebSvcRequest::CleanUp(bool success) {
    MILO_ASSERT(mHttpReq, 0x16e);
    MILO_ASSERT(!mResponseData, 0x16f);
    if (success) {
        mResponseData = mHttpReq->DetachBuffer();
        mResponseDataLength = mHttpReq->GetBufferSize();
    }
    mResponseStatusCode = mHttpReq->GetStatusCode();
}

void WebSvcRequest::SendCallback(bool success, bool cancelled) {
    if (mCallback) {
        static WebReqCompleteMsg msg(this, false);
        msg[0] = this;
        msg[1] = success;
        mCallback->Handle(msg, true);
    }
}

void WebSvcRequest::Reset() {
    if (mResponseData) {
        MemFree(mResponseData, __FILE__, 0x32);
        mResponseData = nullptr;
    }
    mResponseDataLength = 0;
    mResponseStatusCode = 0;
    mState = kNotStarted;
    if (mHttpReq) {
        SetURL(mBaseUrl.c_str());
        mHttpReq->Reset();
    }
}

void WebSvcRequest::Cancel(bool send_callback) {
    mState = kFinished;
    if (send_callback) {
        SendCallback(false, true);
    }
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

void WebSvcRequest::SetTimeout(unsigned int ms) {
    MILO_ASSERT(mHttpReq, 0x8f);
    mHttpReq->SetTimeout(ms);
}

void WebSvcRequest::OnReqFailed() {
    MILO_ASSERT(HasFailed(), 0x96);
    CleanUp(false);
    if (mState != kFinished) {
        mState = kFinished;
        SendCallback(false, false);
    }
}

void WebSvcRequest::OnReqSucceeded() {
    MILO_ASSERT(HasSucceeded(), 0xa3);
    CleanUp(true);
    if (CheckReqResult()) {
        mState = kFinished;
        SendCallback(true, false);
    } else {
        mState = kReadyForRemoval;
    }
}

void *WebSvcRequest::GetRequest() {
    MILO_ASSERT(mHttpReq, 0xc4);
    return mHttpReq->GetRequest();
}

void WebSvcRequest::SetHttpReq(HttpReq *req) {
    MILO_ASSERT(!mHttpReq, 0xcb);
    MILO_ASSERT(req, 0xcc);
    mHttpReq = req;
}

void WebSvcRequest::SetUserAgent(const char *agent) {
    MILO_ASSERT(mHttpReq, 0x103);
    mHttpReq->SetUserAgent(agent);
}

void WebSvcRequest::SetStatusCode(unsigned int code) {
    MILO_ASSERT(mHttpReq, 300);
    mHttpReq->SetStatusCode(code);
}

const char *WebSvcRequest::GetHostName() {
    MILO_ASSERT(mHttpReq, 0x134);
    return mHttpReq->GetHostName();
}

unsigned int WebSvcRequest::GetIPAddr() {
    MILO_ASSERT(mHttpReq, 0x13b);
    return mHttpReq->GetIPAddr();
}

const char *WebSvcRequest::GetURL() {
    MILO_ASSERT(mHttpReq, 0x149);
    return mHttpReq->GetURL();
}

void WebSvcRequest::Poll() {
    switch (mState) {
    case kRunning: {
        MILO_ASSERT(mHttpReq, 0x53);
        mHttpReq->Poll();
        if (HasFailed()) {
            OnReqFailed();
        } else if (HasSucceeded()) {
            OnReqSucceeded();
        }
        break;
    }
    case kFinished: {
        if (mHttpReq->IsSafeToDelete()) {
            mState = kReadyForDeletion;
        }
        break;
    }
    case kNotStarted:
    case kReadyForRemoval:
    case kReadyForDeletion:
        break;
    default:
        int state = mState;
        MILO_NOTIFY("WebSvcRequest: Unknown state %d", state);
        break;
    }
}

const std::map<String, String> &WebSvcRequest::GetCookies() const { return mCookies; }
void WebSvcRequest::SetCookies(std::map<String, String> const &map) { mCookies = map; }

void WebSvcRequest::SetIPAddr(unsigned int ip) {
    MILO_ASSERT(mHttpReq, 0xe7);
    mHttpReq->SetIPAddr(ip);
}

void WebSvcRequest::SetURL(char const *url) {
    MILO_ASSERT(mHttpReq, 0xfc);
    mHttpReq->SetURL(url);
}

void WebSvcRequest::MarkSuccess() {
    MILO_ASSERT(mHttpReq, 0x15d);
    mHttpReq->MarkSuccess();
}

void WebSvcRequest::MarkFailure() {
    MILO_ASSERT(mHttpReq, 0x15d);
    mHttpReq->MarkFailure();
}
