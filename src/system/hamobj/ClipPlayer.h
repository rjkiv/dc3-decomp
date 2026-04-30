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
    ClipPlayer()
        : mClipKeys(nullptr), mClipCrossoverKeys(nullptr), mMasterClipKeys(nullptr),
          unk14(0), unk20(-kHugeFloat), unk24(kHugeFloat), mInClip(nullptr),
          mOutClip(nullptr), unk48(0), unk50(0) {}

    void PlayAnims(HamCharacter *, float, float, int);
    bool Init(int);
    bool Init(Difficulty);
    bool CanUseRestStep();

    DataNode AnnotatePractice();
    DataNode AnnotateClip(float);

    static const char *sRestStepNames[4];

protected:
    bool Init(RndPropAnim *);
    void PlayNormal(float, HamDriver::LayerArray *, const char *);
    float ClipLength(CharClip *);
    void PlayClip(CharClip *, float, float, HamDriver::LayerArray *);
    bool PushExpertClip(int, HamDriver::LayerArray *);
    CharClip *GetTransitionBefore(Key<Symbol> *);
    CharClip *GetRoutineTransition(const char *, Key<Symbol> *);
    void GetRoutineCrossoverClips(float, const char *, CharClip **, CharClip **);
    bool PushRoutineBuilderClip(int, HamDriver::LayerArray *);
    void PushClip(int, HamDriver::LayerArray *);
    bool GetClipRange(const char *, const char *, float, float &, float &, float &);

    Keys<Symbol, Symbol> *mClipKeys; // 0x0
    Keys<Symbol, Symbol> *mClipCrossoverKeys; // 0x4
    Keys<Symbol, Symbol> *mMasterClipKeys; // 0x8
    float unkc;
    float unk10;
    int unk14;
    ObjectDir *mClipDir; // 0x18
    HamDriver *unk1c;
    float unk20;
    float unk24;
    CharClip *mInClip; // 0x28
    CharClip *mOutClip; // 0x2c
    CharClip *mRestClip; // 0x30
    CharClip *mRestStepClips[4]; // 0x34
    int unk44;
    int unk48;
    int unk4c;
    float unk50;
};
