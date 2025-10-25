#pragma once

#include "net/DingoJob.h"
#include "obj/Object.h"
#include "utl/DataPointMgr.h"
#include "utl/Str.h"

class AuthenticateReqJob : public DingoJob {
public:
    virtual ~AuthenticateReqJob();
    virtual void Start();

    AuthenticateReqJob(char const *, DataPoint const &, Hmx::Object *);
    bool ParseResponse();

    String unkb0;
    static const char *unkb8;
};
