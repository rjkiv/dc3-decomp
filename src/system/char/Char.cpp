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
#include "math/Vec.h"
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
    CharBonesObject::Register();
    CharBoneOffset::Register();
    CharBlendBone::Register();
    CharBone::Register();
    CharBonesBlender::Register();
    CharBonesMeshes::Init();
    CharBoneTwist::Register();
    CharClip::Init();
    CharClipSet::Register();
    CharClipGroup::Register();
    CharCollide::Register();
    CharCuff::Register();
    CharDriver::Register();
    CharDriverMidi::Register();
    CharEyes::Register();
    CharInterest::Register();
    CharEyeDartRuleset::Register();
    CharFaceServo::Register();
    CharForeTwist::Register();
    CharHair::Register();
    CharIKFingers::Register();
    CharIKFoot::Register();
    CharIKHand::Register();
    CharIKHead::Register();
    CharIKMidi::Register();
    CharIKSliderMidi::Register();
    CharIKRod::Register();
    CharIKScale::Register();
    CharLipSync::Register();
    CharLipSync::Init();
    CharLipSyncDriver::Register();
    CharLookAt::Register();
    CharMeshHide::Init();
    CharMirror::Register();
    CharNeckTwist::Register();
    CharPollGroup::Register();
    CharPosConstraint::Register();
    CharServoBone::Register();
    CharSignalApplier::Register();
    CharSleeve::Register();
    CharTaskMgr::Init();
    CharTransDraw::Register();
    CharUpperTwist::Register();
    CharWeightable::Register();
    CharWeightSetter::Register();
    Waypoint::Init();
    CharGuitarString::Register();
    FileMerger::Register();
    CharBoneDir::Register();
    ClipCollide::Register();
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
                    Vector2 uv(i < 2 ? 1.0f : 0.0f, i == 1 || i == 2 ? 0.0f : 1.0f);

                    auto &vert = mesh->Verts()[i];
                    vert.pos.y = 0;
                    vert.pos.z = -(uv.y * 20 - 60);
                    vert.pos.x = (uv.x + 1) * 20;
                    vert.tex = uv;
                    vert.norm.Set(0, -1, 0);
                    vert.boneWeights.Set(0, 0, 0, 0);
                    vert.color.Set(1, 1, 1, 1);
                }
                mesh->Faces()[0].Set(0, 1, 2);
                mesh->Faces()[1].Set(0, 2, 3);
                mesh->Sync(RndMesh::kFastPatch | RndMesh::kAll);
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
