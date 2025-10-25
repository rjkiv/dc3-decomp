#include "net/DingoJob.h"
#include "JsonUtils.h"
#include "macros.h"
#include "net/HttpReq.h"
#include "net/WebSvcReq.h"
#include "os/Debug.h"
#include "os/OnlineID.h"
#include "utl/DataPointMgr.h"

#pragma region DingoJob

DingoJob::DingoJob(char const *c, Hmx::Object *o)
    : WebSvcRequest(c, "", o), unk7c(0), mDataPoint(0), unka4(0), unka8(0), unkac(10000) {
    unk84 = 0;
}

DingoJob::~DingoJob() { RELEASE(mDataPoint); }

void DingoJob::SendCallback(bool, bool) {}

void DingoJob::Start() {}

void DingoJob::StartImpl() { WebSvcRequest::Start(); }

void DingoJob::Reset() {
    unk88.erase();
    unk7c = 0;
    WebSvcRequest::Reset();
}

void DingoJob::CleanUp(bool b) {
    WebSvcRequest::CleanUp(b);
    if (b) {
    }
}

void DingoJob::AddContent(HttpReq *httpReq) {
    MILO_ASSERT(mDataPoint, 0xf1);
    MILO_ASSERT(httpReq, 0xf2);
}

bool DingoJob::CheckReqResult() { return false; }

void DingoJob::SetDataPoint(DataPoint const &point) {
    MILO_ASSERT(mDataPoint == NULL, 0x27);
    MILO_ASSERT(mDataPoint, 0x29);
}

void DingoJob::ParseResponse() {}

void DingoJob::ParseResponse(JsonConverter *json, JsonObject **response, int *i) {
    MILO_ASSERT(json, 0x123);
    MILO_ASSERT(response, 0x124);

    unk7c = -1000;
    // MILO_ASSERT(strResult, 0x12a);
}

#pragma endregion DingoJob
#pragma region DingoJobCompleteMsg

DingoJobCompleteMsg::DingoJobCompleteMsg(DingoJob *, bool) {}

#pragma endregion DingoJobCompleteMsg
