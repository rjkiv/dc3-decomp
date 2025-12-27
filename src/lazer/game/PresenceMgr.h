#pragma once

#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "utl/Symbol.h"
class PresenceMgr : public Hmx::Object {
public:
    virtual DataNode Handle(DataArray *, bool);

    void Init();
    void SetNotInGame();
    void SetInGame(int);

protected:
    bool IsPadPlaying(int);
    Symbol GetPresenceMode();
    int GetPresenceContextFromMode(Symbol, bool);
    int GetPlayModeContext();
    void UpdatePresence();
    DataNode OnPlayerPresentChange(DataArray *);
    DataNode OnPresenceChange(DataArray *);

    DataArray *unk2c;
    DataArray *unk30;
    DataArray *unk34;
    u32 unk38;
    int mSongID; // 0x3c
    bool mInGame; // 0x40
};

DECLARE_MESSAGE(CurrentScreenChangedMsg, "current_screen_changed")
END_MESSAGE
