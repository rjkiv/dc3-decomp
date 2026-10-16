#include "rndobj/Graph.h"
#include "Rnd.h"
#include "obj/DataFunc.h"
#include "os/Debug.h"
#include "rndobj/Utl.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"

static RndGraph *sOneFrame = nullptr;
static std::list<RndGraph *> *sGraphs = nullptr;
static std::list<FakeGraph> sFakes;
static ObjPtr<RndCam> sCam(nullptr);

class Drawable {
public:
    Drawable() {}
    virtual ~Drawable() {}
    virtual void Draw() = 0;
    virtual void DrawFixedZ(float) = 0;

    MEM_OVERLOAD(Drawable, 0xA9);
};

class Line : public Drawable {
public:
    Line(const Vector3 &va, const Vector3 &vb, const Hmx::Color &clr, bool b)
        : mA(va), mB(vb), mCol(clr), mZBuf(b) {}
    virtual void Draw() { TheRnd.DrawLine(mA, mB, mCol, mZBuf); }
    virtual void DrawFixedZ(float f) {
        TheRnd.DrawLine(Vector3(mA.x, mA.y, f), Vector3(mB.x, mB.y, f), mCol, mZBuf);
    }

    Vector3 mA;
    Vector3 mB;
    Hmx::Color mCol;
    bool mZBuf;
};

class ScreenLine : public Drawable {
public:
    ScreenLine(const Vector2 &va, const Vector2 &vb, const Hmx::Color &clr, bool b)
        : mA(va), mB(vb), mCol(clr), mZBuf(b) {}
    virtual void Draw() { UtilDrawLine(mA, mB, mCol); }
    virtual void DrawFixedZ(float) { UtilDrawLine(mA, mB, mCol); }

    Vector2 mA;
    Vector2 mB;
    Hmx::Color mCol;
    bool mZBuf;
};

class RectFilled2D : public Drawable {
public:
    RectFilled2D(const Hmx::Rect &r, const Hmx::Color &c) : mRect(r), mCol(c) {}
    virtual void Draw() { TheRnd.DrawRectScreen(mRect, mCol, nullptr, nullptr, nullptr); }
    virtual void DrawFixedZ(float) {
        TheRnd.DrawRectScreen(mRect, mCol, nullptr, nullptr, nullptr);
    }

    Hmx::Rect mRect; // 0x4
    Hmx::Color mCol; // 0x14
};

class DrawString : public Drawable {
public:
    DrawString(const char *cc, const Vector2 &v, const Hmx::Color &clr)
        : mPos(v), mText(cc), mCol(clr) {}
    virtual void Draw() { TheRnd.DrawString(mText.c_str(), mPos, mCol, true); }
    virtual void DrawFixedZ(float) { Draw(); }

    Vector2 mPos;
    String mText;
    Hmx::Color mCol;
};

class DrawString3D : public Drawable {
public:
    DrawString3D(const char *cc, const Vector3 &v, const Hmx::Color &clr)
        : mPos(v), mText(cc), mCol(clr) {}
    virtual void Draw() { UtilDrawString(mText.c_str(), mPos, mCol); }
    virtual void DrawFixedZ(float) { Draw(); }

    Vector3 mPos;
    String mText;
    Hmx::Color mCol;
};

class DrawSphere : public Drawable {
public:
    DrawSphere(const Vector3 &v, float r, const Hmx::Color &clr)
        : mCenter(v), mRadius(r), mCol(clr) {}
    virtual void Draw() { UtilDrawSphere(mCenter, mRadius, mCol, nullptr); }
    virtual void DrawFixedZ(float f) {
        UtilDrawSphere(Vector3(mCenter.x, mCenter.z, f), mRadius, mCol, nullptr);
    }

    Vector3 mCenter; // 0x4
    float mRadius; // 0x14
    Hmx::Color mCol; // 0x18
};

void RndGraph::Draw() {
    for (int i = 0; i < mStuff.size(); i++) {
        if (mDrawFixedZ) {
            mStuff[i]->DrawFixedZ(mZ);
        } else
            mStuff[i]->Draw();
    }
}

