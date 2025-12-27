#pragma once
#include "HttpReq.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/NetworkSocket.h"
#include "utl/Symbol.h"

class WebSvcRequest : public Hmx::Object {
public:
    enum State {
        kNotStarted = 0,
        kRunning = 1,
        kFinished = 2,
        kReadyForRemoval = 3,
        kReadyForDeletion = 4,
        kNumStates = -1,
    };

    WebSvcRequest(const char *url, const char *additional_hdr, Hmx::Object *callback);
    virtual ~WebSvcRequest();
    virtual void Start();
    virtual char *GetResponseData() { return mResponseData; }
    virtual unsigned int GetResponseDataLength() { return mResponseDataLength; }
    virtual unsigned int GetResponseStatusCode() { return mResponseStatusCode; }
    virtual bool IsWebSvcRequest() const { return true; }

    void Cancel(bool send_callback);
    void Do();
    bool HasFailed();
    bool HasSucceeded();
    void SetTimeout(unsigned int ms);
    void OnReqFailed();
    void OnReqSucceeded();
    void *GetRequest();
    void SetHttpReq(HttpReq *req);
    void SetUserAgent(const char *agent);
    void SetStatusCode(unsigned int code);
    char const *GetHostName();
    unsigned int GetIPAddr();
    char const *GetURL();
    void Poll();
    void SetCookies(const std::map<String, String> &cookies);
    const std::map<String, String> &GetCookies() const;
    bool IsDeleteReady() const { return mState == kReadyForDeletion; }
    bool IsNotStarted() const { return mState == kNotStarted; }
    bool IsRunning() const { return mState == kRunning; }
    bool IsFinished() const { return mState == kFinished; }
    void UpdateIP(unsigned int ip) { SetIPAddr(ip); }
    const char *GetBaseURL() const { return mBaseUrl.c_str(); }
    HttpReq *GetHttpReq() const { return mHttpReq; }

protected:
    virtual void CleanUp(bool success);
    virtual void SendCallback(bool success, bool cancelled);
    virtual bool CheckReqResult() { return true; }
    virtual void Reset();
    virtual bool MustFinishBeforeNext() { return false; }

    void SetIPAddr(unsigned int ip);
    void SetURL(const char *url);
    void MarkSuccess();
    void MarkFailure();

    char *mResponseData; // 0x2c
    unsigned int mResponseDataLength; // 0x30
    ObjPtr<Hmx::Object> mCallback; // 0x34
    String mBaseUrl; // 0x48
    HttpReq *mHttpReq; // 0x50
    State mState; // 0x54
    String mAdditionalHdr; // 0x58
    unsigned int mResponseStatusCode; // 0x60
    std::map<String, String> mCookies; // 0x64
};

DECLARE_MESSAGE(WebReqCompleteMsg, "web_req_complete")
WebReqCompleteMsg(WebSvcRequest *req, bool success) : Message(Type(), req, success) {}
END_MESSAGE
