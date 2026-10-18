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
        friend class RndSpline;

    public:
        CtrlPoint();
        void Save(BinStream &) const;
        void Load(BinStreamRev &);
        void Interp(const CtrlPoint &iA, const CtrlPoint &iB, float iBlend);

        Vector3 mPos; // 0x0
        float mRoll; // 0x10
    private:
        bool mNew; // 0x14
        bool mDirtyConstants; // 0x15
        Vector4 mA; // 0x18
        Vector4 mB; // 0x28
        Vector4 mC; // 0x38
        Vector4 mD; // 0x48
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
    REGISTER_OBJ(RndSpline)
    static RndSpline *GlobalDefaultSpline() { return sGlobalDefaultSpline; }

    void SetStartCtrlPoint(int iStartCtrlPt);
    void SetEndCtrlPoint(int iEndCtrlPt);
    void PrepareShader(float iYOffset, float iYPerCtrlPt) const;
    void PrepareShader() const {
        PrepareShader(Max(mStartCtrlPoint, 0) * mYPerCtrlPoint - mYOffset, mYPerCtrlPoint);
    }
    int NumCtrlPts() const { return mPristineCtrlPoints.size(); }
    bool Manual() const { return mManual; }
    bool HasPulse() const { return mPulse; }

    const CtrlPoint &GetDeformedCtrlPoint(int iIndex) const;

protected:
    RndSpline();

private:
    void SyncPristineCtrlPoints();
    void SyncDeformedCtrlPoints(int iStartIndex, int iEndIndex) const;
    void SyncDeformedDummyCtrlPoints(int iStartIndex, int iEndIndex) const;
    const CtrlPoint &GetDeformedCtrlPointOrDummy(int iIndex) const;

    DataNode OnTestPulse(DataArray *);
    DataNode OnSetGlobalDefaultSpline(DataArray *);
    DataNode OnClearGlobalDefaultSpline(DataArray *);

    static RndSpline *sGlobalDefaultSpline;

    std::vector<CtrlPoint> mPristineCtrlPoints; // 0x8
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
    CtrlPoint mDummyStart; // 0x3c
    CtrlPoint mDummyEnd1; // 0x94
    CtrlPoint mDummyEnd2; // 0xec
    bool mDeformedCtrlPointsDirtyStart; // 0x144
    bool mDeformedCtrlPointsDirtyEnd; // 0x145
    bool mPulse; // 0x146
    float mPulseT; // 0x148
    bool mTestPulse; // 0x14c
};
