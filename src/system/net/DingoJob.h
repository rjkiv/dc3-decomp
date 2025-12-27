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
    virtual unsigned int GetResponseDataLength() { return mResponseDataLength; }
    virtual unsigned int GetResponseStatusCode() { return mResponseStatusCode; }
    virtual bool IsWebSvcRequest() const { return false; }
    virtual void SendCallback(bool success, bool cancelled);

    const char *GetResponseString();

private:
    void ParseResponse(JsonConverter *reader, JsonObject **response, int *retVersion);

protected:
    virtual void CleanUp(bool success);
    virtual bool CheckReqResult();
    virtual void Reset();
    virtual void StartImpl();
    virtual bool DidCallSucceed() { return true; }
    virtual bool CheckReturned() { return true; }
    virtual void AddContent(HttpReq *httpReq);

    void ParseResponse();
    void SetDataPoint(const DataPoint &pt);

    int mResult; // 0x7c
    DataPoint *mDataPoint; // 0x80
    void *unk84;
    String mResponseStr; // 0x88
    JsonConverter mJsonReader; // 0x90
    JsonObject *mJsonResponse; // 0xa4
    int mJsonResponseVersion; // 0xa8
    int mTimeoutMs; // 0xac
};

DECLARE_MESSAGE(DingoJobCompleteMsg, "dingo_job_complete")
DingoJobCompleteMsg(DingoJob *job, bool success) : Message(Type(), job, success) {}
END_MESSAGE
