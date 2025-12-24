#pragma once
#include "RCJobDingo.h"

class RedeemTokenJob : public RCJob {
public:
    RedeemTokenJob(Hmx::Object *, int, String);
    bool GetRedeemTokenData(int &, String &);
};
