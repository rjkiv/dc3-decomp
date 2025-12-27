#include "net/DingoJob.h"
#include "JsonUtils.h"
#include "macros.h"
#include "net/HttpReq.h"
#include "net/WebSvcMgr.h"
#include "net/WebSvcReq.h"
#include "os/Debug.h"
#include "os/OnlineID.h"
#include "utl/DataPointMgr.h"
#include "utl/MemMgr.h"
#include "utl/UrlEncode.h"

#pragma region DingoJob

DingoJob::DingoJob(char const *url, Hmx::Object *callback)
    : WebSvcRequest(url, "", callback), mResult(0), mDataPoint(0), mJsonResponse(0),
      mJsonResponseVersion(0), mTimeoutMs(10000) {
    unk84 = 0;
}

DingoJob::~DingoJob() { RELEASE(mDataPoint); }

void DingoJob::Start() {
    MILO_ASSERT(GetURL(), 0x49);
    MILO_ASSERT(strlen(GetURL()) != 0, 0x4A);

    //   local_40 = WebSvcRequest::GetURL((WebSvcRequest *)this);
    //   local_3c = *(char **)(TheServer + 0x44);
    //   local_38[0] = (char *)(**(code **)(*(int *)TheServer + 0x80))();
    //   pcVar2 =
    //   MakeString<>("/%s/%s/%s/%s",&PTR_s_1_82066608,local_38,&local_3c,&local_40);
    //   WebSvcRequest::SetURL((WebSvcRequest *)this,pcVar2);
    //   (**(code **)(*(int *)this + 0x80))(this);
    //   return;
}

void DingoJob::SendCallback(bool success, bool cancelled) {
    if (success) {
        ParseResponse();
        if (!mJsonResponse || mResult == -1 || mResult == -4 || mResult == -0xb
            || mResult == -0x138b) {
            success = false;
        }
    }
    if (mCallback) {
        static DingoJobCompleteMsg msg(this, false);
        msg[0] = this;
        msg[1] = success;
        mCallback->Handle(msg, true);
        if (!success) {
            if (!cancelled) {
                DataPoint pt("dingo_job_failed");
                pt.AddPair("location", "DingoJob::SendCallback");
                pt.AddPair("mResult", mResult);
                pt.AddPair("mJsonResponse", mJsonResponse ? "non-NULL" : "NULL");
                pt.AddPair("mResponseStr", mResponseStr.c_str());
                pt.AddPair("mBaseUrl", mBaseUrl.c_str());
                pt.AddPair("mResponseStatusCode", (int)GetResponseStatusCode());
                TheDataPointMgr.RecordDataPoint(pt);
                TheWebSvcMgr.CancelOutstandingCalls();
            }
        }
    }
}

void DingoJob::CleanUp(bool success) {
    WebSvcRequest::CleanUp(success);
    if (success) {
        char *src = mResponseData;
        int size = GetResponseDataLength();
        char *str_buffer =
            (char *)_MemAllocTemp(size + 1, __FILE__, 0x6D, "DingoJobTmp", 0);
        MILO_ASSERT(str_buffer, 0x6E);
        memcpy(str_buffer, src, size);
        str_buffer[size] = '\0';
        mResponseStr = str_buffer;
        MemFree(str_buffer);
    }
}

bool DingoJob::CheckReqResult() {
    JsonConverter converter;
    JsonObject *jObj = nullptr;
    ParseResponse(&converter, &jObj, nullptr);
    // more
    return false;
}

void DingoJob::Reset() {
    mResponseStr.erase();
    mResult = 0;
    WebSvcRequest::Reset();
}

void DingoJob::StartImpl() {
    AddContent(mHttpReq);
    WebSvcRequest::Start();
}

void DingoJob::AddContent(HttpReq *httpReq) {
    MILO_ASSERT(mDataPoint, 0xf1);
    MILO_ASSERT(httpReq, 0xf2);
    String str1, str2;
    mDataPoint->ToJSON(str1);
    URLEncode(str1.c_str(), str2, false);
    unk84 = new char[str2.length()];
}

void DingoJob::SetDataPoint(const DataPoint &point) {
    MILO_ASSERT(mDataPoint == NULL, 0x27);
    mDataPoint = new DataPoint(point);
    MILO_ASSERT(mDataPoint, 0x29);
}

void DingoJob::ParseResponse() {
    ParseResponse(&mJsonReader, &mJsonResponse, &mJsonResponseVersion);
}

void DingoJob::ParseResponse(JsonConverter *json, JsonObject **response, int *iptr) {
    MILO_ASSERT(json, 0x123);
    MILO_ASSERT(response, 0x124);
    const char *strResult = mResponseStr.c_str();
    mResult = -1000;
    MILO_ASSERT(strResult, 0x12a);
    JsonObject *jObj = json->LoadFromString(strResult);
    if (!jObj) {
        mResult = -1001;
    } else {
        JsonObject *resultObj = json->GetByName(jObj, "result");
        if (resultObj) {
            mResult = resultObj->Int();
            *response = json->GetByName(jObj, "response");
            if (iptr) {
                JsonObject *versionObj = json->GetByName(jObj, "version");
                if (versionObj) {
                    *iptr = versionObj->Int();
                }
            }
        }
    }
}
