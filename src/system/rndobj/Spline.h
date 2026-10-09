#pragma once
#include "obj/Data.h"
#include "rndobj/Poll.h"
#include "utl/BinStream.h"

/** "sexy sexy cubic spline"
    I am not making this up, this is actually what HMX put
    in their original description for a RndSpline.
*/
class RndSpline : public RndPollable {
public:
    // size 0x58
    class CtrlPoint {
    public:
        CtrlPoint();
        void Save(BinStream &) const;
        void Load(BinStreamRev &);
        void Interp(const CtrlPoint &, const CtrlPoint &, float);

        Vector3 mPos; // 0x0
        float mRoll; // 0x10
        bool unk14;
        bool mDirtyConstants; // 0x15
        Vector4 unk18;
        Vector4 unk28;
        Vector4 unk38;
        Vector4 unk48;
    };
    // Hmx::Object
    virtual ~RndSpline() {
        if (this == sGlobalDefaultSpline) {
            sGlobalDefaultSpline = nullptr;
        }
    }
    OBJ_CLASSNAME(Spline);
    OBJ_SET_TYPE(Spline);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndPollable
    virtual void Poll();

    OBJ_MEM_OVERLOAD(0x18);
    NEW_OBJ(RndSpline)
    static void Init() { REGISTER_OBJ_FACTORY(RndSpline) }
    static RndSpline *GlobalDefaultSpline() { return sGlobalDefaultSpline; }

    void SetStartCtrlPoint(int);
    void SetEndCtrlPoint(int);
    void PrepareShader(float, float) const;
    void PrepareShader() const {
        PrepareShader(Max(mStartCtrlPoint, 0) * mYPerCtrlPoint - mYOffset, mYPerCtrlPoint);
    }
    int NumCtrlPts() const { return mCtrlPoints.size(); }
    bool Manual() const { return mManual; }
    bool HasPulse() const { return mPulse; }

    const CtrlPoint &GetDeformedCtrlPoint(int) const;

protected:
    RndSpline();

private:
    void SyncPristineCtrlPoints();
    void SyncDeformedCtrlPoints(int, int) const;
    void SyncDeformedDummyCtrlPoints(int, int) const;
    const CtrlPoint &GetDeformedCtrlPointOrDummy(int) const;

    DataNode OnTestPulse(DataArray *);
    DataNode OnSetGlobalDefaultSpline(DataArray *);
    DataNode OnClearGlobalDefaultSpline(DataArray *);

    static RndSpline *sGlobalDefaultSpline;

    std::vector<CtrlPoint> mCtrlPoints; // 0x8
    bool mManual; // 0x14
    float mPulseLength; // 0x18
    float mPulseAmplitude; // 0x1c
    /** "Control point to start drawing from (set -1 to start from the beginning)" */
    int mStartCtrlPoint; // 0x20
    /** "Control point to stop drawing at (set -1 to stop at the end)" */
    int mEndCtrlPoint; // 0x24
    float mYOffset; // 0x28
    float mYPerCtrlPoint; // 0x2c
    std::vector<CtrlPoint> mDeformedCtrlPoints; // 0x30
    CtrlPoint unk3c; // 0x3c
    CtrlPoint unk94; // 0x94
    CtrlPoint unkec; // 0xec
    bool unk144;
    bool unk145;
    bool mPulse;
    float unk148;
    bool unk14c;
};