void RndGraph::SetCamera(RndCam *c) { sCam = c; }

RndGraph *RndGraph::GetOneFrame() {
    if (sOneFrame == nullptr)
        sOneFrame = new RndGraph(nullptr);
    return sOneFrame;
}

RndGraph *RndGraph::Get(const void *v) {
    FOREACH_PTR (it, sGraphs) {
        if ((*it)->mId == v)
            return *it;
    }
    RndGraph *graph = new RndGraph(v);
    sGraphs->push_back(graph);
    return graph;
}

void RndGraph::AddLine(
    const Vector3 &a, const Vector3 &b, const Hmx::Color &col, bool buf
) {
    if (mEnable)
        mStuff.push_back(new Line(a, b, col, buf));
}

void RndGraph::AddScreenLine(
    const Vector2 &a, const Vector2 &b, const Hmx::Color &col, bool buf
) {
    if (mEnable)
        mStuff.push_back(new ScreenLine(a, b, col, buf));
}

void RndGraph::AddRectFilled2D(const Hmx::Rect &r, const Hmx::Color &c) {
    if (mEnable)
        mStuff.push_back(new RectFilled2D(r, c));
}

void RndGraph::AddSphere(const Vector3 &center, float radius, const Hmx::Color &col) {
    if (mEnable)
        mStuff.push_back(new DrawSphere(center, radius, col));
}

void RndGraph::AddString(const char *cc, const Vector2 &vec, const Hmx::Color &col) {
    if (mEnable)
        mStuff.push_back(new DrawString(cc, vec, col));
}

void RndGraph::AddString3D(
    const char *text, const Vector3 &worldPos, const Hmx::Color &col
) {
    if (mEnable)
        mStuff.push_back(new DrawString3D(text, worldPos, col));
}

void RndGraph::AddScreenString(const char *cc, const Vector2 &v, const Hmx::Color &c) {
    if (mEnable) {
        mStuff.push_back(
            new DrawString(cc, Vector2(v.x * TheRnd.Width(), v.y * TheRnd.Height()), c)
        );
    }
}

void RndGraph::Reset() {
    if (mEnable) {
        for (int i = 0; i < mStuff.size(); i++) {
            delete mStuff[i];
        }
        mStuff.clear();
    }
}

void RndGraph::DrawAll() {
    if (sCam)
        sCam->Select();
    FOREACH_PTR (it, sGraphs) {
        (*it)->Draw();
    }
    if (sOneFrame) {
        sOneFrame->Draw();
        sOneFrame->Reset();
    }
    FOREACH (it, sFakes) {
        it->mDrawCallback();
    }
    sCam = nullptr;
}

RndGraph::~RndGraph() {
    mEnable = true;
    Reset();
}

void RndGraph::ResetAll() {
    RELEASE(sOneFrame);
    while (!sGraphs->empty()) {
        RndGraph *cur = sGraphs->front();
        sGraphs->pop_front();
        delete cur;
    }
}

void RndGraph::Free(const void *id, bool notify) {
    if (sGraphs == nullptr)
        return;
    FOREACH_PTR (it, sGraphs) {
        if ((*it)->mId == id) {
            delete *it;
            sGraphs->erase(it);
            return;
        }
    }
    FOREACH (it, sFakes) {
        if (it->mId == id) {
            sFakes.erase(it);
            return;
        }
    }
    if (notify)
        MILO_NOTIFY("could not find line graph %x to free\n", (int)id);
}

DataNode OnGraphReset(DataArray *) {
    RndGraph::ResetAll();
    return 0;
}

void RndGraph::Terminate() {
    RELEASE(sGraphs);
    RELEASE(sOneFrame);
}

void RndGraph::Init() {
    sGraphs = new std::list<RndGraph *>;
    TheDebug.AddExitCallback(RndGraph::Terminate);
    DataRegisterFunc("graph_reset", OnGraphReset);
}
