#pragma once
#include "net_ham/RCJobDingo.h"
#include "obj/Object.h"

class GetMotdJob : public RCJob {
public:
    GetMotdJob(Hmx::Object *);
    void GetMotdData(
        unsigned int &,
        int &,
        bool &,
        int &,
        std::vector<String> &,
        String &,
        String &,
        String &,
        String &,
        String &,
        String &,
        String &
    );
};
