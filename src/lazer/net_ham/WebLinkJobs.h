#pragma once
#include "RCJobDingo.h"
#include "obj/Object.h"

class GetWebLinkCodeJob : public RCJob {
public:
    GetWebLinkCodeJob(Hmx::Object *, int);
    bool GetWebLinkCodeData(String &);
};
