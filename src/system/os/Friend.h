#pragma once
#include "obj/Msg.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"

// size 0x20
class Friend {
public:
    Friend() {}
    void SetName(String name) { mName = name; }
    const char *GetName() const { return mName.c_str(); }
    void SetOnline(bool online) { mOnline = online; }
    bool GetOnline() const { return mOnline; }
    void SetGame(String game) { mGame = game; }
    const char *GetGame() const { return mGame.c_str(); }
    void SetXUID(XUID xuid) { mXUID = xuid; }
    XUID GetXUID() const { return mXUID; }

    MEM_OVERLOAD(Friend, 0x1b)

private:
    String mName; // 0x0
    bool mOnline; // 0x8
    String mGame; // 0xc
    XUID mXUID; // 0x18
};

DECLARE_MESSAGE(FriendsListChangedMsg, "friends_list_changed")
FriendsListChangedMsg(int i) : Message(Type(), i) {}
END_MESSAGE
