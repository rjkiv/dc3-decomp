#include "char/CharIKFoot.h"
#include "CharIKHand.h"
#include "char/Character.h"
#include "obj/Object.h"
#include "rndobj/Trans.h"

CharIKFoot::CharIKFoot() : unkb0(this), unkc4(0), mData(this), mDataIndex(0) {
    unkb0 = Hmx::Object::New<RndTransformable>();
    unkb0->DirtyLocalXfm().Reset();
}

CharIKFoot::~CharIKFoot() { delete unkb0; }

BEGIN_HANDLERS(CharIKFoot)
    HANDLE_SUPERCLASS(CharIKHand)
END_HANDLERS

BEGIN_PROPSYNCS(CharIKFoot)
    SYNC_PROP(data, mData)
    SYNC_PROP(data_index, mDataIndex)
    SYNC_SUPERCLASS(CharIKHand)
END_PROPSYNCS

BEGIN_SAVES(CharIKFoot)
    SAVE_REVS(6, 0)
    SAVE_SUPERCLASS(CharIKHand)
    bs << mData;
    bs << mDataIndex;
END_SAVES

BEGIN_COPYS(CharIKFoot)
    COPY_SUPERCLASS(CharIKHand)
    CREATE_COPY(CharIKFoot)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mData)
        COPY_MEMBER(mDataIndex)
    END_COPYING_MEMBERS
END_COPYS

void CharIKFoot::Enter() {
    unkc4 = 0;
    unkf0 = 0.0f;
}

void CharIKFoot::PollDeps(std::list<Hmx::Object *> &l1, std::list<Hmx::Object *> &l2) {
    CharIKHand::PollDeps(l1, l2);
}

void CharIKFoot::Poll() {
    if (mFinger && mHand && mData) {
        mTargets.clear();
        mTargets.push_back(IKTarget(unkb0, 0));
        DoFSM(Character::Current(), unkb0->DirtyLocalXfm());
        CharIKHand::Poll();
        mTargets.clear();
    }
}
