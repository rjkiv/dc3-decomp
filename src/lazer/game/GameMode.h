#pragma once

#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/Symbol.h"

class GameMode : public Hmx::Object {
public:
    GameMode();
    // Hmx::Object
    virtual ~GameMode() {}
    virtual DataNode Handle(DataArray *, bool);

    bool InMode(Symbol, bool = true);
    int RequiresTwoPlayers(Symbol);
    int MinPlayers(Symbol);
    int MaxPlayers(Symbol);
    void SetMode(Symbol, Symbol);
    void SetGameplayMode(Symbol, bool);

    void SetInPartyMode(bool mode) { mInPartyMode = mode; }
    bool InPartyMode() const { return mInPartyMode; }
    int Infinite() const { return mInfinite; }

    Symbol GameplayMode() const { return mGameplayMode; }
    bool IsGameplayModePerform() const;
    bool IsGameplayModePractice() const {
        static Symbol practice("practice");
        return mGameplayMode == practice;
    }
    bool IsGameplayModeBustamove() const {
        static Symbol bustamove("bustamove");
        return mGameplayMode == bustamove;
    }
    bool IsGameplayModeRhythmBattle() const {
        static Symbol rhythm_battle("rhythm_battle");
        return mGameplayMode == rhythm_battle;
    }
    bool IsGameplayModeDanceBattle() const {
        static Symbol dance_battle("dance_battle");
        return mGameplayMode == dance_battle;
    }
    Symbol Mode() const { return mMode; }

    DataNode OnInMode(const DataArray *);
    DataNode OnSetMode(const DataArray *);

private:
    Symbol mMode; // 0x2c
    Symbol mBattleMode; // 0x30
    Symbol mParentMode; // 0x34
    int mParentOnly; // 0x38
    Symbol mGameplayMode; // 0x3c
    int mCanLose; // 0x40
    int mPauseCountIn; // 0x44
    int mRequires2Players; // 0x48
    int mCrowdReacts; // 0x4c
    int mLoadChars; // 0x50
    int mUseStaticTip; // 0x54
    int mRanked; // 0x58
    int mUpdateLeaderboards; // 0x5c
    int mInfinite; // 0x60
    int mMinPlayers; // 0x64
    int mMaxPlayers; // 0x68
    int mInPartyMode; // 0x6c

protected:
    void FillModeArrayWithParentData(Symbol, DataArray *, DataArray *);
};

void GameModeInit();
void GameModeTerminate();
bool IsInLoaderMode(const Symbol &);

extern GameMode *TheGameMode;
extern bool (*g_LoaderModeCallback)(const Symbol &);
