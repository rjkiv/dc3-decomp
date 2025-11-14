#include "char/CharSleeve.h"
#include "obj/Object.h"
#include "rndobj/Rnd.h"
#include "rndobj/Utl.h"

CharSleeve::CharSleeve()
    : mSleeve(this), mTopSleeve(this), unk38(0), unk3c(0), unk40(0), unk48(0), unk4c(0),
      unk50(0), unk58(0), mInertia(0.5f), mGravity(1.0f), mRange(0), mNegLength(0),
      mPosLength(0), mStiffness(0.02f) {}

CharSleeve::~CharSleeve() {}

BEGIN_PROPSYNCS(CharSleeve)
    SYNC_PROP(sleeve, mSleeve)
    SYNC_PROP(top_sleeve, mTopSleeve)
    SYNC_PROP(inertia, mInertia)
    SYNC_PROP(gravity, mGravity)
    SYNC_PROP(stiffness, mStiffness)
    SYNC_PROP(range, mRange)
    SYNC_PROP(neg_length, mNegLength)
    SYNC_PROP(pos_length, mPosLength)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharSleeve)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mSleeve;
    bs << mTopSleeve;
    bs << mInertia;
    bs << mGravity;
    bs << mStiffness;
    bs << mRange;
    bs << mNegLength;
    bs << mPosLength;
END_SAVES

BEGIN_COPYS(CharSleeve)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharSleeve)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mSleeve)
        COPY_MEMBER(mTopSleeve)
        COPY_MEMBER(mInertia)
        COPY_MEMBER(mGravity)
        COPY_MEMBER(mStiffness)
        COPY_MEMBER(mRange)
        COPY_MEMBER(mNegLength)
        COPY_MEMBER(mPosLength)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(CharSleeve)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    bs >> mSleeve >> mTopSleeve;
    bs >> mInertia >> mGravity >> mStiffness >> mRange >> mNegLength >> mPosLength;
END_LOADS

void CharSleeve::Poll() {}

void CharSleeve::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    if (mSleeve) {
        changedBy.push_back(mSleeve->TransParent());
        change.push_back(mSleeve);
        change.push_back(mTopSleeve);
    }
}

void CharSleeve::Highlight() {
    if (!mSleeve || !mSleeve->TransParent())
        return;
    UtilDrawAxes(mSleeve->WorldXfm(), 1.0f, Hmx::Color(0.0f, 1.0f, 0.0f));
    TheRnd.DrawLine(
        mSleeve->WorldXfm().v,
        mSleeve->TransParent()->WorldXfm().v,
        Hmx::Color(0.0f, 1.0f, 0.0f),
        false
    );
    if (mTopSleeve) {
        UtilDrawAxes(mTopSleeve->WorldXfm(), 1.0f, Hmx::Color(0.0f, 1.0f, 1.0f));
        TheRnd.DrawLine(
            mTopSleeve->WorldXfm().v,
            mTopSleeve->TransParent()->WorldXfm().v,
            Hmx::Color(0.0f, 1.0f, 1.0f),
            false
        );
    }
}

BEGIN_HANDLERS(CharSleeve)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS
