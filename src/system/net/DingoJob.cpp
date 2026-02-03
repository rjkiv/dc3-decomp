#include "net/DingoJob.h"
#include "JsonUtils.h"
#include "macros.h"
#include "net/HttpReq.h"
#include "net/WebSvcMgr.h"
#include "net/WebSvcReq.h"
#include "net/DingoSvr.h"
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
}

void DingoJob::SendCallback(bool success, bool cancelled) {
    // Validate the response if the request succeeded
    if (success) {
        ParseResponse();
        // Check for error result codes
        if (!mJsonResponse || mResult == -1 || mResult == -4 || mResult == -0xb
            || mResult == -0x138b) {
            success = false;
        }
    }

    // Send completion message if callback is set
    if (mCallback) {
        static DingoJobCompleteMsg msg(this, false);
        msg[0] = this;
        msg[1] = success;
        mCallback->Handle(msg, true);

        // Log failure details if this failed and wasn't user-cancelled
        if (!success && !cancelled) {
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

void DingoJob::CleanUp(bool success) {
    WebSvcRequest::CleanUp(success);
    if (success) {
        // Copy response data to string member for safe ownership
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
    JsonObject *response = nullptr;
    ParseResponse(&converter, &response, nullptr);
    if (mResult == -3) {
        if (!TheServer.IsAuthenticated()) {
            TheServer.Poll();
        }
        if (TheServer.IsAuthenticated()) {
            TheServer.DelayJob(this);
            return false;
        }
    }
    return true;
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

    // Find the end of the encoded string
    const char *scan;
    for (scan = str2.c_str(); *scan != '\0'; scan++) {
    }

    // Calculate total size: "params=" (7 bytes) + encoded string length
    int size = (scan - str2.c_str() - 1) + 7;

    // Allocate buffer for the complete request body
    char *buf = (char *)_MemAllocTemp(size + 1, __FILE__, 0x6D, "", 0);
    unk84 = buf;

    // Copy the "params=" prefix into the buffer
    *(s64 *)buf = *(s64 *)"params=";

    // Find the end of the prefix (after the null terminator byte)
    char *end;
    for (end = (char *)unk84; *end != '\0'; end++) {
    }
    end--;

    // Append the encoded data to the prefix
    const char *data;
    for (data = str2.c_str(); *data != '\0'; data++) {
        *end++ = *data;
    }

    httpReq->SetContent((const char *)unk84);
    httpReq->SetContentLength(size);
}

void DingoJob::SetDataPoint(const DataPoint &point) {
    MILO_ASSERT(!mDataPoint, 0x27);
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
