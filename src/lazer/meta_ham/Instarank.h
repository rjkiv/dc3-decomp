#pragma once
#include "obj/Object.h"
#include "types.h"

class Instarank : public virtual Hmx::Object {
public:
    const char *Str() const;

private:
    bool mFriendMode; // 0x4
    char fmtToken; // 0x5
    char strArg[255]; // 0x6
    int rankArg; // 0x108
    int scoreArg; // 0x10c
};
