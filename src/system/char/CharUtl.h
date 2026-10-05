#pragma once
#include "char/CharBone.h"
#include "char/CharClip.h"
#include "char/Character.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Dir.h"

void CharUtlResetHair(Character *);
void CharUtlInit();
void CharUtlMergeBones(ObjectDir *, ObjectDir *, int);
RndTransformable *CharUtlFindBoneTrans(const char *, ObjectDir *);
bool CharUtlIsAnimatable(RndTransformable *);
void CharUtlResetTransform(ObjectDir *);
CharBone *CharUtlFindBone(const char *, ObjectDir *);
CharBone *GrabBone(CharBone *, ObjectDir *);
DataNode OnResetHair(DataArray *);
DataNode OnCharMergeBones(DataArray *);

class CharUtlBoneSaver {
public:
    CharUtlBoneSaver(ObjectDir *);
    ~CharUtlBoneSaver();

    ObjectDir *mDir; // 0x0
    std::vector<Transform> mXfms; // 0x4
};

class ClipPredict {
public:
    ClipPredict() : mClip(nullptr) {}
    ClipPredict(CharClip *, const Vector3 &, float);
    void SetClip(CharClip *);
    void PredictDeltaPos(float, float);
    void Predict(float, float);
    float Angle() const { return mAng; }
    Vector3 &Pos() { return mPos; }
    Vector3 &LastPos() { return mLastPos; }
    void SetPos(const Vector3 &v) { mPos = v; }

protected:
    CharClip *mClip; // 0x0
    void *mAngChannel; // 0x4
    void *mPosChannel; // 0x8
    Vector3 mPos; // 0xc
    float mAng; // 0x18
    Vector3 mLastPos; // 0x1c
    float mLastAng; // 0x28
};
