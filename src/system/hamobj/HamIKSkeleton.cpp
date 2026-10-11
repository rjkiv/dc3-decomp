#include "hamobj/HamIKSkeleton.h"
#include "hamobj/HamCharacter.h"
#include "math/Mtx.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Trans.h"

HamIKSkeleton::HamIKSkeleton() : mNeutralSkelDir(nullptr), mChar(this) {}

BEGIN_HANDLERS(HamIKSkeleton)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(HamIKSkeleton)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(HamIKSkeleton)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(Hmx::Object)
END_SAVES

BEGIN_COPYS(HamIKSkeleton)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(HamIKSkeleton)
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(HamIKSkeleton)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
END_LOADS

void HamIKSkeleton::Poll() {
    if (mChar) {
        mNeutralSkelDir = mChar->GetNeutralSkeleton();
        RndTransformable *charTrans =
            mChar->Find<RndTransformable>("bone_pelvis.mesh", true);
        RndTransformable *neutralSkelTrans =
            mNeutralSkelDir->Find<RndTransformable>("bone_pelvis.mesh", true);
        neutralSkelTrans->SetWorldXfm(charTrans->WorldXfm());
    }
}

void HamIKSkeleton::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    changedBy.push_back(mChar->Find<RndTransformable>("bone_pelvis.mesh", false));
}

void HamIKSkeleton::SetName(const char *name, ObjectDir *dir) {
    Hmx::Object::SetName(name, dir);
    mChar = dynamic_cast<HamCharacter *>(dir);
}

void HamIKSkeleton::NeutralLocalPos(RndTransformable *bone, Vector3 &pos) {
    if (mNeutralSkelDir) {
        if (!streq(bone->Name(), "bone_pelvis.mesh")) {
            bone = mNeutralSkelDir->Find<RndTransformable>(bone->Name(), true);
        }
    }
    pos = bone->LocalXfm().v;
}

void HamIKSkeleton::NeutralWorldXfm(RndTransformable *bone, Transform &world) {
    if (mNeutralSkelDir && mNeutralSkelDir != mChar) {
        RndTransformable *charTrans =
            mNeutralSkelDir->Find<RndTransformable>(bone->Name(), false);
        if (charTrans) {
            SetBone(bone, charTrans);
            bone = charTrans;
        }
    }
    world = bone->WorldXfm();
}

void HamIKSkeleton::SetBone(RndTransformable *bone, RndTransformable *neutral) {
    if (neutral->Dirty()) {
        if (!bone) {
            MILO_NOTIFY_ONCE(
                "%s bone is NULL, neutral is %s", PathName(this), neutral->Name()
            );
        } else {
            SetBone(bone->TransParent(), neutral->TransParent());
            neutral->SetLocalRot(bone->LocalXfm().m);
            neutral->WorldXfm();
        }
    }
}
