#pragma once
#include "hamobj/Difficulty.h"
#include "hamobj/HamLabel.h"
#include "hamobj/StarsDisplay.h"
#include "obj/Data.h"
#include "obj/Object.h"

class HamStarsDisplay : public StarsDisplay {
public:
    enum StarDisplayMode {
        kStarDisplayMode_Normal = 0x0000,
        kStarDisplayMode_Difficulty = 0x0001,
        kStarDisplayMode_ForceShowDifficulty = 0x0002,
        kStarDisplayMode_LastPlayed = 0x0003,
        kStarDisplayMode_Campaign = 0x0004
    };

    OBJ_CLASSNAME(StarsDisplay) // bruh
    OBJ_SET_TYPE(StarsDisplay)
    virtual DataNode Handle(DataArray *, bool);

    void SetSongChallenge(Difficulty);
    void SetSong(int);
    void SetSongCampaign(int);
    void SetSongWithDifficulty(int, Difficulty, bool);
    void SetSongLastPlayed(int);

    NEW_OBJ(HamStarsDisplay)

private:
    void SetSongImpl(int, Difficulty, StarDisplayMode);
};
