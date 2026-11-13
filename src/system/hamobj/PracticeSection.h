#pragma once
#include "hamobj/DancerSequence.h"
#include "hamobj/Difficulty.h"
#include "rndobj/Anim.h"
#include "utl/MemMgr.h"

class PracticeStep {
public:
    PracticeStep() {
        mType = "learn";
        mBoundary = false;
    }

    void SetType(Symbol t) {
        mType = t;
        if (mType != "learn") {
            mBoundary = true;
        }
    }

    /** "Type of step". Options are (learn review freestyle) */
    Symbol mType; // 0x0
    /** "Start of sequence" */
    Symbol mStart; // 0x4
    /** "End of sequence" */
    Symbol mEnd; // 0x8
    /** "True if this step is the START of a subsection" */
    bool mBoundary; // 0xc
    /** "Name to display on the PracticeChoosePanel,
        if left blank it tries to automatically pick one" */
    String mNameOverride; // 0x10
};

/** "Practice section details for skills mode" */
class PracticeSection : public RndAnimatable {
public:
    // Hmx::Object
    virtual ~PracticeSection();
    OBJ_CLASSNAME(PracticeSection);
    OBJ_SET_TYPE(PracticeSection);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndAnimatable
    virtual void SetFrame(float frame, float blend);
    virtual float StartFrame();
    virtual float EndFrame();

    OBJ_MEM_OVERLOAD(0x2D)
    NEW_OBJ(PracticeSection)
    REGISTER_OBJ(PracticeSection)

    const std::vector<PracticeStep> &Steps() const;
    void ClearSteps();
    void AddStep(PracticeStep);
    DancerSequence *SequenceForDetection(Symbol, Symbol);
    Difficulty GetDifficulty() const { return mDifficulty; }

protected:
    PracticeSection();

    /** "Difficulty this section is tied to" */
    Difficulty mDifficulty; // 0x10
    /** "Display name used to show in selection screens" */
    Symbol mDisplayName; // 0x14
    /** "List of steps for this practice section" */
    std::vector<PracticeStep> mSteps; // 0x18
    std::vector<DancerSequence *> mSeqs; // 0x24
    /** "Index of step/sequence to test" */
    int mTestStepSequence; // 0x30
};
