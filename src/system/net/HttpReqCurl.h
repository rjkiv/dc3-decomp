#pragma once
#include "net/HttpReq.h"
#include "utl/Str.h"

class HttpReqCurl : public HttpReq {
public:
    virtual ~HttpReqCurl();
    virtual void Start();
    virtual void Do();
    virtual void Reset();
    virtual char *DetachBuffer();
    virtual unsigned int GetBufferSize();
    virtual void SetTimeout(unsigned int);
    virtual void SetSSLCertPath(char const *);
    virtual void SetSSLCertName(char const *);
    virtual void SetSSLVerifyPeer(unsigned short);
    virtual void SetSSLVerifyHost(unsigned short);

    HttpReqCurl(ReqType, unsigned int, unsigned short, char const *);
    HttpReqCurl(ReqType, char const *, unsigned short, char const *);

    int unk50;
    int mReq; // 0x54
    int unk58;
    unsigned int unk5c;
    String unk60;
    String unk68;
    unsigned short unk70;
    unsigned short unk72;
    int unk74;
};
