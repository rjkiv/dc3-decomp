#pragma once
#include "char/CharClip.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamCharacter.h"
#include "hamobj/HamDriver.h"
#include "math/Key.h"
#include "math/Utl.h"
#include "obj/Dir.h"
#include "rndobj/PropAnim.h"
#include "utl/Symbol.h"

class ClipPlayer {
public:
    enum RestStepType {
        kRestStepLeft = 0,
        kRestStepRight = 1,
        kRestStepForward = 2,
        kRestStepBack = 3,
        kNumRestSteps = 4,
    };

    ClipPlayer(int i)
        : mClipKeys(nullptr), mCrossoverKeys(nullptr), mMasterClipKeys(nullptr),
          mCharIndex(i), mB1(-kHugeFloat), mB2(kHugeFloat), mIntro(nullptr),
          mOutro(nullptr), mBlendDebug(0), mOffset(0) {}

    void PlayAnims(HamCharacter *c, float frame, float lastFrame, int blendDebug);
    bool Init(int player);
    bool Init(Difficulty d);
    bool CanUseRestStep();

    DataNode AnnotatePractice();
    DataNode AnnotateClip(float frame);

    static const char *sRestStepNames[kNumRestSteps];

protected:
    bool Init(RndPropAnim *songAnim);
    void PlayNormal(float blendStart, HamDriver::LayerArray *layer, const char *name);
    float ClipLength(CharClip *c);
    void PlayClip(
        CharClip *clip, float startBeat, float startBlend, HamDriver::LayerArray *layer
    );
    bool PushExpertClip(int index, HamDriver::LayerArray *layer);
    CharClip *GetTransitionBefore(Key<Symbol> *key);
    CharClip *GetRoutineTransition(const char *, Key<Symbol> *);
    CharClip *GetPrevRoutineTransition(int);
    void GetRoutineCrossoverClips(float, const char *, CharClip **, CharClip **);
    bool PushRoutineBuilderClip(int index, HamDriver::LayerArray *layer);
    void PushClip(int index, HamDriver::LayerArray *layer);
    bool GetClipRange(
        const char *clipName,
        const char *nextClipName,
        float beat,
        float &startBeat,
        float &endBeat,
        float &blendBeat
    );

    Keys<Symbol, Symbol> *mClipKeys; // 0x0
    Keys<Symbol, Symbol> *mCrossoverKeys; // 0x4
    Keys<Symbol, Symbol> *mMasterClipKeys; // 0x8
    float mBeat; // 0xc
    float mLastBeat; // 0x10
    int mCharIndex; // 0x14
    ObjectDir *mClips; // 0x18
    HamDriver *mDriver; // 0x1c
    float mB1; // 0x20
    float mB2; // 0x24
    CharClip *mIntro; // 0x28
    CharClip *mOutro; // 0x2c
    CharClip *mRest; // 0x30
    CharClip *mRestSteps[kNumRestSteps]; // 0x34
    int mBlendCount; // 0x44
    int mBlendDebug; // 0x48
    int mLine; // 0x4c
    float mOffset; // 0x50
};
