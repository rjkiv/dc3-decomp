#include "net_ham/DataMinerJobs.h"
#include "net_ham/RCJobDingo.h"
#include "obj/Object.h"
#include "utl/DataPointMgr.h"
#include "utl/Symbol.h"

OmgScoresJob::OmgScoresJob(Hmx::Object *callback, int p1Score, int p2Score)
    : RCJob("dataminer/omg_scores/", callback) {
    static Symbol player1_score("player1_score");
    static Symbol player2_score("player2_score");
    DataPoint dataP;
    dataP.AddPair(player1_score, p1Score);
    dataP.AddPair(player2_score, p2Score);
    SetDataPoint(dataP);
}

PlayerDroppedInJob::PlayerDroppedInJob(Hmx::Object *callback, int playerIdx)
    : RCJob("dataminer/player_dropped_in/", callback) {
    static Symbol player_idx("player_idx");
    DataPoint dataP;
    dataP.AddPair(player_idx, playerIdx);
    SetDataPoint(dataP);
}

PlayerDroppedOutJob::PlayerDroppedOutJob(Hmx::Object *callback, int playerIdx)
    : RCJob("dataminer/player_dropped_out/", callback) {
    static Symbol player_idx("player_idx");
    DataPoint dataP;
    dataP.AddPair(player_idx, playerIdx);
    SetDataPoint(dataP);
}

ControllerModeJob::ControllerModeJob(Hmx::Object *callback, int enterCount, int exitCount)
    : RCJob("dataminer/controller_mode/", callback) {
    static Symbol enter_count("enter_count");
    static Symbol exit_count("exit_count");
    DataPoint dataP;
    dataP.AddPair(enter_count, enterCount);
    dataP.AddPair(exit_count, exitCount);
    SetDataPoint(dataP);
}

PlaylistChangedJob::PlaylistChangedJob(Hmx::Object *callback, Symbol name, int numSongs)
    : RCJob("dataminer/playlist_changed/", callback) {
    static Symbol playlist_name("playlist_name");
    static Symbol num_songs("num_songs");
    DataPoint dataP;
    dataP.AddPair(playlist_name, name);
    dataP.AddPair(num_songs, numSongs);
    SetDataPoint(dataP);
}

ScreenResJob::ScreenResJob(Hmx::Object *callback, _XVIDEO_MODE *videoMode)
    : RCJob("dataminer/screen_resolution/", callback) {
    DataPoint dataP;
    static Symbol dwDisplayWidth("dwDisplayWidth");
    dataP.AddPair(dwDisplayWidth, videoMode->dwDisplayWidth);
    static Symbol dwDisplayHeight("dwDisplayHeight");
    dataP.AddPair(dwDisplayHeight, videoMode->dwDisplayHeight);
    static Symbol fIsInterlaced("fIsInterlaced");
    dataP.AddPair(fIsInterlaced, videoMode->fIsInterlaced);
    static Symbol fIsWideScreen("fIsWidescreen");
    dataP.AddPair(fIsWideScreen, videoMode->fIsWideScreen);
    static Symbol fIsHiDef("fIsHiDef");
    dataP.AddPair(fIsHiDef, videoMode->fIsHiDef);
    static Symbol RefreshRate("RefreshRate");
    dataP.AddPair(RefreshRate, videoMode->RefreshRate);
    static Symbol VideoStandard("VideoStandard");
    dataP.AddPair(VideoStandard, videoMode->VideoStandard);
    SetDataPoint(dataP);
}
