#pragma once
#include "net/HttpReq.h"
#include "net/JsonUtils.h"
#include "net/WebSvcReq.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "utl/DataPointMgr.h"

class DingoJob : public WebSvcRequest {
public:
    DingoJob(const char *url, Hmx::Object *callback);
    virtual ~DingoJob();
    virtual void Start();
    virtual unsigned int GetResponseDataLength() {
        return WebSvcRequest::GetResponseDataLength();
    }
    virtual unsigned int GetResponseStatusCode() {
        return WebSvcRequest::GetResponseStatusCode();
    }
    virtual bool IsWebSvcRequest() const { return false; }
    virtual void SendCallback(bool success, bool cancelled);

    DataPoint *GetDataPoint() { return mDataPoint; }
    int GetResult() { return mResult; }
    //  public: bool Success() const;
    const char *GetResponseString();
    //  public: const char* GetBaseUrl();
    void SetTimeout(unsigned int ms) { mTimeoutMs = ms; }
    unsigned int GetTimeout() { return mTimeoutMs; }

protected:
    virtual void CleanUp(bool success);
    virtual bool CheckReqResult();
    virtual void Reset();
    virtual void StartImpl();
    virtual bool DidCallSucceed() { return true; }
    virtual bool CheckReturned() { return true; }
    virtual void AddContent(HttpReq *httpReq);

    // void SetResponseString(const char *);
    void ParseResponse();
    void SetDataPoint(const DataPoint &pt);
    JsonConverter &GetJsonConverter() { return mJsonConverter; }
    JsonObject *GetJsonResponse() const { return mJsonResponse; }
    int GetJsonResponseVersion() const { return mJsonResponseVersion; }

    int mResult; // 0x7c

private:
    void ParseResponse(JsonConverter *reader, JsonObject **response, int *retVersion);

    DataPoint *mDataPoint; // 0x80
    char *mParams;
    String mResponseStr; // 0x88
    JsonConverter mJsonConverter; // 0x90
    JsonObject *mJsonResponse; // 0xa4
    int mJsonResponseVersion; // 0xa8
    unsigned int mTimeoutMs; // 0xac
};

DECLARE_MESSAGE(DingoJobCompleteMsg, "dingo_job_complete")
DingoJobCompleteMsg(DingoJob *job, bool success) : Message(Type(), job, success) {}
DingoJob *GetJob() const { return mData->Obj<DingoJob>(2); }
bool Success() const { return mData->Int(3); }
END_MESSAGE
