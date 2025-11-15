#include "char/Char.h"
#include "CharBonesMeshes.h"
#include "CharClipSet.h"
#include "CharMeshHide.h"
#include "CharPollGroup.h"
#include "CharTaskMgr.h"
#include "CharUtl.h"
#include "char/CharBlendBone.h"
#include "char/CharBone.h"
#include "char/CharBoneDir.h"
#include "char/CharBoneOffset.h"
#include "char/CharBoneTwist.h"
#include "char/CharBones.h"
#include "char/CharBonesBlender.h"
#include "char/CharClip.h"
#include "char/CharClipGroup.h"
#include "char/CharCollide.h"
#include "char/CharCuff.h"
#include "char/CharDriver.h"
#include "char/CharDriverMidi.h"
#include "char/CharEyeDartRuleset.h"
#include "char/CharEyes.h"
#include "char/CharFaceServo.h"
#include "char/CharForeTwist.h"
#include "char/CharGuitarString.h"
#include "char/CharHair.h"
#include "char/CharIKFingers.h"
#include "char/CharIKFoot.h"
#include "char/CharIKHand.h"
#include "char/CharIKMidi.h"
#include "char/CharIKRod.h"
#include "char/CharIKScale.h"
#include "char/CharIKSliderMidi.h"
#include "char/CharInterest.h"
#include "char/CharLipSync.h"
#include "char/CharLipSyncDriver.h"
#include "char/CharLookAt.h"
#include "char/CharMirror.h"
#include "char/CharNeckTwist.h"
#include "char/CharPosConstraint.h"
#include "char/CharServoBone.h"
#include "char/CharSleeve.h"
#include "char/CharTransDraw.h"
#include "char/CharUpperTwist.h"
#include "char/CharWeightSetter.h"
#include "char/CharWeightable.h"
#include "char/Character.h"
#include "char/ClipCollide.h"
#include "char/FileMerger.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "rndobj/Highlight.h"
#include "rndobj/Overlay.h"
#include "world/Dir.h"

#pragma region CharDebug

CharDebug::CharDebug() : mObjects(nullptr), mOnce(nullptr) {}

CharDebug::~CharDebug() {}

void CharDebug::Once(Hmx::Object *obj) {
    mOverlay->SetShowing(!mObjects.empty() || !mOnce.empty());
}

void CharDebug::Init() {
    DataRegisterFunc("char_debug", OnSetObjects);
    mOverlay = RndOverlay::Find("char_debug", true);
    mOverlay->SetCallback(this);
}

float CharDebug::UpdateOverlay(RndOverlay *ovl, float hilite_y) { return 1.0f; }

void CharDebug::AddObject(Hmx::Object *o, bool b) {}

void CharDebug::SetObjects(DataArray *msg) {}

DataNode CharDebug::OnSetObjects(DataArray *) { return NULL_OBJ; }

void CharDebug::DisplayObject(Hmx::Object *) {}

#pragma endregion CharDebug

void CharInit() {
    Character::Init();
    CharBonesObject::Init();
    REGISTER_OBJ_FACTORY(CharBoneOffset);
    REGISTER_OBJ_FACTORY(CharBlendBone);
    REGISTER_OBJ_FACTORY(CharBone);
    REGISTER_OBJ_FACTORY(CharBonesBlender);
    CharBonesMeshes::Init();
    REGISTER_OBJ_FACTORY(CharBoneTwist);
    CharClip::Init();
    REGISTER_OBJ_FACTORY(CharClipSet);
    REGISTER_OBJ_FACTORY(CharClipGroup);
    REGISTER_OBJ_FACTORY(CharCollide);
    REGISTER_OBJ_FACTORY(CharCuff);
    REGISTER_OBJ_FACTORY(CharDriver);
    REGISTER_OBJ_FACTORY(CharDriverMidi);
    REGISTER_OBJ_FACTORY(CharEyes);
    REGISTER_OBJ_FACTORY(CharInterest);
    REGISTER_OBJ_FACTORY(CharEyeDartRuleset);
    REGISTER_OBJ_FACTORY(CharFaceServo);
    REGISTER_OBJ_FACTORY(CharForeTwist);
    REGISTER_OBJ_FACTORY(CharHair);
    REGISTER_OBJ_FACTORY(CharIKFingers);
    REGISTER_OBJ_FACTORY(CharIKFoot);
    REGISTER_OBJ_FACTORY(CharIKHand);
    REGISTER_OBJ_FACTORY(CharIKMidi);
    REGISTER_OBJ_FACTORY(CharIKSliderMidi);
    REGISTER_OBJ_FACTORY(CharIKRod);
    REGISTER_OBJ_FACTORY(CharIKScale);
    REGISTER_OBJ_FACTORY(CharLipSync);
    CharLipSync::Init();
    REGISTER_OBJ_FACTORY(CharLipSyncDriver);
    REGISTER_OBJ_FACTORY(CharLookAt);
    CharMeshHide::Init();
    REGISTER_OBJ_FACTORY(CharMirror);
    REGISTER_OBJ_FACTORY(CharNeckTwist);
    REGISTER_OBJ_FACTORY(CharPollGroup);
    REGISTER_OBJ_FACTORY(CharPosConstraint);
    REGISTER_OBJ_FACTORY(CharServoBone);
    // REGISTER_OBJ_FACTORY(CharSignalApplier);
    REGISTER_OBJ_FACTORY(CharSleeve);
    CharTaskMgr::Init();
    REGISTER_OBJ_FACTORY(CharTransDraw);
    REGISTER_OBJ_FACTORY(CharUpperTwist);
    REGISTER_OBJ_FACTORY(CharWeightable);
    REGISTER_OBJ_FACTORY(CharWeightSetter);
    REGISTER_OBJ_FACTORY(CharGuitarString);
    REGISTER_OBJ_FACTORY(FileMerger);
    REGISTER_OBJ_FACTORY(CharBoneDir);
    REGISTER_OBJ_FACTORY(ClipCollide);
    PreloadSharedSubdirs("char");
    CharBoneDir::Init();
    CharUtlInit();
}
