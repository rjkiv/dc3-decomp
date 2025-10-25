#include "net/DingoAuthJob.h"
#include "DingoAuthJob.h"
#include "DingoSvr.h"
#include "net/DingoJob.h"
#include "net/JsonUtils.h"
#include "os/Debug.h"
#include "utl/MakeString.h"

AuthenticateReqJob::AuthenticateReqJob(
    char const *c, DataPoint const &point, Hmx::Object *o
)
    : DingoJob(c, o) {
    DingoJob::SetDataPoint(point);
}

AuthenticateReqJob::~AuthenticateReqJob() {}

void AuthenticateReqJob::Start() {
    MILO_ASSERT(GetURL(), 0x24);
    MILO_ASSERT(strlen(GetURL()) != 0, 0x25);
}

bool AuthenticateReqJob::ParseResponse() {
    unkb0 = "";
    if (unka4 && unka8 == 1) {
        JsonObject *o = unk90.GetByName(unka4, "session_id");
        if (o) {
            unkb0 = o->Str();
            return true;
        }

    } else {
        MILO_NOTIFY(
            "AuthenticateReqJob: New version of Authenticate response API!  Needs attention!"
        );
    }
    return false;
}
