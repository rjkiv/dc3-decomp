#pragma once
#include "hamobj/Difficulty.h"
#include "hamobj/HamLabel.h"
#include "hamobj/StarsDisplay.h"
#include "obj/Data.h"
#include "obj/Object.h"

class HamStarsDisplay : public StarsDisplay {
public:
    enum StarDisplayMode {
        kStarDisplay_0,
        kStarDisplay_1,
        kStarDisplay_2,
        kStarDisplay_3,
        kStarDisplay_4
    };

    OBJ_CLASSNAME(StarsDisplay) // bruh
    virtual DataNode Handle(DataArray *, bool);

    HamStarsDisplay();
    void SetSongChallenge(Difficulty);
    void SetSong(int);
    void SetSongCampaign(int);
    void SetSongWithDifficulty(int, Difficulty, bool);

private:
    void SetSongImpl(int, Difficulty, StarDisplayMode);
};
