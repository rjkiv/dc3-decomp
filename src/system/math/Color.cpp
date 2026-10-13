#include "math/Color.h"
#include "utl/TextStream.h"

TextStream &operator<<(TextStream &ts, const Hmx::Color &color) {
    ts << "(r:" << color.red << " g:" << color.green << " b:" << color.blue
       << " a:" << color.alpha << ")";
    return ts;
}

void MakeColor(float h, float s, float l, Hmx::Color &color) {
    if (s == 0) {
        color.Set(l, l, l);
        return;
    }
    float q; // upper lightness mix
    if (l < 0.5f) {
        q = (s + 1.0f) * l;
    } else {
        q = -(s * l - (s + l));
    }
    float p = l * 2.0f - q; // lower lightness mix
    float *colorArr = color.AsArray();
    for (int i = 0; i < 3; i++) {
        float t; // hue offset per channel
        if (i == 0) {
            t = h + (1.0f / 3.0f);
        } else if (i == 1) {
            t = h;
        } else {
            t = h - (1.0f / 3.0f);
        }

        if (t < 0) {
            t += 1;
        } else if (t > 1) {
            t -= 1;
        }

        if (t * 6.0f < 1.0f) {
            colorArr[i] = (q - p) * t * 6.0f + p;
        } else if (t * 2.0f < 1.0f) {
            colorArr[i] = q;
        } else if (t * 3.0f < 2.0f) {
            t = (2.0f / 3.0f) - t;
            colorArr[i] = (q - p) * t * 6.0f + p;
        } else {
            colorArr[i] = p;
        }
    }
}

void MakeHSL(const Hmx::Color &color, float &h, float &s, float &l) {
    float maxCol = Max(color.red, color.green, color.blue);
    float minCol = Min(color.red, color.green, color.blue);
    l = (maxCol + minCol) / 2.0f;
    if (maxCol == minCol) {
        h = 0;
        s = 0;
    } else {
        float deltaCol = maxCol - minCol;
        if (l < 0.5f)
            s = deltaCol / (minCol + maxCol);
        else
            s = deltaCol / ((2.0f - maxCol) - minCol);
        if (color.red == maxCol) {
            h = (color.green - color.blue) / deltaCol;
        } else if (color.green == maxCol) {
            h = (color.blue - color.red) / deltaCol + 2.0f;
        } else {
            h = (color.red - color.green) / deltaCol + 4.0f;
        }
        h /= 6.0f;
        if (h < 0.0f)
            h += 1.0f;
    }
}
