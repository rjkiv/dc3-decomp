#pragma once
#include "net/HttpReq.h"
#include "obj/Msg.h"
#include "obj/Object.h"

class WebSvcRequest : public Hmx::Object {
    // i guess? both of these call protected virtual methods,
    // but WebSvcRequest doesn't have any public getters for them
    friend class WebSvcMgrCurl;
    friend class WebSvcMgr;

public:
    WebSvcRequest(const char *url, const char *additional_hdr, Hmx::Object *callback);
    virtual ~WebSvcRequest();
    virtual void Start();
    virtual char *GetResponseData() { return mResponseData; }
    virtual unsigned int GetResponseDataLength() { return mResponseDataLength; }
    virtual unsigned int GetResponseStatusCode() { return mResponseStatusCode; }
    virtual bool IsWebSvcRequest() const { return true; }

    void Poll();
    void Do();
    bool IsInited() { return mHttpReq; } // guessing this is it
    bool IsNotStarted() { return mState == kNotStarted; }
    bool IsRunning() { return mState == kRunning; }
    bool IsFinished() { return mState == kFinished; }
    bool IsRemovalReady() { return mState == kReadyForRemoval; }
    bool IsDeleteReady() { return mState == kReadyForDeletion; }
    bool HasFailed();
    bool HasSucceeded();
    void SetTimeout(unsigned int ms);
    void OnReqFailed();
    void OnReqSucceeded();
    void Cancel(bool send_callback);
    Hmx::Object *GetCallback() const { return mCallback; }
    void SetCallback(Hmx::Object *cb) { mCallback = cb; }
    void *GetRequest();
    //  public: char* DetachResponseData();
    void SetHttpReq(HttpReq *req);
    void SetCookies(const std::map<String, String> &cookies);
    const std::map<String, String> &GetCookies() const;
    //  public: void SetSSLCertPath(const char*);
    //  public: void SetSSLCertName(const char*);
    //  public: void SetSSLVerifyPeer(const uint16_t);
    //  public: void SetSSLVerifyHost(const uint16_t);
    void SetStatusCode(unsigned int code);
    void SetUserAgent(const char *agent);
    const char *GetHostName();
    unsigned int GetIPAddr();
    //  public: uint32_t GetPort();
    const char *GetBaseURL() { return mBaseUrl.c_str(); }
    const char *GetURL();
    //  public: const char* GetAdditionalHeaders();

protected:
    virtual void CleanUp(bool success);
    virtual void SendCallback(bool success, bool cancelled);
    virtual bool CheckReqResult() { return true; }
    virtual void Reset();
    virtual bool MustFinishBeforeNext() { return false; }

    // void SetHostName(const char *);
    // void SetPort(unsigned int);
    // void SetType(ReqType);

    void SetIPAddr(unsigned int ip);
    void SetURL(const char *url);
    void MarkSuccess();
    void MarkFailure();

    char *mResponseData; // 0x2c
    unsigned int mResponseDataLength; // 0x30
    ObjPtr<Hmx::Object> mCallback; // 0x34
    String mBaseUrl; // 0x48
    HttpReq *mHttpReq; // 0x50
    enum {
        kNotStarted = 0x0000,
        kRunning = 0x0001,
        kFinished = 0x0002,
        kReadyForRemoval = 0x0003,
        kReadyForDeletion = 0x0004,
    } mState; // 0x54
private:
    String mAdditionalHdr; // 0x58
    unsigned int mResponseStatusCode; // 0x60
    std::map<String, String> mCookies; // 0x64
};

DECLARE_MESSAGE(WebReqCompleteMsg, "web_req_complete")
WebReqCompleteMsg(WebSvcRequest *req, bool success) : Message(Type(), req, success) {}
WebSvcRequest *GetReq() const { return mData->Obj<WebSvcRequest>(2); }
bool Success() const { return mData->Int(3); }
END_MESSAGE
