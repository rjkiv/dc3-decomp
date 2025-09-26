#pragma once
#include <system/math/Color.h>
#include <rndobj/Rnd.h>

class DebugMeter {
public:
    void DrawBar(
        float startNorm,
        float endNorm,
        Hmx::Color color,
        float scaleX = 1.0f,
        float scaleY = 0.0f
    );
    void DrawLine(float, Hmx::Color, float, float);
    void DrawText(char const *, float, float, Hmx::Color);
    void Draw();

private:
    float x;
    float y;
    float width;
    float height;
    Hmx::Color color;
};
