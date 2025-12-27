#pragma once
#include "curl/curl.h"
#include "net/HttpReq.h"
#include "utl/Str.h"

class HttpReqCurl : public HttpReq {
public:
    HttpReqCurl(ReqType type, unsigned int ip, unsigned short port, const char *url);
    HttpReqCurl(ReqType type, const char *hostname, unsigned short port, const char *url);
    virtual ~HttpReqCurl();
    virtual void Start();
    virtual void Poll() {}
    virtual void Do();
    virtual void Reset();
    virtual void *GetRequest() { return mReq; }
    virtual char *DetachBuffer();
    virtual unsigned int GetBufferSize();
    virtual void SetTimeout(unsigned int timeout);
    virtual void SetSSLCertPath(const char *path);
    virtual void SetSSLCertName(const char *name);
    virtual void SetSSLVerifyPeer(unsigned short value);
    virtual void SetSSLVerifyHost(unsigned short value);

private:
    curl_slist *mHeaders; // 0x50
    CURL *mReq; // 0x54
    char *mBuffer; // 0x58 - file the output should be written to
    unsigned int mBufferLength; // 0x5c
    String mSSLCertPath; // 0x60
    String mSSLCertName; // 0x68
    unsigned short mSSLVerifyPeer; // 0x70
    unsigned short mSSLVerifyHost; // 0x72
    unsigned int mTimeoutMs; // 0x74
};
