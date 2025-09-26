#include "DebugMeter.h"
#include "math/Geo.h"
#include <system/rndobj/Rnd.h>

void DebugMeter::Draw() { DrawBar(0.0f, 1.0f, color, 1.0f, 0.0f); }

void DebugMeter::DrawBar(
    float startNorm, float endNorm, const Hmx::Color color, float scaleX, float scaleY
) {
    float rectWidth = width * endNorm;
    float rectHeight = height * scaleX;
    float rectX = width * startNorm + x;
    float rectY = height * scaleY + y;

    TheRnd.DrawRectScreen(
        Hmx::Rect(rectX, rectY, rectWidth, rectHeight), color, nullptr, nullptr, nullptr
    );
}

void DebugMeter::DrawLine(float startNorm, Hmx::Color color, float scaleX, float scaleY) {
    DrawBar(startNorm, 0.001f, color, scaleX, scaleY);
}

void DebugMeter::DrawText(const char *text, float relX, float relY, Hmx::Color color) {
    Vector2 pos;
    pos.x = width * relX + x;
    pos.y = height * relY + y;
    TheRnd.DrawStringScreen(text, pos, color, true);
}
