#pragma once
#include "obj/Msg.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"

// size 0x20
class Friend {
public:
    Friend();
    void SetName(String name) { mName = name; }

    MEM_OVERLOAD(Friend, 0x1b)

    String mName; // 0x0
    String unk8; // 0x8
};

DECLARE_MESSAGE(FriendsListChangedMsg, "friends_list_changed")
FriendsListChangedMsg(int i) : Message(Type(), i) {}
END_MESSAGE
