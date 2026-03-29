#pragma once
#include "types.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"
#include "xdk/XAPILIB.h"

// size 0x18
class OnlineID {
private:
    friend BinStream &operator<<(BinStream &, const OnlineID &);

    XUID mXUID; // 0x0
    String mPlayerName; // 0x8
    bool mValid; // 0x10
public:
    OnlineID();
    OnlineID(const XUID &);
    void Clear();
    void SetXUID(const XUID &);
    void SetPlayerName(const char *);
    XUID GetXUID() const;
    const char *ToString() const;
    bool GetIsValid() const { return mValid; }

    MEM_OVERLOAD(OnlineID, 0x1E)
};

// BinStream &operator>>(BinStream &, OnlineID &);
