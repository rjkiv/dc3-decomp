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
    : mHand(nullptr), mForeArm(nullptr), mUpperArm(nullptr), mBlendInFrames(0),
      mBlendOutFrames(0), mResetHandDest(1), mResetCurHandTrans(1),
      mFingerCurledLength(0.85), mHandMoveForward(1), mHandPinkyRotation(-0.06),
      mHandThumbRotation(0.23), mHandDestOffset(-0.4), mIsRightHand(1), mMoveHand(0),
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

INIT_REVS(5, 0)

BEGIN_LOADS(CharIKFingers)
    LOAD_REVS(bs)
    ASSERT_REVS(5, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(CharWeightable)
    if (d.rev > 1)
        d >> mIsRightHand;
    if (d.rev > 2)
        d >> mOutputTrans;
    if (d.rev > 3)
        d >> mKeyboardRefBone;
    if (d.rev > 4) {
        d >> mHandKeyboardOffset;
        d >> mHandThumbRotation;
        d >> mHandPinkyRotation;
        d >> mHandMoveForward;
        d >> mHandDestOffset;
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
                -0.023f, 0.979f, 0.201f, -0.228f, 0.191f, -0.955f, -0.972, -0.068f, 0.218f
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
                -0.067f, 0.985f, 0.156f, 0.224f, 0.167f, -0.96f, -0.972f, -0.029f, -0.232f
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
    if (mHand && mIsSetup) {
        Hmx::Matrix3 mtx58;
        Hmx::Matrix3 mtx7c;
        Invert(mKeyboardRefBone->WorldXfm().m, mtx58);
        Multiply(mHand->WorldXfm().m, mtx58, mtx7c);
        Vector3 v88;
        Subtract(mKeyboardRefBone->WorldXfm().v, mHand->WorldXfm().v, v88);
        if (Weight() < 1.0) {
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
            int i1 = -1;
            for (int i = 0; i < 5; i++) {
                if (mFingers.begin()[i].unk0) {
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
    for (int i = 0; i < kNumFingers; i++) {
        RndTransformable *finger2 = mFingers[i].mFinger02;
        RndTransformable *finger3 = mFingers[i].mFinger03;
        RndTransformable *fingertip = mFingers[i].mFingertip;
        if (finger2 && finger3 && fingertip) {
            float len = Length(finger3->LocalXfm().v) + Length(fingertip->LocalXfm().v);
            mFingers[i].unk4 = Length(finger2->LocalXfm().v) + len;
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
    if (desc.unk0 || desc.unk88 > 0 || desc.unk8c > 0) {
        Transform tf68;
        Transform tf98 = mDestHandTrans;
        if (desc.mFinger01->TransParent() != mHand) {
            Multiply(desc.mFinger01->TransParent()->LocalXfm(), mDestHandTrans, tf98);
        }
        Multiply(desc.mFinger01->LocalXfm(), tf98, tf68);
        float f6 = 1.0f;
        if (desc.unk88 > 0 || desc.unk8c > 0) {
            if (desc.unk88 > 0) {
                f6 = 1.0f - desc.unk88 / 5.0f;
            } else if (desc.unk8c > 0) {
                f6 = 1.0f - desc.unk8c / 5.0f;
            }
        }
        Interp(desc.unk80, desc.unk78, f6, desc.unk80);
        Interp(desc.unk84, desc.unk7c, f6, desc.unk84);
        Hmx::Matrix3 me0;
        Vector3 v180(0, 0, desc.unk80);
        MakeRotMatrix(v180, me0, true);
        desc.mFinger02->SetLocalRot(me0);
        v180.Set(0, 0, desc.unk84);
        MakeRotMatrix(v180, me0, true);
        desc.mFinger03->SetLocalRot(me0);
        Interp(desc.unka4, desc.unk94, f6, desc.unka4);
        Hmx::Quat q138;
        MakeRotQuat(tf68.m.x, desc.unka4, q138);
        Transform tfe8;
        Multiply(tf68.m.x, q138, tfe8.m.x);
        Multiply(tf68.m.y, q138, tfe8.m.y);
        Multiply(tf68.m.z, q138, tfe8.m.z);
        Normalize(tfe8.m, tfe8.m);
        tfe8.v = tf68.v;
        Transform tf118;
        Invert(tf98, tf118);
        Multiply(tfe8, tf118, desc.mFinger01->DirtyLocalXfm());
        if (desc.unk8c > 0) {
            desc.unk8c--;
        }
        if (desc.unk88 > 0) {
            desc.unk88--;
        }
    }
}

void CharIKFingers::CalculateFingerDest(FingerNum num) {
    if (mOutputTrans) {
        FingerDesc &cur = mFingers[num];
        if (cur.unk90) {
            Transform tf78;
            Multiply(cur.mFinger01->LocalXfm(), mOutputTrans->WorldXfm(), tf78);
            cur.unka4 = tf78.m.x;
            Vector3 v1cc;
            MakeEuler(cur.mFinger02->LocalXfm().m, v1cc);
            Vector3 v1d8;
            MakeEuler(cur.mFinger03->LocalXfm().m, v1d8);
            cur.unk80 = v1cc.z;
            cur.unk84 = v1d8.z;
            cur.unk90 = false;
        }
        if (cur.unkb4) {
            if (cur.unk0) {
                Transform tfa8;
                Transform tfd8;
                Transform tf108;
                Transform tf138;
                if (cur.mFinger01->TransParent() != mHand) {
                    Transform tf168;
                    Multiply(
                        cur.mFinger01->TransParent()->LocalXfm(), mDestHandTrans, tf168
                    );
                    Multiply(cur.mFinger01->LocalXfm(), tf168, tfa8);
                } else {
                    Multiply(cur.mFinger01->LocalXfm(), mDestHandTrans, tfa8);
                }

                Multiply(cur.mFinger02->LocalXfm(), tfa8, tfd8);
                Multiply(cur.mFinger03->LocalXfm(), tfd8, tf108);
                Multiply(cur.mFingertip->LocalXfm(), tf108, tf138);
                Vector3 v1e4;
                if (Distance(tf138.v, cur.unk18) < Distance(tf138.v, cur.unk8)) {
                    v1e4 = cur.unk8;
                } else {
                    v1e4 = cur.unk18;
                }

                Hmx::Matrix3 m190;
                Vector3 v1f0 = tfa8.m.y;
                Vector3 v1fc = tfa8.m.x;
                Vector3 v208 = tfa8.m.z;
                Vector3 v214 = tfa8.v;
                Vector3 v220;
                Subtract(v1e4, v214, v220);

                float len5 = Length(cur.mFinger02->LocalXfm().v);
                float len6 = Length(cur.mFingertip->LocalXfm().v);
                float len7 = Length(cur.mFinger03->LocalXfm().v);
                float len8 = Length(v220);
                float f9 = std::acos(
                    -((len8 - len7) * (len8 - len7) - (len5 * len5 + len6 * len6))
                    / (len5 * 2.0f * len6)
                );
                if (f9 < 0.87f)
                    f9 = 0.87f;
                float f5 = f9 / 2.0f + (PI / 2);
                cur.unk78 = PI - f5;
                cur.unk7c = PI - f5;
                Hmx::Quat q230;
                q230.Set(v208, -(f5 * 2.0f - 2 * PI) / 2.0f);
                Multiply(v1fc, q230, v1fc);
                Hmx::Quat q240;
                MakeRotQuat(v1fc, v220, q240);
                Multiply(tfa8.m.x, q240, cur.unk94);
            } else {
                Transform tf1c0;
                Multiply(cur.mFinger01->LocalXfm(), mOutputTrans->WorldXfm(), tf1c0);
                cur.unk94 = tf1c0.m.x;
                Vector3 v24c;
                MakeEuler(cur.mFinger02->LocalXfm().m, v24c);
                Vector3 v258;
                MakeEuler(cur.mFinger03->LocalXfm().m, v258);
                cur.unk78 = v24c.z;
                cur.unk7c = v258.z;
            }
            cur.unkb4 = false;
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
            Vector3 v1a0(mHandDestOffset, 0, 0);
            if (!mIsRightHand) {
                Scale(v1a0, -1.0f, v1a0);
            }
            Multiply(v1a0, mKeyboardRefBone->WorldXfm().m, v1a0);
            Hmx::Matrix3 m134;
            Multiply(mtx, mKeyboardRefBone->WorldXfm().m, m134);
            Normalize(m134, mDestHandTrans.m);
            for (int i = 0; i < kNumFingers; i++) {
                FingerDesc &curDesc = mFingers[i];
                if (curDesc.unk0) {
                    Add(curDesc.unk8, v194, v194);
                    Vector3 v1ac;
                    Scale(v1a0, i - 2.0, v1ac);
                    Add(v1ac, v194, v194);
                    if (i == 0) {
                        Hmx::Matrix3 m158;
                        float f5 = mHandThumbRotation;
                        if (!mIsRightHand)
                            f5 *= -1.0f;
                        MakeRotMatrixY(f5, m158);
                        Multiply(m158, m134, mDestHandTrans.m);
                        b1 = true;
                    } else if (i == 4) {
                        Hmx::Matrix3 m17c;
                        float f5 = mHandPinkyRotation;
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
                v188.y += mHandMoveForward;
            }
            Add(mHandKeyboardOffset, v188, v188);
            Multiply(v188, mKeyboardRefBone->WorldXfm().m, v188);
            Vector3 v1b8;
            Add(v194, v188, v1b8);
            mDestHandTrans.v.Set(v1b8.x, v1b8.y, v1b8.z);
        }
        mMoveHand = false;
    }
}
