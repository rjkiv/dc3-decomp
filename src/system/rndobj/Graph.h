#pragma once
#include "utl/MemMgr.h"
#include <vector>
#include "math/Vec.h"
#include "math/Color.h"
#include "math/Geo.h"
#include "rndobj/Cam.h"

class RndGraph {
public:
    void Reset();
    void AddLine(const Vector3 &, const Vector3 &, const Hmx::Color &, bool);
    void AddScreenLine(const Vector2 &, const Vector2 &, const Hmx::Color &, bool);
    void AddRectFilled2D(const Hmx::Rect &, const Hmx::Color &);
    void AddSphere(const Vector3 &, float, const Hmx::Color &);
    void AddString(const char *, const Vector2 &, const Hmx::Color &);
    void AddString3D(const char *, const Vector3 &, const Hmx::Color &);
    void AddScreenString(const char *, const Vector2 &, const Hmx::Color &);

    static void Init();
    static void Terminate();
    static void ResetAll();
    static void DrawAll();
    static RndGraph *Get(const void *);
    static void Free(const void *, bool);
    static void SetCamera(RndCam *);
    static RndGraph *GetOneFrame();

protected:
    RndGraph(const void *cv) : mEnable(1), mDrawFixedZ(0), mZ(0.0f), mId((void *)cv) {}
    ~RndGraph();

    void Draw();

    MEM_OVERLOAD(RndGraph, 0x1C);

private:
    bool mEnable; // 0x0
    bool mDrawFixedZ; // 0x1
    float mZ; // 0x4
    void *mId; // 0x8
    std::vector<class Drawable *> mStuff; // 0xc
};

struct FakeGraph {
    void *mId; // 0x0
    void (*mDrawCallback)(); // 0x4
};
