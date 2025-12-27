#include "net/HttpReq.h"
#include "macros.h"

HttpReq::HttpReq(ReqType type, unsigned int ip, unsigned short port, char const *url)
    : mHostName(), mIPAddr(ip), mPort(port), mURL(url), mType(type), mContent(0),
      mContentLength(0), mStatusCode(0), mState(kHttpReq_Nil) {}

HttpReq::HttpReq(ReqType type, char const *hostname, unsigned short port, char const *url)
    : mHostName(hostname), mIPAddr(0), mPort(port), mURL(url), mType(type), mContent(0),
      mContentLength(0), mStatusCode(0), mState(kHttpReq_Nil) {}

HttpReq::~HttpReq() { RELEASE(mContent); }

void HttpReq::Reset() {
    mState = kHttpReq_Nil;
    mStatusCode = 0;
}

bool HttpReq::HasFailed() { return mState == kHttpReq_Failure; }

bool HttpReq::HasSucceeded() { return mState == kHttpReq_Done; }

unsigned int HttpReq::GetStatusCode() { return mStatusCode; }
