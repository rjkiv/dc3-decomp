#pragma once
#include "HttpReq.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "utl/Symbol.h"

class WebSvcRequest : public Hmx::Object {
public:
    virtual ~WebSvcRequest();
    virtual void Start();

    WebSvcRequest(char const *, char const *, Hmx::Object *);
    void Cancel(bool);
    void Do();
    bool HasFailed();
    bool HasSucceeded();
    bool IsRunning();
    void SetTimeout(unsigned int);
    void OnReqFailed();
    void OnReqSucceeded();
    void *GetRequest();
    void SetHttpReq(HttpReq *);
    void SetUserAgent(char const *);
    void SetStatusCode(unsigned int);
    char const *GetHostName();
    unsigned int GetIPAddr();
    char const *GetURL();
    void Poll();
    void SetCookies(std::map<String, String> const &);

    int mResponseData; // 0x2c
    int unk30;
    const ObjPtr<Hmx::Object> unk34;
    String unk48;
    HttpReq *mHttpReq; // 0x50
    int unk54;
    String unk58;
    int unk60;
    std::map<String, String> unk64;

protected:
    virtual void CleanUp(bool);
    virtual void Reset();
    virtual void SendCallback(bool, bool);

    void SetIPAddr(unsigned int);
    void SetURL(char const *);
    void MarkSuccess();
    void MarkFailure();
};

class WebReqCompleteMsg {
public:
    WebReqCompleteMsg(WebSvcRequest *, bool);
    static Symbol Type();
};
