#include "char/CharIKFingers.h"
#include "char/CharWeightable.h"
#include "math/Mtx.h"
#include "math/Rot.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/Rnd.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"

CharIKFingers::CharIKFingers()
    : mHand(nullptr), mElbow(nullptr), mShoulder(nullptr), mBlendInFrames(0),
      mBlendOutFrames(0), mResetHandDest(1), mResetCurHandTrans(1),
      mFingerCurledLength(0.85), mMoveForward(1), mPinkyRot(-0.06), mThumbRot(0.23),
      mDestPosOffset(-0.4), mIsRightHand(1), mMoveHand(0), mIsSetup(0),
      mHandOutputTrans(this), mKeyboardRefBone(this) {
    mFingers.resize(5);
    mCurHandTrans.Zero();
    mDestHandTrans.Zero();
    mDefaultOffset = Vector3(0.3f, -6.0f, 0.4f);
    mDefaultRotation = Hmx::Matrix3();
}

CharIKFingers::~CharIKFingers() {}

BEGIN_HANDLERS(CharIKFingers)
    HANDLE_SUPERCLASS(CharWeightable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharIKFingers)
    SYNC_PROP(is_right_hand, mIsRightHand)
    SYNC_PROP(output_trans, mHandOutputTrans)
    SYNC_PROP(keyboard_ref_bone, mKeyboardRefBone)
    SYNC_PROP(hand_keyboard_offset, mDefaultOffset)
    SYNC_PROP(hand_thumb_rotation, mThumbRot)
    SYNC_PROP(hand_pinky_rotation, mPinkyRot)
    SYNC_PROP(hand_move_forward, mMoveForward)
    SYNC_PROP(hand_dest_offset, mDestPosOffset)
    SYNC_SUPERCLASS(CharWeightable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharIKFingers)
    SAVE_REVS(5, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(CharWeightable)
    bs << mIsRightHand;
    bs << mHandOutputTrans;
    bs << mKeyboardRefBone;
    bs << mDefaultOffset;
    bs << mThumbRot;
    bs << mPinkyRot;
    bs << mMoveForward;
    bs << mDestPosOffset;
END_SAVES

BEGIN_COPYS(CharIKFingers)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(CharWeightable)
    CREATE_COPY(CharIKFingers)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mIsRightHand)
        COPY_MEMBER(mHandOutputTrans)
        COPY_MEMBER(mKeyboardRefBone)
        COPY_MEMBER(mDefaultOffset)
        COPY_MEMBER(mThumbRot)
        COPY_MEMBER(mPinkyRot)
        COPY_MEMBER(mMoveForward)
        COPY_MEMBER(mDestPosOffset)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(5, 0)

