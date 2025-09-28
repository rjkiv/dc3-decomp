#pragma once
#include "obj/Data.h"
#include "obj/PropSync.h"
#include "utl/BinStream.h"
#include "utl/TimeConversion.h"

struct Range {
    Range() : start(0), end(0) {}
    int start;
    int end;
};

bool PropSync(Range &, DataNode &, DataArray *, int, PropOp);

float FrameToBeat(float secs) { return SecondsToBeat(secs * 0.033333335f); }
float BeatToFrame(float beat);
