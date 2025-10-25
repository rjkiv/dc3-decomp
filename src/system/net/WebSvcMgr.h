#pragma once
#include "net/WebSvcReq.h"
#include "obj/Object.h"

class WebSvcMgr : public Hmx::Object {
public:
    virtual ~WebSvcMgr();
    virtual void Init();
    virtual void OnReqFinished(WebSvcRequest *);
    virtual void Poll();
    virtual int NumRequestsStarted();
    virtual int NumRequests();

    WebSvcMgr();
    void CancelOutstandingCalls();
    bool AddRequest(WebSvcRequest *, unsigned int, bool, bool);

    std::list<WebSvcRequest *> unk2c;
    std::map<String, String> unk34; // second parameter is NetAdress

protected:
    virtual void Start(WebSvcRequest *);
    // NetAdress ResolveHostname(char const *, char const *, unsigned short);

private:
    bool ResolveHostname(WebSvcRequest *);
};

extern WebSvcMgr &TheWebSvcMgr;
