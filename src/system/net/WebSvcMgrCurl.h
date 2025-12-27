#pragma once
#include "WebSvcReq.h"
#include "curl/multi.h"
#include "net/HttpReq.h"
#include "net/WebSvcMgr.h"

class WebSvcMgrCurl : public WebSvcMgr {
public:
    WebSvcMgrCurl() : mCurlMultiHandle(0) {}
    virtual ~WebSvcMgrCurl();
    virtual void Init();
    virtual void Poll();
    virtual bool DoRequest(
        ReqType req_type,
        unsigned int ip_addr,
        unsigned short port,
        const char *url,
        const char *additional_hdr,
        unsigned int timeout_ms,
        const char *content,
        unsigned int content_length
    );
    virtual bool InitRequest(
        WebSvcRequest *req,
        ReqType req_type,
        const char *host_name,
        unsigned short port,
        const char *content,
        unsigned int content_length
    );
    virtual bool InitRequest(
        WebSvcRequest *req,
        ReqType req_type,
        unsigned int ip_addr,
        unsigned short port,
        const char *content,
        unsigned int content_length
    );

    void InitCurl();

private:
    void FindAndFinish(void *handle, bool success, unsigned int http_status);

    bool InitRequest(
        WebSvcRequest *req,
        ReqType req_type,
        const char *host_name,
        unsigned int ip_addr,
        unsigned short port,
        const char *content,
        unsigned int content_length
    );

protected:
    virtual void Start(WebSvcRequest *req);

    CURLM *mCurlMultiHandle; // 0x4c
};

extern WebSvcMgrCurl gWebSvcMgr;
