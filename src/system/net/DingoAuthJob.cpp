#include "net/DingoAuthJob.h"
#include "DingoAuthJob.h"
#include "DingoSvr.h"
#include "net/DingoJob.h"
#include "net/JsonUtils.h"
#include "os/Debug.h"
#include "utl/MakeString.h"

AuthenticateReqJob::AuthenticateReqJob(
    char const *url, const DataPoint &point, Hmx::Object *callback
)
    : DingoJob(url, callback) {
    DingoJob::SetDataPoint(point);
}

AuthenticateReqJob::~AuthenticateReqJob() {}

void AuthenticateReqJob::Start() {
    MILO_ASSERT(GetURL(), 0x24);
    MILO_ASSERT(strlen(GetURL()) != 0, 0x25);
    SetURL(MakeString("/%s/%s/%s", "1", TheServer.GetPlatform(), GetURL()));
    StartImpl();
}

bool AuthenticateReqJob::ParseResponse() {
    mSessionID = "";
    if (mJsonResponse) {
        if (mJsonResponseVersion == 1) {
            JsonObject *o = mJsonReader.GetByName(mJsonResponse, "session_id");
            if (o) {
                mSessionID = o->Str();
                return true;
            }
        } else {
            MILO_NOTIFY(
                "AuthenticateReqJob: New version of Authenticate response API!  Needs attention!"
            );
        }
    }
    return false;
}
