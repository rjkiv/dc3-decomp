#pragma once
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "utl/Symbol.h"

class PresenceMgr : public Hmx::Object {
public:
    PresenceMgr()
        : mDataModes(0), mDataModeContextsMap(0), mDataInstrumentContextsMap(0),
          mSongID(0), mInGame(0) {}
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

private:
    DataArray *mDataModes; // 0x2c
    DataArray *mDataModeContextsMap; // 0x30
    DataArray *mDataInstrumentContextsMap; // 0x34
    Symbol mSymPrevMode; // 0x38
    int mSongID; // 0x3c
    bool mInGame; // 0x40
};

extern PresenceMgr ThePresenceMgr;

DECLARE_MESSAGE(CurrentScreenChangedMsg, "current_screen_changed")
CurrentScreenChangedMsg(Symbol s) : Message(Type(), s) {}
END_MESSAGE
