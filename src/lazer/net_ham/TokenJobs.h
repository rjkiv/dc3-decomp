#pragma once
#include "RCJobDingo.h"

class RedeemTokenJob : public RCJob {
public:
    RedeemTokenJob(Hmx::Object *callback, int, String);
    bool GetRedeemTokenData(int &, String &);
};
