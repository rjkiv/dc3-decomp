#pragma once
#include "utl/SongPos.h"
#include "utl/BeatMap.h"
#include "utl/TempoMap.h"
#include "utl/MeasureMap.h"

class HxSongData {
public:
    HxSongData() {}
    virtual ~HxSongData() {}
    virtual SongPos CalcSongPos(float) = 0;
    virtual TempoMap *GetTempoMap() const = 0;
    virtual BeatMap *GetBeatMap() const = 0;
    virtual MeasureMap *GetMeasureMap() const = 0;
};
