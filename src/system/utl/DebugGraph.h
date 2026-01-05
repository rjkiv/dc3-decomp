#pragma once
#include "math/Geo.h"
#include "math/Color.h"


class DebugGraph {
public:
    struct Sample {
        float data;
        bool b;
    };
    DebugGraph(float f1, float f2, float f3, float f4, Hmx::Color c1, Hmx::Color c2, int i1, float f5, float f6, String s);
    ~DebugGraph();
    void AddData(float, bool);
    void Draw();
    void SetUnk50(bool b) { unk50 = b; }
    bool GetUnk50() { return unk50; }
    void SetUnk44(float f) { unk44 = f; }
    int GetUnk38() { return unk38; }

protected:
    Hmx::Rect mRect; // 0x0
    Hmx::Color mColorA; // 0x10
    Hmx::Color mColorB; // 0x20
    std::list<Sample> mSamples; // 0x30
    int unk38; // 0x38
    float unk3c; // 0x3c
    float unk40; // 0x40
    float unk44; // 0x44
    String unk48; // 0x48
    bool unk50; // 0x50

};