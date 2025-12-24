#pragma once
#include "hamobj/Difficulty.h"
#include "hamobj/HamLabel.h"
#include "hamobj/StarsDisplay.h"
#include "obj/Data.h"

class HamStarsDisplay : public StarsDisplay {
public:
    enum StarDisplayMode {
    };

    virtual DataNode Handle(DataArray *, bool);

    HamStarsDisplay();
    void SetSongChallenge(Difficulty);
    void SetSong(int);
    void SetSongCampaign(int);
    void SetSongWithDifficulty(int, Difficulty, bool);

private:
    void SetSongImpl(int, Difficulty, StarDisplayMode);
};
