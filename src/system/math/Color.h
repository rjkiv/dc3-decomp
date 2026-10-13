#pragma once
#include "types.h"
#include "utl/TextStream.h"
#include "utl/BinStream.h"
#include "math/Utl.h"
#include "math/Vec.h"

namespace Hmx {
    class Color {
    public:
        static Color kWhite;
        static Color kTransparent;
        static Color kBlack;
        float red;
        float green;
        float blue;
        float alpha;

        Color() {}
        Color(float r, float g, float b) : red(r), green(g), blue(b), alpha(1.0f) {}
        Color(float r, float g, float b, float a) : red(r), green(g), blue(b), alpha(a) {}
        Color(int packed) : alpha(1.0f) { UnpackRgb(packed); }

        Hmx::Color &Set(float r, float g, float b) {
            red = r;
            green = g;
            blue = b;
            return *this;
        }

        Hmx::Color &Set(float r, float g, float b, float a) {
            red = r;
            green = g;
            blue = b;
            alpha = a;
            return *this;
        }

        Hmx::Color &SetRgb(const Hmx::Color &c) {
            red = c.red;
            green = c.green;
            blue = c.blue;
            return *this;
        }

        bool SameRgb(const Hmx::Color &c) const {
            return red == c.red && green == c.green && blue == c.blue;
        }
        bool operator==(const Color &c) const {
            return red == c.red && green == c.green && blue == c.blue && alpha == c.alpha;
        }
        bool operator!=(const Color &c) const { return !(*this == c); }

        void Clear() { red = green = blue = alpha = 1.0f; }
        void Zero() { red = green = blue = alpha = 0; }

        int PackRgb() const {
            return (((int)(blue * 255.0f) & 0xFF) << 16)
                | ((int)(green * 255.0f) & 0xFF) << 8 | ((int)(red * 255.0f) & 0xFF);
        }

        int PackRgba() const {
            return ((int)(alpha * 255.0f) & 0xFF) << 24
                | ((int)(blue * 255.0f) & 0xFF) << 16
                | ((int)(green * 255.0f) & 0xFF) << 8 | ((int)(red * 255.0f) & 0xFF);
        }

        void UnpackRgb(int packed) {
            red = (packed & 255) / 255.0f;
            green = ((packed >> 8) & 255) / 255.0f;
            blue = ((packed >> 0x10) & 255) / 255.0f;
        }

        void UnpackRgba(int packed) {
            red = (packed & 255) / 255.0f;
            green = ((packed >> 8) & 255) / 255.0f;
            blue = ((packed >> 0x10) & 255) / 255.0f;
            alpha = ((packed >> 0x18) & 255) / 255.0f;
        }

        const float *AsArray() const { return &red; }
        float *AsArray() { return &red; }

        Vector3 &AsVector3() { return reinterpret_cast<Vector3 &>(*this); }
        const Vector3 &AsVector3() const {
            return reinterpret_cast<const Vector3 &>(*this);
        }
        Vector4 &AsVector4() { return reinterpret_cast<Vector4 &>(*this); }
        const Vector4 &AsVector4() const {
            return reinterpret_cast<const Vector4 &>(*this);
        }
    };
}

/** Given a color, return the corresponding HSL values. */
void MakeHSL(const Hmx::Color &color, float &h, float &s, float &l);
/** Given HSL values, create an RGB Color. */
void MakeColor(float h, float s, float l, Hmx::Color &color);
TextStream &operator<<(TextStream &, const Hmx::Color &);

inline BinStream &operator<<(BinStream &bs, const Hmx::Color &color) {
    bs << color.red << color.green << color.blue << color.alpha;
    return bs;
}

inline BinStream &operator>>(BinStream &bs, Hmx::Color &color) {
    bs >> color.red >> color.green >> color.blue >> color.alpha;
    return bs;
}

inline void Add(const Hmx::Color &c1, const Hmx::Color &c2, Hmx::Color &res) {
    res.Set(c1.red + c2.red, c1.green + c2.green, c1.blue + c2.blue, c1.alpha + c2.alpha);
}

inline void Subtract(const Hmx::Color &c1, const Hmx::Color &c2, Hmx::Color &res) {
    res.Set(c1.red - c2.red, c1.green - c2.green, c1.blue - c2.blue, c1.alpha - c2.alpha);
}

inline void Multiply(const Hmx::Color &c1, float f, Hmx::Color &res) {
    res.Set(c1.red * f, c1.green * f, c1.blue * f, c1.alpha * f);
}

inline Hmx::Color &Average(Hmx::Color &res, const Hmx::Color &c1, const Hmx::Color &c2) {
    res.Set(
        (c1.red + c2.red) / 2,
        (c1.green + c2.green) / 2,
        (c1.blue + c2.blue) / 2,
        (c1.alpha + c2.alpha) / 2
    );
    return res;
}

inline void Interp(const Hmx::Color &c1, const Hmx::Color &c2, float f, Hmx::Color &res) {
    res.Set(
        Interp(c1.red, c2.red, f),
        Interp(c1.green, c2.green, f),
        Interp(c1.blue, c2.blue, f),
        Interp(c1.alpha, c2.alpha, f)
    );
}
