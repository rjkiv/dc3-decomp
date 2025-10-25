#pragma once

#include "net/HttpReq.h"
#include "net/JsonUtils.h"
#include "net/WebSvcReq.h"
#include "obj/Object.h"
#include "utl/DataPointMgr.h"

class DingoJob : public WebSvcRequest {
public:
    virtual ~DingoJob();
    virtual void SendCallback(bool, bool);
    virtual void Start();

    DingoJob(char const *, Hmx::Object *);

    int unk7c;
    DataPoint *mDataPoint; // 0x80
    int unk84;
    String unk88;
    JsonConverter unk90;
    JsonObject *unka4;
    int unka8;
    int unkac;

protected:
    virtual void StartImpl();
    virtual void Reset();
    virtual void CleanUp(bool);
    virtual void AddContent(HttpReq *);
    virtual bool CheckReqResult();

    void ParseResponse();
    void SetDataPoint(DataPoint const &);

private:
    void ParseResponse(JsonConverter *, JsonObject **, int *);
};

class DingoJobCompleteMsg {
public:
    DingoJobCompleteMsg(DingoJob *, bool);
};
