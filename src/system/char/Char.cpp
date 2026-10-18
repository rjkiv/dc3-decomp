#include "char/Char.h"
#include "CharBonesMeshes.h"
#include "CharClipSet.h"
#include "CharIKHead.h"
#include "CharMeshHide.h"
#include "CharPollGroup.h"
#include "CharTaskMgr.h"
#include "CharUtl.h"
#include "FileMergerOrganizer.h"
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
#include "char/CharSignalApplier.h"
#include "char/CharSleeve.h"
#include "char/CharTransDraw.h"
#include "char/CharUpperTwist.h"
#include "char/CharWeightSetter.h"
#include "char/CharWeightable.h"
#include "char/Character.h"
#include "char/ClipCollide.h"
#include "char/FileMerger.h"
#include "char/Waypoint.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Cam.h"
#include "rndobj/Highlight.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Overlay.h"
#include "rndobj/Tex.h"
#include "rndobj/Utl.h"
#include "world/Dir.h"
#include <cstring>

CharDebug TheCharDebug;

void CharInit() {
    TheCharDebug.Init();
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
    REGISTER_OBJ_FACTORY(CharIKHead)
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
    REGISTER_OBJ_FACTORY(CharSignalApplier);
    REGISTER_OBJ_FACTORY(CharSleeve);
    CharTaskMgr::Init();
    REGISTER_OBJ_FACTORY(CharTransDraw);
    REGISTER_OBJ_FACTORY(CharUpperTwist);
    REGISTER_OBJ_FACTORY(CharWeightable);
    REGISTER_OBJ_FACTORY(CharWeightSetter);
    Waypoint::Init();
    REGISTER_OBJ_FACTORY(CharGuitarString);
    REGISTER_OBJ_FACTORY(FileMerger);
    REGISTER_OBJ_FACTORY(CharBoneDir);
    REGISTER_OBJ_FACTORY(ClipCollide);
    FileMergerOrganizer::Init();
    PreloadSharedSubdirs("char");
    CharBoneDir::Init();
    CharUtlInit();
    TheDebug.AddExitCallback(CharTerminate);
}

void CharTerminate() {
    TheDebug.RemoveExitCallback(CharTerminate);
    Character::Terminate();
    CharBoneDir::Terminate();
    CharBonesMeshes::Terminate();
    CharLipSync::Terminate();
}

#pragma region CharDebug

CharDebug::CharDebug() : mObjects(nullptr), mOnce(nullptr) {}

CharDebug::~CharDebug() {}

float CharDebug::UpdateOverlay(RndOverlay *ovl, float hilite_y) {
    gCharHighlightY = hilite_y;
    RndCam *cur = RndCam::Current();
    RndCam *worldCam = nullptr;
    if (TheWorld) {
        worldCam = TheWorld->Cam();
        if (worldCam) {
            worldCam->Select();
        }
    }
    FOREACH (it, mObjects) {
        DisplayObject(*it);
    }
    FOREACH (it, mOnce) {
        DisplayObject(*it);
    }
    mOnce.clear();
    if (mObjects.empty()) {
        ovl->SetShowing(false);
    }
    if (worldCam) {
        cur->Select();
    }
    float ret = gCharHighlightY;
    gCharHighlightY = -1;
    return ret;
}

void CharDebug::Init() {
    DataRegisterFunc("char_debug", OnSetObjects);
    mOverlay = RndOverlay::Find("char_debug", true);
    mOverlay->SetCallback(this);
}

void CharDebug::Once(Hmx::Object *obj) {
    AddObject(obj, true);
    mOverlay->SetShowing(!mObjects.empty() || !mOnce.empty());
}

void CharDebug::AddObject(Hmx::Object *o, bool b) {
    if (o) {
        ObjPtrList<Hmx::Object> &which = b ? mOnce : mObjects;
        auto it = which.find(o);
        if (it == which.end()) {
            which.push_back(o);
        } else {
            which.erase(it);
        }
    }
}

void CharDebug::SetObjects(DataArray *msg) {
    int i = 1;
    bool clear = false;
    bool once = false;
    if (msg->Size() > 1 && msg->Type(1) == kDataSymbol) {
        if (msg->Sym(1) == "clear" || msg->Sym(1) == "once") {
            i = 2;
            clear = msg->Sym(1) == "clear";
            once = msg->Sym(1) == "once";
        }
    }
    if (clear) {
        mObjects.clear();
    }
    for (; i < msg->Size(); i++) {
        AddObject(msg->Obj<Hmx::Object>(i), once);
    }
    mOverlay->SetShowing(!mObjects.empty() || !mOnce.empty());
}

void CharDebug::DisplayObject(Hmx::Object *obj) {
    RndHighlightable *rh = dynamic_cast<RndHighlightable *>(obj);
    if (rh) {
        rh->Highlight();
    } else {
        RndTex *tex = dynamic_cast<RndTex *>(obj);
        if (tex) {
            static RndMesh *mesh = nullptr;
            static RndMat *mat = nullptr;
            if (!mesh) {
                mesh = Hmx::Object::New<RndMesh>();
                mat = Hmx::Object::New<RndMat>();
                mat->SetUseEnv(false);
                mesh->Verts().resize(4);
                mesh->Faces().resize(2);
                for (int i = 0; i < 4; i++) {
                    float f4 = i == 1 || i == 2 ? 0.0f : 1.0f;
                    float f5 = i < 2 ? 1.0f : 0.0f;
                    float floats[2] = { f5, f4 };
                    mesh->Verts()[i].pos.Set(
                        (floats[0] + 1) * 20, 0, -(floats[1] * 20 - 60)
                    );
                    memcpy(&mesh->Verts()[i].bone0, floats, sizeof(floats));
                    mesh->Verts()[i].norm.Set(0, -1, 0);
                    mesh->Verts()[i].boneWeights.Set(0, 0, 0, 0);
                    mesh->Verts()[i].color.Set(1, 1, 1, 1);
                }
                mesh->Faces()[0].Set(0, 1, 2);
                mesh->Faces()[1].Set(0, 2, 3);
                mesh->Sync(0x13F);
                mesh->SetMat(mat);
            }
            mat->SetDiffuseTex(tex);
            CreateAndSetMetaMat(mat);
            mesh->DrawShowing();
        }
    }
}

DataNode CharDebug::OnSetObjects(DataArray *a) {
    TheCharDebug.SetObjects(a);
    return 0;
}

#pragma endregion CharDebug

void CharDeferHighlight(Hmx::Object *obj) { TheCharDebug.Once(obj); }
