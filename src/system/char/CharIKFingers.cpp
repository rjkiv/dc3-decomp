#include "char/CharIKFingers.h"
#include "char/CharWeightable.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rndobj/Rnd.h"
#include "rndobj/Utl.h"

CharIKFingers::CharIKFingers()
    : mHand(nullptr), mForeArm(nullptr), mUpperArm(nullptr), mBlendInFrames(0),
      mBlendOutFrames(0), mResetHandDest(1), mResetCurHandTrans(1),
      mFingerCurledLength(0.85), mHandMoveForward(1), mHandPinkyRotation(-0.06),
      mHandThumbRotation(0.23), mHandDestOffset(-0.4), mIsRightHand(1), unk16d(0),
      mIsSetup(0), mOutputTrans(this), mKeyboardRefBone(this) {
    mFingers.resize(5);
    mCurHandTrans.Zero();
    mDestHandTrans.Zero();
    mHandKeyboardOffset = Vector3(0.3f, -6.0f, 0.4f);
    mtx = Hmx::Matrix3();
}

CharIKFingers::~CharIKFingers() {}

BEGIN_HANDLERS(CharIKFingers)
    HANDLE_SUPERCLASS(CharWeightable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharIKFingers)
    SYNC_PROP(is_right_hand, mIsRightHand)
    SYNC_PROP(output_trans, mOutputTrans)
    SYNC_PROP(keyboard_ref_bone, mKeyboardRefBone)
    SYNC_PROP(hand_keyboard_offset, mHandKeyboardOffset)
    SYNC_PROP(hand_thumb_rotation, mHandThumbRotation)
    SYNC_PROP(hand_pinky_rotation, mHandPinkyRotation)
    SYNC_PROP(hand_move_forward, mHandMoveForward)
    SYNC_PROP(hand_dest_offset, mHandDestOffset)
    SYNC_SUPERCLASS(CharWeightable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharIKFingers)
    SAVE_REVS(5, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(CharWeightable)
    bs << mIsRightHand;
    bs << mOutputTrans;
    bs << mKeyboardRefBone;
    bs << mHandKeyboardOffset;
    bs << mHandThumbRotation;
    bs << mHandPinkyRotation;
    bs << mHandMoveForward;
    bs << mHandDestOffset;
END_SAVES

BEGIN_COPYS(CharIKFingers)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(CharWeightable)
    CREATE_COPY(CharIKFingers)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mIsRightHand)
        COPY_MEMBER(mOutputTrans)
        COPY_MEMBER(mKeyboardRefBone)
        COPY_MEMBER(mHandKeyboardOffset)
        COPY_MEMBER(mHandThumbRotation)
        COPY_MEMBER(mHandPinkyRotation)
        COPY_MEMBER(mHandMoveForward)
        COPY_MEMBER(mHandDestOffset)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(CharIKFingers)
    LOAD_REVS(bs)
    ASSERT_REVS(5, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(CharWeightable)
    if (d.rev > 1)
        d >> mIsRightHand;
    if (d.rev > 2)
        bs >> mOutputTrans;
    if (d.rev > 3)
        bs >> mKeyboardRefBone;
    if (d.rev > 4) {
        bs >> mHandKeyboardOffset;
        bs >> mHandThumbRotation;
        bs >> mHandPinkyRotation;
        bs >> mHandMoveForward;
        bs >> mHandDestOffset;
    }
END_LOADS

void CharIKFingers::SetName(const char *name, ObjectDir *dir) {
    Hmx::Object::SetName(name, dir);
    if (dir) {
        for (int i = 0; i < 5; i++) {
            mFingers[i] = FingerDesc();
        }
        if (mIsRightHand) {
            mHand = dir->Find<RndTransformable>("bone_R-hand.mesh", false);
            mForeArm = dir->Find<RndTransformable>("bone_R-foreArm.mesh", false);
            mUpperArm = dir->Find<RndTransformable>("bone_R-upperArm.mesh", false);
            mFingers[kFingerThumb].mFinger01 =
                dir->Find<RndTransformable>("bone_R-thumb01.mesh", false);
            mFingers[kFingerThumb].mFinger02 =
                dir->Find<RndTransformable>("bone_R-thumb02.mesh", false);
            mFingers[kFingerThumb].mFinger03 =
                dir->Find<RndTransformable>("bone_R-thumb03.mesh", false);
            mFingers[kFingerThumb].mFingertip =
                dir->Find<RndTransformable>("spot_R-thumb_tip.mesh", false);
            mFingers[kFingerIndex].mFinger01 =
                dir->Find<RndTransformable>("bone_R-index01.mesh", false);
            mFingers[kFingerIndex].mFinger02 =
                dir->Find<RndTransformable>("bone_R-index02.mesh", false);
            mFingers[kFingerIndex].mFinger03 =
                dir->Find<RndTransformable>("bone_R-index03.mesh", false);
            mFingers[kFingerIndex].mFingertip =
                dir->Find<RndTransformable>("spot_R-index_tip.mesh", false);
            mFingers[kFingerMiddle].mFinger01 =
                dir->Find<RndTransformable>("bone_R-middlefinger01.mesh", false);
            mFingers[kFingerMiddle].mFinger02 =
                dir->Find<RndTransformable>("bone_R-middlefinger02.mesh", false);
            mFingers[kFingerMiddle].mFinger03 =
                dir->Find<RndTransformable>("bone_R-middlefinger03.mesh", false);
            mFingers[kFingerMiddle].mFingertip =
                dir->Find<RndTransformable>("spot_R-middlefinger_tip.mesh", false);
            mFingers[kFingerRing].mFinger01 =
                dir->Find<RndTransformable>("bone_R-ringfinger01.mesh", false);
            mFingers[kFingerRing].mFinger02 =
                dir->Find<RndTransformable>("bone_R-ringfinger02.mesh", false);
            mFingers[kFingerRing].mFinger03 =
                dir->Find<RndTransformable>("bone_R-ringfinger03.mesh", false);
            mFingers[kFingerRing].mFingertip =
                dir->Find<RndTransformable>("spot_R-ringfinger_tip.mesh", false);
            mFingers[kFingerPinky].mFinger01 =
                dir->Find<RndTransformable>("bone_R-pinky01.mesh", false);
            mFingers[kFingerPinky].mFinger02 =
                dir->Find<RndTransformable>("bone_R-pinky02.mesh", false);
            mFingers[kFingerPinky].mFinger03 =
                dir->Find<RndTransformable>("bone_R-pinky03.mesh", false);
            mFingers[kFingerPinky].mFingertip =
                dir->Find<RndTransformable>("spot_R-pinky_tip.mesh", false);
            mtx = Hmx::Matrix3(
                -0.023f,
                0.97899997f,
                0.201f,
                -0.228f,
                0.191f,
                -0.95499998f,
                -0.972,
                -0.068f,
                0.21799999f
            );
            Normalize(mtx, mtx);
            mIsSetup = true;
        } else {
            mHand = dir->Find<RndTransformable>("bone_L-hand.mesh", false);
            mForeArm = dir->Find<RndTransformable>("bone_L-foreArm.mesh", false);
            mUpperArm = dir->Find<RndTransformable>("bone_L-upperArm.mesh", false);
            mFingers[kFingerThumb].mFinger01 =
                dir->Find<RndTransformable>("bone_L-thumb01.mesh", false);
            mFingers[kFingerThumb].mFinger02 =
                dir->Find<RndTransformable>("bone_L-thumb02.mesh", false);
            mFingers[kFingerThumb].mFinger03 =
                dir->Find<RndTransformable>("bone_L-thumb03.mesh", false);
            mFingers[kFingerThumb].mFingertip =
                dir->Find<RndTransformable>("spot_L-thumb_tip.mesh", false);
            mFingers[kFingerIndex].mFinger01 =
                dir->Find<RndTransformable>("bone_L-index01.mesh", false);
            mFingers[kFingerIndex].mFinger02 =
                dir->Find<RndTransformable>("bone_L-index02.mesh", false);
            mFingers[kFingerIndex].mFinger03 =
                dir->Find<RndTransformable>("bone_L-index03.mesh", false);
            mFingers[kFingerIndex].mFingertip =
                dir->Find<RndTransformable>("spot_L-index_tip.mesh", false);
            mFingers[kFingerMiddle].mFinger01 =
                dir->Find<RndTransformable>("bone_L-middlefinger01.mesh", false);
            mFingers[kFingerMiddle].mFinger02 =
                dir->Find<RndTransformable>("bone_L-middlefinger02.mesh", false);
            mFingers[kFingerMiddle].mFinger03 =
                dir->Find<RndTransformable>("bone_L-middlefinger03.mesh", false);
            mFingers[kFingerMiddle].mFingertip =
                dir->Find<RndTransformable>("spot_L-middlefinger_tip.mesh", false);
            mFingers[kFingerRing].mFinger01 =
                dir->Find<RndTransformable>("bone_L-ringfinger01.mesh", false);
            mFingers[kFingerRing].mFinger02 =
                dir->Find<RndTransformable>("bone_L-ringfinger02.mesh", false);
            mFingers[kFingerRing].mFinger03 =
                dir->Find<RndTransformable>("bone_L-ringfinger03.mesh", false);
            mFingers[kFingerRing].mFingertip =
                dir->Find<RndTransformable>("spot_L-ringfinger_tip.mesh", false);
            mFingers[kFingerPinky].mFinger01 =
                dir->Find<RndTransformable>("bone_L-pinky01.mesh", false);
            mFingers[kFingerPinky].mFinger02 =
                dir->Find<RndTransformable>("bone_L-pinky02.mesh", false);
            mFingers[kFingerPinky].mFinger03 =
                dir->Find<RndTransformable>("bone_L-pinky03.mesh", false);
            mFingers[kFingerPinky].mFingertip =
                dir->Find<RndTransformable>("spot_L-pinky_tip.mesh", false);
            mtx = Hmx::Matrix3(
                -0.067f,
                0.985f,
                0.156f,
                0.224f,
                0.167f,
                -0.95999998f,
                -0.972f,
                -0.028999999f,
                -0.23199999f
            );
            Normalize(mtx, mtx);
            mIsSetup = true;
        }
        for (int i = 0; i < 5; i++) {
            FingerDesc cur = mFingers[i];
            if (!cur.mFinger01 || !cur.mFinger02 || !cur.mFinger03 || !cur.mFingertip) {
                mIsSetup = false;
                break;
            }
        }
    }
    MeasureLengths();
}

void CharIKFingers::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    change.push_back(mHand);
    changedBy.push_back(mHand);
    for (int i = 0; i < 5; i++) {
        FingerDesc desc(mFingers[i]);
        if (desc.mFinger01) {
            changedBy.push_back(desc.mFinger01);
        }
        if (desc.mFinger02) {
            changedBy.push_back(desc.mFinger02);
        }
        if (desc.mFinger03) {
            changedBy.push_back(desc.mFinger03);
        }
        if (desc.mFingertip) {
            changedBy.push_back(desc.mFingertip);
        }
    }
    if (mForeArm) {
        change.push_back(mForeArm);
        changedBy.push_back(mForeArm);
    }
    if (mUpperArm) {
        change.push_back(mUpperArm);
        changedBy.push_back(mUpperArm);
    }
}

