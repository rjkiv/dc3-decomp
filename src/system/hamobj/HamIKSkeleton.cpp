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

void HamIKSkeleton::NeutralLocalPos(RndTransformable *t, Vector3 &v) {
    if (mNeutralSkelDir) {
        if (!streq(t->Name(), "bone_pelvis.mesh")) {
            t = mNeutralSkelDir->Find<RndTransformable>(t->Name(), true);
        }
    }
    v = t->LocalXfm().v;
}

void HamIKSkeleton::NeutralWorldXfm(RndTransformable *t, Transform &xfm) {
    if (mNeutralSkelDir && mNeutralSkelDir != mChar) {
        RndTransformable *charTrans = mChar->Find<RndTransformable>(t->Name(), false);
        if (charTrans) {
            SetBone(t, charTrans);
            t = charTrans;
        }
    }
    xfm = t->WorldXfm();
}

void HamIKSkeleton::SetBone(RndTransformable *t1, RndTransformable *t2) {
    if (t2->Dirty()) {
        if (!t1) {
            MILO_NOTIFY_ONCE("%s bone is NULL, neutral is %s", PathName(this), t2->Name());
        } else {
            SetBone(t1->TransParent(), t2->TransParent());
            t2->SetLocalRot(t1->LocalXfm().m);
            t2->WorldXfm();
        }
    }
}
