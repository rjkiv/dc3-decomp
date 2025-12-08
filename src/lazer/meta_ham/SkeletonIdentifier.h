#pragma once

#include "obj/Object.h"

class SkeletonIdentifier : public Hmx::Object {
public:
    SkeletonIdentifier();
    virtual ~SkeletonIdentifier();

    void Init();
    void Poll();
};

extern SkeletonIdentifier *TheSkeletonIdentifier;
