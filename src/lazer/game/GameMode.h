#pragma once

#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/Symbol.h"

class GameMode : public Hmx::Object {
public:
    // Hmx::Object
    virtual DataNode Handle(DataArray *, bool);

    GameMode();
    bool InMode(Symbol, bool);
    DataNode OnInMode(DataArray const *);
    int RequiresTwoPlayers(Symbol);
    int MinPlayers(Symbol);
    int MaxPlayers(Symbol);
    void SetMode(Symbol, Symbol);
    void SetGameplayMode(Symbol, bool);
    DataNode OnSetMode(DataArray const *);

    Symbol unk2c;
    Symbol unk30;
    Symbol unk34;
    int unk38;
    Symbol unk3c;
    int unk40;
    int unk44;
    int unk48;
    int unk4c;
    int unk50;
    int unk54;
    int unk58;
    int unk5c;
    int unk60;
    int unk64;
    int unk68;
    int unk6c;

protected:
    void FillModeArrayWithParentData(Symbol, DataArray *, DataArray *);
};

void GameModeTerminate();
void GameModeInit();
bool IsInLoaderMode(Symbol const &);

extern GameMode *TheGameMode;
