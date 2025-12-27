#pragma once
#include "utl/Str.h"
#include <map>

enum ReqType {
    kHttpReqType_POST = 0,
    kHttpReqType_GET = 1,
    kHttpReqType_PUT = 2,
    kHttpReqType_HTTPS_POST = 3,
    kHttpReqType_HTTPS_GET = 4,
    kHttpReqType_HTTPS_PUT = 5,
    kHttpReqType_NIL = -1
};

class HttpReq {
public:
    enum State {
        kHttpReq_Connecting = 0,
        kHttpReq_Sending = 1,
        kHttpReq_Receiving = 2,
        kHttpReq_Done = 3,
        kHttpReq_Failure = 4,
        kHttpReq_Max = 5,
        kHttpReq_Nil = -1,
    };

    HttpReq(ReqType type, unsigned int ip, unsigned short port, const char *url);
    HttpReq(ReqType type, const char *hostname, unsigned short port, const char *url);
    virtual ~HttpReq(); // 0x0
    virtual void SetContent(const char *cnt) { mContent = cnt; } // 0x4
    virtual void SetContentLength(unsigned int len) { mContentLength = len; } // 0x8
    virtual void SetCookies(const std::map<String, String> &cookies) {
        mCookies = cookies;
    } // 0xc
    virtual void Start() = 0; // 0x10
    virtual void Poll() = 0; // 0x14
    virtual void Do() = 0; // 0x18
    virtual void Reset(); // 0x1c
    virtual void *GetRequest() = 0; // 0x20
    virtual char *DetachBuffer() = 0; // 0x24
    virtual unsigned int GetBufferSize() = 0; // 0x28
    virtual bool IsSafeToDelete() const { return true; } // 0x2c
    virtual void SetTimeout(unsigned int timeout) = 0; // 0x30
    virtual void SetType(ReqType t) { mType = t; } // 0x34
    virtual void SetUserAgent(const char *agent) { mUserAgent = agent; } // 0x38
    virtual void SetSSLCertPath(const char *path) {} // 0x3c
    virtual void SetSSLCertName(const char *name) {} // 0x40
    virtual void SetSSLVerifyPeer(unsigned short value) {} // 0x44
    virtual void SetSSLVerifyHost(unsigned short value) {} // 0x48
    virtual bool HasFailed(); // 0x4c
    virtual bool HasSucceeded(); // 0x50

    unsigned int GetStatusCode();
    void SetStatusCode(unsigned int code) { mStatusCode = code; }
    const char *GetHostName() const { return mHostName.c_str(); }
    unsigned int GetIPAddr() const { return mIPAddr; }
    const char *GetURL() const { return mURL.c_str(); }
    void SetIPAddr(unsigned int ip) { mIPAddr = ip; }
    void SetURL(const char *url) { mURL = url; }
    void MarkSuccess() { mState = kHttpReq_Done; }
    void MarkFailure() { mState = kHttpReq_Failure; }

protected:
    String mHostName; // 0x4
    unsigned int mIPAddr; // 0xc
    unsigned int mPort; // 0x10
    String mURL; // 0x14
    ReqType mType; // 0x1c
    String mUserAgent; // 0x20
    const char *mContent; // 0x28
    unsigned int mContentLength; // 0x2c
    unsigned int mStatusCode; // 0x30
    std::map<String, String> mCookies; // 0x34
    State mState; // 0x4c
};
