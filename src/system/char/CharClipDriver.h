#pragma once
#include "char/CharBones.h"
#include "char/CharClip.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "utl/MemMgr.h"
#include "utl/PoolAlloc.h"
#include "utl/Symbol.h"

class CharClipDriver {
public:
    CharClipDriver(
        Hmx::Object *, CharClip *, int, float, CharClipDriver *, float, float, bool
    );
    CharClipDriver(Hmx::Object *, CharClipDriver const &);
    void ScaleAdd(CharBones &, float);
    void RotateTo(CharBones &, float);
    int NumBeatEvents() const;
    void DeleteStack();
    float AlignToBeat(float);
    void SetBeatOffset(float, TaskUnits, Symbol);
    float Evaluate(float, float, float);
    CharClipDriver *Exit(bool);
    CharClipDriver *DeleteRef(ObjRef *, bool &);
    CharClipDriver *PreEvaluate(float, float, float);

    CharClipDriver *Next() const { return mNext; }
    CharClip *GetClip() const { return mClip; }

    POOL_OVERLOAD(CharClipDriver, 0x17);

    int mPlayFlags; // 0x0
    float mBlendWidth; // 0x4
    float mTimeScale; // 0x8
    float mRampIn; // 0xc
    float mBeat; // 0x10
    float mDBeat; // 0x14
    float mBlendFrac; // 0x18
    float mAdvanceBeat; // 0x1c
    float mWeight; // 0x20
    ObjOwnerPtr<CharClip> mClip; // 0x24
    CharClipDriver *mNext; // 0x38
    int mNextEvent; // 0x3c
    DataArray *mEventData; // 0x40
    bool mPlayMultipleClips; // 0x44

protected:
    void PlayEvents(float);
    void ExecuteEvent(Symbol);
};
