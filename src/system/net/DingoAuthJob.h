#pragma once

#include "net/DingoJob.h"
#include "obj/Object.h"
#include "utl/DataPointMgr.h"
#include "utl/Str.h"

class AuthenticateReqJob : public DingoJob {
public:
    AuthenticateReqJob(const char *url, const DataPoint &pt, Hmx::Object *callback);
    virtual ~AuthenticateReqJob();
    virtual void Start();

    bool ParseResponse();

private:
    String mSessionID; // 0xb0
};
