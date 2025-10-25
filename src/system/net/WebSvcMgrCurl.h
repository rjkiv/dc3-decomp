#pragma once

#include "WebSvcReq.h"
#include "net/HttpReq.h"
#include "net/WebSvcMgr.h"
class WebSvcMgrCurl : public WebSvcMgr {
public:
    virtual ~WebSvcMgrCurl();
    virtual void Init();
    virtual bool DoRequest(
        ReqType,
        unsigned int,
        unsigned short,
        char const *,
        char const *,
        unsigned int,
        char const *,
        unsigned int
    );
    virtual void Poll();

    virtual bool InitRequest(
        WebSvcRequest *, ReqType, unsigned int, unsigned short, char const *, unsigned int
    );

    virtual bool InitRequest(
        WebSvcRequest *, ReqType, char const *, unsigned short, char const *, unsigned int
    );

    void InitCurl();

protected:
    virtual void Start(WebSvcRequest *);

private:
    void FindAndFinish(void *, bool, unsigned int);

    bool InitRequest(
        WebSvcRequest *,
        ReqType,
        char const *,
        unsigned int,
        unsigned short,
        char const *,
        unsigned int
    );
};
