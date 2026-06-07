#include "net/DingoJob.h"
#include "JsonUtils.h"
#include "macros.h"
#include "net/DingoSvr.h"
#include "net/HttpReq.h"
#include "net/WebSvcMgr.h"
#include "net/WebSvcReq.h"
#include "obj/Msg.h"
#include "os/Debug.h"
#include "os/OnlineID.h"
#include "utl/DataPointMgr.h"
#include "utl/MakeString.h"
#include "utl/MemMgr.h"
#include "utl/UrlEncode.h"

static const char *sStr = "1";

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

    const char *url = GetURL();
    const char *s = TheServer.GetUnk40();
    WebSvcRequest::SetURL(
        MakeString("/%s/%s/%s/%s", sStr, TheServer.GetPlatform(), s, url)
    );
    StartImpl();
}

void DingoJob::SendCallback(bool success, bool cancelled) {
    bool s = success;
    if (s) {
        ParseResponse();
        if (!mJsonResponse || mResult == -1 || mResult == -4 || mResult == -0xb
            || mResult == -0x138b) {
            s = false;
        }
    }
    if (mCallback) {
        static DingoJobCompleteMsg msg(this, false);
        msg[0] = this;
        msg[1] = s;
        mCallback->Handle(msg, true);
        if (!s) {
            if (TheServer.IsAuthenticated() && !cancelled) {
                DataPoint pt("dingo_job_failed");
                pt.AddPair("location", "DingoJob::SendCallback");
                pt.AddPair("mResult", mResult);
                pt.AddPair("mJsonResponse", mJsonResponse ? "non-NULL" : "NULL");
                pt.AddPair("mResponseStr", mResponseStr.c_str());
                pt.AddPair("mBaseUrl", mBaseUrl.c_str());
                pt.AddPair("mResponseStatusCode", (int)GetResponseStatusCode());
                pt.AddPair("session_id", TheServer.GetUnk40());
                OnlineID id = TheServer.GetOnlineID();
                pt.AddPair("player", id.ToString());
                pt.AddPair("severity", "warn");
                pt.AddPair("project", "sync");
                TheDataPointMgr.RecordDebugDataPoint(pt);
                TheWebSvcMgr.CancelOutstandingCalls();
                TheServer.Logout();
                static ServerStatusChangedMsg msg(kServerStatusDisconnected);
                TheServer.Export(msg, true);
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
    if (mResult == -3) {
        bool authenticating = TheServer.IsAuthenticating();
        if (!authenticating) {
            int padnum = TheServer.GetUnk74();
            TheServer.Logout();
            authenticating = TheServer.Authenticate(padnum);
        } else {
            authenticating = true;
        }

        if (authenticating) {
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
