#pragma once
#include "obj/Data.h"
#include "obj/PropSync.h"
#include "utl/BinStream.h"

struct Range {
    Range() : start(0), end(0) {}
    int start;
    int end;
};

bool PropSync(Range &, DataNode &, DataArray *, int, PropOp);
