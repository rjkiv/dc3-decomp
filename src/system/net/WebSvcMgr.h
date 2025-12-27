#pragma once
#include "net/WebSvcReq.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/NetworkSocket.h"

class WebSvcMgr : public Hmx::Object {
public:
    WebSvcMgr();
    virtual ~WebSvcMgr();
    virtual DataNode Handle(DataArray *, bool);
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
    ) = 0;
    virtual void OnReqFinished(WebSvcRequest *req);
    virtual bool InitRequest(
        WebSvcRequest *req,
        ReqType req_type,
        const char *host_name,
        unsigned short port,
        const char *content,
        unsigned int content_length
    ) = 0;
    virtual bool InitRequest(
        WebSvcRequest *req,
        ReqType req_type,
        unsigned int ip_addr,
        unsigned short port,
        const char *content,
        unsigned int content_length
    ) = 0;
    virtual int NumRequestsStarted();
    virtual int NumRequests();

    void CancelOutstandingCalls();
    bool
    AddRequest(WebSvcRequest *req, unsigned int timeout_ms, bool immediate, bool resolve);

private:
    bool ResolveHostname(WebSvcRequest *req);

protected:
    virtual void Start(WebSvcRequest *req);
    NetAddress
    ResolveHostname(const char *hostname, const char *domain, unsigned short port);

    std::list<WebSvcRequest *> mRequests; // 0x2c
    std::map<String, NetAddress> mHostCache; // 0x34
};

extern WebSvcMgr &TheWebSvcMgr;
extern const char *kHMXDomain;
