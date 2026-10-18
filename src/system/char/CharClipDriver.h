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
        Hmx::Object *owner,
        CharClip *clip,
        int playFlags,
        float blendWidth,
        CharClipDriver *next,
        float startBeat,
        float deltaStart,
        bool playMultiple
    );
    CharClipDriver(Hmx::Object *owner, const CharClipDriver &d);
    void ScaleAdd(CharBones &bones, float weight);
    void RotateTo(CharBones &bones, float weight);
    int NumBeatEvents() const;
    void DeleteStack();
    float AlignToBeat(float);
    void SetBeatOffset(float offset, TaskUnits units, Symbol beatEvent);
    float Evaluate(float beat, float, float);
    CharClipDriver *Exit(bool stack);
    CharClipDriver *DeleteRef(ObjRef *, bool &);
    CharClipDriver *PreEvaluate(float beat, float dbeat, float dt);

    CharClipDriver *Next() const { return mNext; }
    CharClip *Clip() const { return mClip; }

    POOL_OVERLOAD(CharClipDriver, 0x17);

    // RB2 says these specific fields are public
    int mPlayFlags; // 0x0
    float mBlendWidth; // 0x4
    float mTimeScale; // 0x8
    float mRampIn; // 0xc
    float mBeat; // 0x10
    float mDBeat; // 0x14
    float mBlendFrac; // 0x18
    float mAdvanceBeat; // 0x1c
    float mWeight; // 0x20

protected:
    void PlayEvents(float oldBeat);
    void ExecuteEvent(Symbol handler);

    ObjOwnerPtr<CharClip> mClip; // 0x24
    CharClipDriver *mNext; // 0x38
    int mNextEvent; // 0x3c
    DataArray *mEventData; // 0x40
    bool mPlayMultipleClips; // 0x44
};