void CharIKFingers::Highlight() {
    for (int i = 0; i < 5; i++) {
        FingerDesc desc(mFingers[i]);
        if (desc.unk0) {
            UtilDrawSphere(desc.unk8, 0.2f, Hmx::Color(1, 0, 0), 0);
            UtilDrawSphere(desc.unk18, 0.2f, Hmx::Color(0, 1, 0), 0);
            UtilDrawAxes(desc.mFinger01->WorldXfm(), 1.0f, Hmx::Color(1, 1, 1));
            TheRnd.DrawLine(
                desc.mFinger01->WorldXfm().v,
                desc.mFinger02->WorldXfm().v,
                Hmx::Color(1, 1, 1),
                false
            );
            TheRnd.DrawLine(
                desc.mFinger02->WorldXfm().v,
                desc.mFinger03->WorldXfm().v,
                Hmx::Color(1, 1, 1),
                false
            );
            TheRnd.DrawLine(
                desc.mFinger03->WorldXfm().v,
                desc.mFingertip->WorldXfm().v,
                Hmx::Color(1, 1, 1),
                false
            );
        }
    }
}

void CharIKFingers::Poll() {
    if (!mHand || !mIsSetup)
        return;
    else {
        Hmx::Matrix3 mtx58;
        Hmx::Matrix3 mtx7c;
        Invert(mKeyboardRefBone->WorldXfm().m, mtx58);
        Multiply(mHand->WorldXfm().m, mtx58, mtx7c);
        Vector3 v88;
        Subtract(mKeyboardRefBone->WorldXfm().v, mHand->WorldXfm().v, v88);
        float weight = Weight();
        if (weight < 1.0) {
            if (mOutputTrans) {
                mOutputTrans->SetWorldXfm(mHand->WorldXfm());
            }
        } else {
            if (mResetCurHandTrans) {
                mCurHandTrans.Set(mHand->WorldXfm().m, mHand->WorldXfm().v);
                mDestHandTrans.Set(mHand->WorldXfm().m, mHand->WorldXfm().v);
                mResetCurHandTrans = false;
            }
            int i3 = 0;
            float f8 = 1.0f;
            int i1 = -1;
            for (int i = 0; i < 5; i++) {
                if (mFingers[i].unk0) {
                    if (i1 == -1)
                        i1 = i;
                    i3++;
                }
            }
            CalculateHandDest(i3, i1);
            if (mBlendInFrames > 0) {
                f8 = 1.0f - mBlendInFrames / 5.0f;
            } else if (mBlendOutFrames > 0) {
                f8 = 1.0f - mBlendOutFrames / 5.0f;
            }
            Interp(mCurHandTrans.v, mDestHandTrans.v, f8, mCurHandTrans.v);
            Interp(mCurHandTrans.m, mDestHandTrans.m, f8, mCurHandTrans.m);
            if (mOutputTrans) {
                mOutputTrans->SetWorldXfm(mCurHandTrans);
            }
            for (int i = 0; i < 5; i++) {
                CalculateFingerDest((FingerNum)i);
                MoveFinger((FingerNum)i);
            }
            if (i3 > 0) {
                for (int i = 2; i <= 4; i++) {
                    FingerDesc &prevFinger = mFingers[i - 1];
                    FingerDesc &curFinger = mFingers[i];
                    if (!curFinger.unk0) {
                        if (i == 4) {
                            FixSingleFinger(
                                prevFinger.mFinger01, curFinger.mFinger01, nullptr
                            );
                        } else {
                            FixSingleFinger(
                                prevFinger.mFinger01,
                                curFinger.mFinger01,
                                mFingers[i + 1].mFinger01
                            );
                        }
                    }
                }
            }
            if (mBlendInFrames > 0)
                mBlendInFrames--;
            if (mBlendOutFrames > 0)
                mBlendOutFrames--;
        }
    }
}