BEGIN_LOADS(CharIKFingers)
    LOAD_REVS(bs)
    ASSERT_REVS(5, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(CharWeightable)
    if (d.rev > 1)
        d >> mIsRightHand;
    if (d.rev > 2)
        d >> mHandOutputTrans;
    if (d.rev > 3)
        d >> mKeyboardRefBone;
    if (d.rev > 4) {
        d >> mDefaultOffset;
        d >> mThumbRot;
        d >> mPinkyRot;
        d >> mMoveForward;
        d >> mDestPosOffset;
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
            mElbow = dir->Find<RndTransformable>("bone_R-foreArm.mesh", false);
            mShoulder = dir->Find<RndTransformable>("bone_R-upperArm.mesh", false);
            mFingers[kFingerThumb].mBone1 =
                dir->Find<RndTransformable>("bone_R-thumb01.mesh", false);
            mFingers[kFingerThumb].mBone2 =
                dir->Find<RndTransformable>("bone_R-thumb02.mesh", false);
            mFingers[kFingerThumb].mBone3 =
                dir->Find<RndTransformable>("bone_R-thumb03.mesh", false);
            mFingers[kFingerThumb].mBoneTip =
                dir->Find<RndTransformable>("spot_R-thumb_tip.mesh", false);
            mFingers[kFingerIndex].mBone1 =
                dir->Find<RndTransformable>("bone_R-index01.mesh", false);
            mFingers[kFingerIndex].mBone2 =
                dir->Find<RndTransformable>("bone_R-index02.mesh", false);
            mFingers[kFingerIndex].mBone3 =
                dir->Find<RndTransformable>("bone_R-index03.mesh", false);
            mFingers[kFingerIndex].mBoneTip =
                dir->Find<RndTransformable>("spot_R-index_tip.mesh", false);
            mFingers[kFingerMiddle].mBone1 =
                dir->Find<RndTransformable>("bone_R-middlefinger01.mesh", false);
            mFingers[kFingerMiddle].mBone2 =
                dir->Find<RndTransformable>("bone_R-middlefinger02.mesh", false);
            mFingers[kFingerMiddle].mBone3 =
                dir->Find<RndTransformable>("bone_R-middlefinger03.mesh", false);
            mFingers[kFingerMiddle].mBoneTip =
                dir->Find<RndTransformable>("spot_R-middlefinger_tip.mesh", false);
            mFingers[kFingerRing].mBone1 =
                dir->Find<RndTransformable>("bone_R-ringfinger01.mesh", false);
            mFingers[kFingerRing].mBone2 =
                dir->Find<RndTransformable>("bone_R-ringfinger02.mesh", false);
            mFingers[kFingerRing].mBone3 =
                dir->Find<RndTransformable>("bone_R-ringfinger03.mesh", false);
            mFingers[kFingerRing].mBoneTip =
                dir->Find<RndTransformable>("spot_R-ringfinger_tip.mesh", false);
            mFingers[kFingerPinky].mBone1 =
                dir->Find<RndTransformable>("bone_R-pinky01.mesh", false);
            mFingers[kFingerPinky].mBone2 =
                dir->Find<RndTransformable>("bone_R-pinky02.mesh", false);
            mFingers[kFingerPinky].mBone3 =
                dir->Find<RndTransformable>("bone_R-pinky03.mesh", false);
            mFingers[kFingerPinky].mBoneTip =
                dir->Find<RndTransformable>("spot_R-pinky_tip.mesh", false);
            mDefaultRotation = Hmx::Matrix3(
                -0.023f, 0.979f, 0.201f, -0.228f, 0.191f, -0.955f, -0.972, -0.068f, 0.218f
            );
            Normalize(mDefaultRotation, mDefaultRotation);
            mIsSetup = true;
        } else {
            mHand = dir->Find<RndTransformable>("bone_L-hand.mesh", false);
            mElbow = dir->Find<RndTransformable>("bone_L-foreArm.mesh", false);
            mShoulder = dir->Find<RndTransformable>("bone_L-upperArm.mesh", false);
            mFingers[kFingerThumb].mBone1 =
                dir->Find<RndTransformable>("bone_L-thumb01.mesh", false);
            mFingers[kFingerThumb].mBone2 =
                dir->Find<RndTransformable>("bone_L-thumb02.mesh", false);
            mFingers[kFingerThumb].mBone3 =
                dir->Find<RndTransformable>("bone_L-thumb03.mesh", false);
            mFingers[kFingerThumb].mBoneTip =
                dir->Find<RndTransformable>("spot_L-thumb_tip.mesh", false);
            mFingers[kFingerIndex].mBone1 =
                dir->Find<RndTransformable>("bone_L-index01.mesh", false);
            mFingers[kFingerIndex].mBone2 =
                dir->Find<RndTransformable>("bone_L-index02.mesh", false);
            mFingers[kFingerIndex].mBone3 =
                dir->Find<RndTransformable>("bone_L-index03.mesh", false);
            mFingers[kFingerIndex].mBoneTip =
                dir->Find<RndTransformable>("spot_L-index_tip.mesh", false);
            mFingers[kFingerMiddle].mBone1 =
                dir->Find<RndTransformable>("bone_L-middlefinger01.mesh", false);
            mFingers[kFingerMiddle].mBone2 =
                dir->Find<RndTransformable>("bone_L-middlefinger02.mesh", false);
            mFingers[kFingerMiddle].mBone3 =
                dir->Find<RndTransformable>("bone_L-middlefinger03.mesh", false);
            mFingers[kFingerMiddle].mBoneTip =
                dir->Find<RndTransformable>("spot_L-middlefinger_tip.mesh", false);
            mFingers[kFingerRing].mBone1 =
                dir->Find<RndTransformable>("bone_L-ringfinger01.mesh", false);
            mFingers[kFingerRing].mBone2 =
                dir->Find<RndTransformable>("bone_L-ringfinger02.mesh", false);
            mFingers[kFingerRing].mBone3 =
                dir->Find<RndTransformable>("bone_L-ringfinger03.mesh", false);
            mFingers[kFingerRing].mBoneTip =
                dir->Find<RndTransformable>("spot_L-ringfinger_tip.mesh", false);
            mFingers[kFingerPinky].mBone1 =
                dir->Find<RndTransformable>("bone_L-pinky01.mesh", false);
            mFingers[kFingerPinky].mBone2 =
                dir->Find<RndTransformable>("bone_L-pinky02.mesh", false);
            mFingers[kFingerPinky].mBone3 =
                dir->Find<RndTransformable>("bone_L-pinky03.mesh", false);
            mFingers[kFingerPinky].mBoneTip =
                dir->Find<RndTransformable>("spot_L-pinky_tip.mesh", false);
            mDefaultRotation = Hmx::Matrix3(
                -0.067f, 0.985f, 0.156f, 0.224f, 0.167f, -0.96f, -0.972f, -0.029f, -0.232f
            );
            Normalize(mDefaultRotation, mDefaultRotation);
            mIsSetup = true;
        }
        for (int i = 0; i < 5; i++) {
            FingerDesc cur = mFingers[i];
            if (!cur.mBone1 || !cur.mBone2 || !cur.mBone3 || !cur.mBoneTip) {
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
        if (desc.mBone1) {
            changedBy.push_back(desc.mBone1);
        }
        if (desc.mBone2) {
            changedBy.push_back(desc.mBone2);
        }
        if (desc.mBone3) {
            changedBy.push_back(desc.mBone3);
        }
        if (desc.mBoneTip) {
            changedBy.push_back(desc.mBoneTip);
        }
    }
    if (mElbow) {
        change.push_back(mElbow);
        changedBy.push_back(mElbow);
    }
    if (mShoulder) {
        change.push_back(mShoulder);
        changedBy.push_back(mShoulder);
    }
}

void CharIKFingers::Highlight() {
    for (int i = 0; i < 5; i++) {
        FingerDesc desc(mFingers[i]);
        if (desc.mIsControlled) {
            UtilDrawSphere(desc.mDestPos, 0.2f, Hmx::Color(1, 0, 0), 0);
            UtilDrawSphere(desc.mSecondDestPos, 0.2f, Hmx::Color(0, 1, 0), 0);
            UtilDrawAxes(desc.mBone1->WorldXfm(), 1.0f, Hmx::Color(1, 1, 1));
            TheRnd.DrawLine(
                desc.mBone1->WorldXfm().v,
                desc.mBone2->WorldXfm().v,
                Hmx::Color(1, 1, 1),
                false
            );
            TheRnd.DrawLine(
                desc.mBone2->WorldXfm().v,
                desc.mBone3->WorldXfm().v,
                Hmx::Color(1, 1, 1),
                false
            );
            TheRnd.DrawLine(
                desc.mBone3->WorldXfm().v,
                desc.mBoneTip->WorldXfm().v,
                Hmx::Color(1, 1, 1),
                false
            );
        }
    }
}

void CharIKFingers::Poll() {
    if (mHand && mIsSetup) {
        Hmx::Matrix3 mtx58;
        Hmx::Matrix3 mtx7c;
        Invert(mKeyboardRefBone->WorldXfm().m, mtx58);
        Multiply(mHand->WorldXfm().m, mtx58, mtx7c);
        Vector3 v88;
        Subtract(mKeyboardRefBone->WorldXfm().v, mHand->WorldXfm().v, v88);
        if (Weight() < 1.0) {
            if (mHandOutputTrans) {
                mHandOutputTrans->SetWorldXfm(mHand->WorldXfm());
            }
        } else {
            if (mResetCurHandTrans) {
                mCurHandTrans.Set(mHand->WorldXfm().m, mHand->WorldXfm().v);
                mDestHandTrans.Set(mHand->WorldXfm().m, mHand->WorldXfm().v);
                mResetCurHandTrans = false;
            }
            int i3 = 0;
            int i1 = -1;
            for (int i = 0; i < 5; i++) {
                if (mFingers.begin()[i].mIsControlled) {
                    if (i1 == -1) {
                        i1 = i;
                    }
                    i3++;
                }
            }
            CalculateHandDest(i3, i1);
            float f8 = 1;
            if (mBlendInFrames > 0) {
                f8 -= mBlendInFrames / 5.0f;
            } else if (mBlendOutFrames > 0) {
                f8 -= mBlendOutFrames / 5.0f;
            }
            Interp(mCurHandTrans.v, mDestHandTrans.v, f8, mCurHandTrans.v);
            Interp(mCurHandTrans.m, mDestHandTrans.m, f8, mCurHandTrans.m);
            if (mHandOutputTrans) {
                mHandOutputTrans->SetWorldXfm(mCurHandTrans);
            }
            for (int i = 0; i < 5; i++) {
                CalculateFingerDest((FingerNum)i);
                MoveFinger((FingerNum)i);
            }
            if (i3 > 0) {
                for (int i = 2; i <= 4; i++) {
                    FingerDesc &prevFinger = mFingers[i - 1];
                    FingerDesc &curFinger = mFingers[i];
                    if (!curFinger.mIsControlled) {
                        if (i == 4) {
                            FixSingleFinger(prevFinger.mBone1, curFinger.mBone1, nullptr);
                        } else {
                            FixSingleFinger(
                                prevFinger.mBone1,
                                curFinger.mBone1,
                                mFingers[i + 1].mBone1
                            );
                        }
                    }
                }
            }
            if (mBlendInFrames > 0) {
                mBlendInFrames--;
            }
            if (mBlendOutFrames > 0) {
                mBlendOutFrames--;
            }
        }
    }
}

void CharIKFingers::MeasureLengths() {
    for (int i = 0; i < kFingerNone; i++) {
        RndTransformable *finger2 = mFingers[i].mBone2;
        RndTransformable *finger3 = mFingers[i].mBone3;
        RndTransformable *fingertip = mFingers[i].mBoneTip;
        if (finger2 && finger3 && fingertip) {
            float len = Length(finger3->LocalXfm().v) + Length(fingertip->LocalXfm().v);
            mFingers[i].mLength = Length(finger2->LocalXfm().v) + len;
        }
    }
    if (mHand && mHand->TransParent() && mHand->TransParent()->TransParent()) {
        mInv2ab = 2;
        mAAPlusBB = 0;
        float len = Length(mHand->LocalXfm().v);
        mInv2ab *= len;
        mAAPlusBB += len * len;
        float parentLen = Length(mHand->TransParent()->LocalXfm().v);
        mInv2ab = 1.0f / (mInv2ab * parentLen);
        mAAPlusBB += parentLen * parentLen;
    }
}

void CharIKFingers::FixSingleFinger(
    RndTransformable *t1, RndTransformable *t2, RndTransformable *t3
) {
    Vector3 va8;
    if (t3) {
        Add(t1->WorldXfm().m.x, t3->WorldXfm().m.x, va8);
        Scale(va8, 0.5f, va8);
    } else {
        va8 = t1->WorldXfm().m.x;
    }
    Hmx::Quat rot;
    MakeRotQuat(t2->WorldXfm().m.x, va8, rot);
    Transform tf68;
    Multiply(t2->WorldXfm().m.x, rot, tf68.m.x);
    Multiply(t2->WorldXfm().m.y, rot, tf68.m.y);
    Multiply(t2->WorldXfm().m.z, rot, tf68.m.z);
    tf68.v = t2->WorldXfm().v;
    Transform tf98;
    Invert(t2->TransParent()->WorldXfm(), tf98);
    Multiply(tf68, tf98, t2->DirtyLocalXfm());
}

void CharIKFingers::MoveFinger(FingerNum num) {
    FingerDesc &desc = mFingers[num];
    if (desc.mIsControlled || desc.mFBlendInFrames > 0 || desc.mFBlendOutFrames > 0) {
        Transform tf68;
        Transform tf98 = mDestHandTrans;
        if (desc.mBone1->TransParent() != mHand) {
            Multiply(desc.mBone1->TransParent()->LocalXfm(), mDestHandTrans, tf98);
        }
        Multiply(desc.mBone1->LocalXfm(), tf98, tf68);
        float f6 = 1.0f;
        if (desc.mFBlendInFrames > 0 || desc.mFBlendOutFrames > 0) {
            if (desc.mFBlendInFrames > 0) {
                f6 = 1.0f - desc.mFBlendInFrames / 5.0f;
            } else if (desc.mFBlendOutFrames > 0) {
                f6 = 1.0f - desc.mFBlendOutFrames / 5.0f;
            }
        }
        Interp(desc.mBone2CurAngle, desc.mBone2DestAngle, f6, desc.mBone2CurAngle);
        Interp(desc.mBone3CurAngle, desc.mBone3DestAngle, f6, desc.mBone3CurAngle);
        Hmx::Matrix3 me0;
        Vector3 v180(0, 0, desc.mBone2CurAngle);
        MakeRotMatrix(v180, me0, true);
        desc.mBone2->SetLocalRot(me0);
        v180.Set(0, 0, desc.mBone3CurAngle);
        MakeRotMatrix(v180, me0, true);
        desc.mBone3->SetLocalRot(me0);
        Interp(
            desc.mCurForwardVector, desc.mDestForwardVector, f6, desc.mCurForwardVector
        );
        Hmx::Quat q138;
        MakeRotQuat(tf68.m.x, desc.mCurForwardVector, q138);
        Transform tfe8;
        Multiply(tf68.m.x, q138, tfe8.m.x);
        Multiply(tf68.m.y, q138, tfe8.m.y);
        Multiply(tf68.m.z, q138, tfe8.m.z);
        Normalize(tfe8.m, tfe8.m);
        tfe8.v = tf68.v;
        Transform tf118;
        Invert(tf98, tf118);
        Multiply(tfe8, tf118, desc.mBone1->DirtyLocalXfm());
        if (desc.mFBlendOutFrames > 0) {
            desc.mFBlendOutFrames--;
        }
        if (desc.mFBlendInFrames > 0) {
            desc.mFBlendInFrames--;
        }
    }
}

void CharIKFingers::CalculateFingerDest(FingerNum num) {
    if (mHandOutputTrans) {
        FingerDesc &cur = mFingers[num];
        if (cur.mResetFingerRots) {
            Transform tf78;
            Multiply(cur.mBone1->LocalXfm(), mHandOutputTrans->WorldXfm(), tf78);
            cur.mCurForwardVector = tf78.m.x;
            Vector3 v1cc;
            MakeEuler(cur.mBone2->LocalXfm().m, v1cc);
            Vector3 v1d8;
            MakeEuler(cur.mBone3->LocalXfm().m, v1d8);
            cur.mBone2CurAngle = v1cc.z;
            cur.mBone3CurAngle = v1d8.z;
            cur.mResetFingerRots = false;
        }
        if (cur.mDestChanged) {
            if (cur.mIsControlled) {
                Transform tfa8;
                Transform tfd8;
                Transform tf108;
                Transform tf138;
                if (cur.mBone1->TransParent() != mHand) {
                    Transform tf168;
                    Multiply(cur.mBone1->TransParent()->LocalXfm(), mDestHandTrans, tf168);
                    Multiply(cur.mBone1->LocalXfm(), tf168, tfa8);
                } else {
                    Multiply(cur.mBone1->LocalXfm(), mDestHandTrans, tfa8);
                }

                Multiply(cur.mBone2->LocalXfm(), tfa8, tfd8);
                Multiply(cur.mBone3->LocalXfm(), tfd8, tf108);
                Multiply(cur.mBoneTip->LocalXfm(), tf108, tf138);
                Vector3 v1e4;
                if (Distance(tf138.v, cur.mSecondDestPos)
                    < Distance(tf138.v, cur.mDestPos)) {
                    v1e4 = cur.mDestPos;
                } else {
                    v1e4 = cur.mSecondDestPos;
                }

                Hmx::Matrix3 m190;
                Vector3 v1f0 = tfa8.m.y;
                Vector3 v1fc = tfa8.m.x;
                Vector3 v208 = tfa8.m.z;
                Vector3 v214 = tfa8.v;
                Vector3 v220;
                Subtract(v1e4, v214, v220);

                float len5 = Length(cur.mBone2->LocalXfm().v);
                float len6 = Length(cur.mBoneTip->LocalXfm().v);
                float len7 = Length(cur.mBone3->LocalXfm().v);
                float len8 = Length(v220);
                float f9 = std::acos(
                    -((len8 - len7) * (len8 - len7) - (len5 * len5 + len6 * len6))
                    / (len5 * 2.0f * len6)
                );
                if (f9 < 0.87f)
                    f9 = 0.87f;
                float f5 = f9 / 2.0f + (PI / 2);
                cur.mBone2DestAngle = PI - f5;
                cur.mBone3DestAngle = PI - f5;
                Hmx::Quat q230;
                q230.Set(v208, -(f5 * 2.0f - 2 * PI) / 2.0f);
                Multiply(v1fc, q230, v1fc);
                Hmx::Quat q240;
                MakeRotQuat(v1fc, v220, q240);
                Multiply(tfa8.m.x, q240, cur.mDestForwardVector);
            } else {
                Transform tf1c0;
                Multiply(cur.mBone1->LocalXfm(), mHandOutputTrans->WorldXfm(), tf1c0);
                cur.mDestForwardVector = tf1c0.m.x;
                Vector3 v24c;
                MakeEuler(cur.mBone2->LocalXfm().m, v24c);
                Vector3 v258;
                MakeEuler(cur.mBone3->LocalXfm().m, v258);
                cur.mBone2DestAngle = v24c.z;
                cur.mBone3DestAngle = v258.z;
            }
            cur.mDestChanged = false;
        }
    }
}

void CharIKFingers::CalculateHandDest(int i1, int i2) {
    Transform tf110(mHand->WorldXfm()); // auStack_110
    if (mMoveHand) {
        if (i1 > 0) {
            Vector3 v188(0, 0, 0);
            FingerDesc desc(mFingers[i2]);
            Vector3 v194;
            v194.Zero();
            bool b1 = false;
            Vector3 v1a0(mDestPosOffset, 0, 0);
            if (!mIsRightHand) {
                Scale(v1a0, -1.0f, v1a0);
            }
            Multiply(v1a0, mKeyboardRefBone->WorldXfm().m, v1a0);
            Hmx::Matrix3 m134;
            Multiply(mDefaultRotation, mKeyboardRefBone->WorldXfm().m, m134);
            Normalize(m134, mDestHandTrans.m);
            for (int i = 0; i < kFingerNone; i++) {
                FingerDesc &curDesc = mFingers[i];
                if (curDesc.mIsControlled) {
                    Add(curDesc.mDestPos, v194, v194);
                    Vector3 v1ac;
                    Scale(v1a0, i - 2.0, v1ac);
                    Add(v1ac, v194, v194);
                    if (i == 0) {
                        Hmx::Matrix3 m158;
                        float f5 = mThumbRot;
                        if (!mIsRightHand)
                            f5 *= -1.0f;
                        MakeRotMatrixY(f5, m158);
                        Multiply(m158, m134, mDestHandTrans.m);
                        b1 = true;
                    } else if (i == 4) {
                        Hmx::Matrix3 m17c;
                        float f5 = mPinkyRot;
                        if (!mIsRightHand)
                            f5 *= -1.0f;
                        MakeRotMatrixY(f5, m17c);
                        Multiply(m17c, m134, mDestHandTrans.m);
                        b1 = true;
                    }
                }
            }
            Scale(v194, 1.0f / i1, v194);
            if (b1) {
                v188.y += mMoveForward;
            }
            Add(mDefaultOffset, v188, v188);
            Multiply(v188, mKeyboardRefBone->WorldXfm().m, v188);
            Vector3 v1b8;
            Add(v194, v188, v1b8);
            mDestHandTrans.v.Set(v1b8.x, v1b8.y, v1b8.z);
        }
        mMoveHand = false;
    }
}
