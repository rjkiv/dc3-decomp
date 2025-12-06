#include "char/CharUtl.h"
#include "char/CharClip.h"
#include "char/CharCuff.h"
#include "char/CharHair.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Cam.h"

#pragma region CharUtl

void CharUtlResetHair(Character *c) {
    for (ObjDirItr<CharHair> it(c, true); it != 0; ++it) {
        it->Enter();
    }
}

void CharUtlInit() {
    DataRegisterFunc("reset_hair", OnResetHair);
    DataRegisterFunc("char_merge_bones", OnCharMergeBones);
}

void CharUtlMergeBones(ObjectDir *dir1, ObjectDir *dir2, int i) {
    for (ObjDirItr<CharBone> it(dir1, true); it != 0; ++it) {
        if (it->Target()) {
            CharBone *bone = GrabBone(it, dir2);
            if (bone) {
                if (!bone->Target()) {
                    const char *name = it->Target()->Name();
                    CharBone *findbone = CharUtlFindBone(name, dir2);
                    if (!findbone)
                        MILO_NOTIFY("could not find target %s in dest, must merge", name);
                    bone->SetTarget(findbone);
                } else {
                    if (!streq(it->Target()->Name(), bone->Target()->Name())) {
                        MILO_NOTIFY(
                            "%s has different targets %s v %s, must resolve",
                            it->Name(),
                            it->Target()->Name(),
                            bone->Target()->Name()
                        );
                    }
                }
            }
        }
        if (it->PositionContext() != 0) {
            CharBone *bone = GrabBone(it, dir2);
            if (bone)
                bone->SetPositionContext(bone->PositionContext() | i);
        }
        if (it->ScaleContext() != 0) {
            CharBone *bone = GrabBone(it, dir2);
            if (bone)
                bone->SetPositionContext(bone->ScaleContext() | i);
        }
        if (it->RotationContext() != 0 && it->RotationType() != CharBones::TYPE_END) {
            CharBone *bone = GrabBone(it, dir2);
            if (bone) {
                if (bone->RotationType() != CharBones::TYPE_END
                    && bone->RotationType() != it->RotationType()) {
                    MILO_NOTIFY(
                        "bones %s have different rotations, must hand resolve", it->Name()
                    );
                } else {
                    bone->SetRotationType(it->RotationType());
                    bone->SetRotationContext(bone->RotationContext() | i);
                }
            }
        }
    }
}

RndTransformable *CharUtlFindBoneTrans(const char *cc, ObjectDir *dir) {
    if (!dir)
        return 0;
    else {
        char buf[256];
        strcpy(buf, cc);
        char *dst = strrchr(buf, 0x2E);
        if (!dst)
            dst = buf + strlen(buf);
        strcpy(dst, ".cb");
        CharBone *bone = dir->Find<CharBone>(buf, false);
        if (bone)
            return bone->BoneTrans();
        else {
            strcpy(dst, ".trans");
            RndTransformable *trans = dir->Find<RndTransformable>(buf, false);
            if (trans)
                return trans;
            else {
                strcpy(dst, ".mesh");
                RndTransformable *mesh = dir->Find<RndTransformable>(buf, false);
                return mesh;
            }
        }
    }
}

bool CharUtlIsAnimatable(RndTransformable *trans) {
    RndMesh *mesh = dynamic_cast<RndMesh *>(trans);
    if (mesh && mesh->NumBones() != 0)
        return false;
    if (dynamic_cast<RndCam *>(trans))
        return false;
    if (dynamic_cast<CharCollide *>(trans))
        return false;
    if (dynamic_cast<CharCuff *>(trans))
        return false;
    if (dynamic_cast<RndDir *>(trans))
        return false;
    return strncmp(trans->Name(), "spot_", 5) != 0;
}

void CharUtlResetTransform(ObjectDir *dir) {}

CharBone *CharUtlFindBone(const char *cc, ObjectDir *dir) {
    if (!dir)
        return 0;
    else {
        char buf[256];
        strcpy(buf, cc);
        char *dst = strrchr(buf, 0x2E);
        if (!dst)
            dst = buf + strlen(buf);
        strcpy(dst, ".cb");
        return dir->Find<CharBone>(buf, false);
    }
}

CharBone *GrabBone(CharBone *bone, ObjectDir *dir) {
    CharBone *found = CharUtlFindBone(bone->Name(), dir);
    if (!found)
        MILO_NOTIFY("Could not find %s must hand merge", bone->Name());
    return found;
}

DataNode OnResetHair(DataArray *da) {
    CharUtlResetHair(da->Obj<Character>(1));
    return DataNode(0);
}

DataNode OnCharMergeBones(DataArray *da) {
    FilePath fp(da->Str(1));
    ObjectDir *dir = DirLoader::LoadObjects(fp, 0, 0);
    ObjectDir *dir2 = da->Obj<ObjectDir>(2);
    CharUtlMergeBones(dir, dir2, da->Int(3));
    delete dir;
    return DataNode(0);
}

#pragma endregion CharUtl
#pragma region CharUtlBoneSaver

CharUtlBoneSaver::CharUtlBoneSaver(ObjectDir *dir) : mDir(dir) {
    for (ObjDirItr<RndTransformable> it(mDir, true); it != 0; ++it) {
        if (strncmp("bone_", it->Name(), 5) == 0) {
            mXfms.push_back(it->LocalXfm());
        }
    }
}

CharUtlBoneSaver::~CharUtlBoneSaver() {
    int idx = 0;
    for (ObjDirItr<RndTransformable> it(mDir, true); it != 0; ++it) {
        if (strncmp("bone_", it->Name(), 5) == 0) {
            it->SetLocalXfm(mXfms[idx++]);
        }
    }
}

#pragma endregion CharUtlBoneSaver
#pragma region ClipPredict

ClipPredict::ClipPredict(CharClip *clip, const Vector3 &pos, float ang) : mClip(0) {
    SetClip(clip);
    mPos = pos;
    mAng = ang;
    MILO_ASSERT(mAngChannel, 0x22c);
}

void ClipPredict::SetClip(CharClip *clip) {
    if (clip != mClip) {
        mClip = clip;
        mAngChannel = clip->GetChannel("bone_facing.rotz");
        mPosChannel = clip->GetChannel("bone_facing.pos");
        MILO_ASSERT(mAngChannel, 0x238);
    }
}

void ClipPredict::PredictDeltaPos(float f1, float f2) {
    mClip->EvaluateChannel(&mPos, mPosChannel, f1);
    mClip->EvaluateChannel(&mLastPos, mPosChannel, f2);
    Subtract(mLastPos, mPos, mPos);
}

void ClipPredict::Predict(float f1, float f2) {
    Vector3 v34;
    float locf;
    mClip->EvaluateChannel(&v34, mPosChannel, f1);
    mClip->EvaluateChannel(&mLastPos, mPosChannel, f2);
    mClip->EvaluateChannel(&locf, mAngChannel, f1);
    mClip->EvaluateChannel(&mLastAng, mAngChannel, f2);
    float norm = LimitAng(mAng - locf);
    Subtract(mLastPos, v34, v34);
    RotateAboutZ(v34, norm, v34);
    mPos += v34;
    float norm1 = LimitAng(mLastAng - locf);
    mAng = LimitAng(norm1 + mAng);
}

#pragma endregion ClipPredict
