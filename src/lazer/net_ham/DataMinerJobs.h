#pragma once
#include "meta_ham/MetaPerformer.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Object.h"

class GameEndedDataPointJob : public RCJob {
public:
    GameEndedDataPointJob(Hmx::Object *, EndGameResult const &);
};

class OmgScoresJob : public RCJob {
public:
    OmgScoresJob(Hmx::Object *, int, int);
};

class PlayerDroppedInJob : public RCJob {
public:
    PlayerDroppedInJob(Hmx::Object *, int);
};

class PlayerDroppedOutJob : public RCJob {
public:
    PlayerDroppedOutJob(Hmx::Object *, int);
};

class ControllerModeJob : public RCJob {
public:
    ControllerModeJob(Hmx::Object *, int, int);
};

class PlaylistChangedJob : public RCJob {
public:
    PlaylistChangedJob(Hmx::Object *, Symbol, int);
};

class ScreenResJob : public RCJob {
public:
    ScreenResJob(Hmx::Object *, _XVIDEO_MODE *);
};
