#pragma once
#include "hamobj/Difficulty.h"
#include "hamobj/HamLabel.h"
#include "hamobj/StarsDisplay.h"
#include "obj/Data.h"
#include "obj/Object.h"

class HamStarsDisplay : public StarsDisplay {
public:
    enum StarDisplayMode {
        kStarDisplayStandard = 0,
        kStarDisplayWithDiff = 1,
        kStarDisplayWithDiffAlways = 2,
        kStarDisplayLastPlayed = 3,
        kStarDisplayCampaign = 4
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
