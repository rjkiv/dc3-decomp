#include "midi/DisplayEvents.h"
#include "math/Geo.h"
#include "math/Utl.h"
#include "rndobj/Rnd.h"

static const float kXOffset = 5;

float DisplayEvents(DataEventList *events, float f1, float f2) {
    float f9 = f1 + 2.0f;
    int min = Min(events->CurIndex(), events->Size() - 1);
    MaxEq(min, 0);
    while (min > 0 && events->Event(min - 1).end > f2)
        min--;
    while (min < events->Size() && events->Event(min).end < f2)
        min++;
    float f10 = -1.0f;
    float fsum = f2 + (float)TheRnd.Width() / 200.0f;
    Hmx::Rect r;
    for (; min < events->Size() && events->Event(min).start < fsum; min++) {
        DataEvent curEvent(events->Event(min));
        float start14c = (curEvent.start - f2) * 200.0f + kXOffset;
        float start150 = (curEvent.end - f2) * 200.0f + kXOffset;
        String str108;
        str108 << curEvent.Msg();
        MaxEq(start14c, kXOffset);
        MinEq(start150, (float)TheRnd.Width() + kXOffset);
        if (min < events->Size() - 1) {
            MinEq(
                start150, ((events->Event(min + 1).start - f2) * 200.0f + kXOffset) - 1.0f
            );
        }
        r.Set(start14c, f1 + 2.0f, Max(1.0f, start150 - start14c), 12.0f);
        TheRnd.DrawRect(r, Hmx::Color(0, 0, 1), 0, 0, 0);
        if (f10 == -1.0f) {
            f10 =
                Max(f10,
                    TheRnd
                        .DrawString(
                            MakeString("%.2f: %s", curEvent.start, str108.c_str()),
                            Vector2(start14c, f9 + 14.0f),
                            Hmx::Color(1, 1, 1),
                            true
                        )
                        .y);
        }
    }
    r.x = kXOffset;
    r.w = 1;
    r.y = f9 - 2;
    r.h = 14;
    TheRnd.DrawRect(r, Hmx::Color(1, 0, 0), 0, 0, 0);
    return f9 + f10;
}
