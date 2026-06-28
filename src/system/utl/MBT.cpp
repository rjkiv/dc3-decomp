#include "utl/MBT.h"
#include "os/Debug.h"
#include <cmath>

const char *TickFormat(int tick, const MeasureMap &map) {
    int m, b, t;
    if (tick >= 0) {
        map.TickToMeasureBeatTick(tick, m, b, t);
        return MakeString("%d:%d:%03d", m + 1, b + 1, t);
    } else
        return "negative tick";
}

const char *FormatTimeMSH(float ms) {
    int first = ms / 60000.0f;
    int second = fmodf(ms, 60000.0) / 1000.0f;
    int third = fmodf(ms, 1000.0) / 10.0f;
    return MakeString("%d:%02d.%02d", first, second, third);
}
