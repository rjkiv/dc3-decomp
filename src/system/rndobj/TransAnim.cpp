#include "rndobj/TransAnim.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/Utl.h"

#pragma region Hmx::Object

RndTransAnim::RndTransAnim()
    : mTrans(this), mTransSpline(false), mScaleSpline(false), mRotSlerp(false),
      mRotSpline(false), mRotKeys(), mTransKeys(), mScaleKeys(), mKeysOwner(this, this),
      mRepeatTrans(false), mFollowPath(false) {}

BEGIN_HANDLERS(RndTransAnim)
    HANDLE(trans, OnTrans)
    HANDLE(splice, OnSplice)
    HANDLE(linearize, OnLinearize)
    HANDLE(set_trans, OnSetTrans)
    HANDLE(remove_rot_keys, OnRemoveRotKeys)
    HANDLE(remove_trans_keys, OnRemoveTransKeys)
    HANDLE(num_trans_keys, OnNumTransKeys)
    HANDLE(num_rot_keys, OnNumRotKeys)
    HANDLE(num_scale_keys, OnNumScaleKeys)
    HANDLE(add_trans_key, OnAddTransKey)
    HANDLE(add_rot_key, OnAddRotKey)
    HANDLE(add_scale_key, OnAddScaleKey)
    HANDLE(set_trans_spline, OnSetTransSpline)
    HANDLE(set_scale_spline, OnSetScaleSpline)
    HANDLE(set_rot_slerp, OnSetRotSlerp)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndTransAnim)
    SYNC_PROP_SET(keys_owner, mKeysOwner.Ptr(), SetKeysOwner(_val.Obj<RndTransAnim>()))
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndTransAnim)
    SAVE_REVS(7, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndAnimatable)
    bs << mTrans << mRotKeys << mTransKeys << mKeysOwner;
    bs << mTransSpline << mRepeatTrans;
    bs << mScaleKeys << mScaleSpline << mFollowPath << mRotSlerp << mRotSpline;
END_SAVES

BEGIN_COPYS(RndTransAnim)
    CREATE_COPY_AS(RndTransAnim, t)
    MILO_ASSERT(t, 0xE6);
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    mTrans = t->mTrans;
    if (ty == kCopyShallow || ty == kCopyFromMax && t->mKeysOwner != t) {
        mKeysOwner = t->mKeysOwner;
    } else {
        mKeysOwner = this;
        mTransKeys = t->mKeysOwner->mTransKeys;
        mRotKeys = t->mKeysOwner->mRotKeys;
        mScaleKeys = t->mKeysOwner->mScaleKeys;
        mTransSpline = t->mKeysOwner->mTransSpline;
        mRepeatTrans = t->mKeysOwner->mRepeatTrans;
        mScaleSpline = t->mKeysOwner->mScaleSpline;
        mFollowPath = t->mKeysOwner->mFollowPath;
        mRotSlerp = t->mKeysOwner->mRotSlerp;
        mRotSpline = t->mKeysOwner->mRotSpline;
    }
END_COPYS

INIT_REVS(7, 0)

BEGIN_LOADS(RndTransAnim)
    LOAD_REVS(bs)
    ASSERT_REVS(7, 0)
    if (d.rev > 4) {
        LOAD_SUPERCLASS(Hmx::Object)
    }
    LOAD_SUPERCLASS(RndAnimatable)
    if (d.rev < 6) {
        RndDrawable::DumpLoad(d.stream);
    }
    d >> mTrans;
    if (d.rev != 2) {
        d >> mRotKeys >> mTransKeys;
    }
    d >> mKeysOwner;
    if (!mKeysOwner) {
        mKeysOwner = this;
    }
    if (d.rev < 3) {
        int numKeys;
        d >> numKeys;
        if (d.rev == 2 || numKeys != 0) {
            mTransKeys.resize(numKeys);
            FOREACH (it, mTransKeys) {
                int i1, i2, i3;
                Vector3 v1, v2;
                d >> it->value >> i1 >> i2 >> i3 >> v1 >> v2 >> it->frame;
            }
        }
        d >> numKeys;
        if (d.rev == 2 || numKeys != 0) {
            mRotKeys.resize(numKeys);
            FOREACH (it, mRotKeys) {
                int i1, i2, i3;
                Hmx::Quat v1, v2;
                d >> it->value >> i1 >> i2 >> i3 >> v1 >> v2 >> it->frame;
            }
        }
        int c0;
        d >> c0;
    }
    if (d.rev > 3) {
        d >> mTransSpline;
    } else {
        int spline;
        d >> spline;
        mTransSpline = spline;
    }
    d >> mRepeatTrans;
    if (d.rev > 3) {
        d >> mScaleKeys >> mScaleSpline;
    } else if (d.rev > 0) {
        if (d.rev != 2) {
            d >> mScaleKeys;
        }
        if (d.rev < 3) {
            int numKeys;
            d >> numKeys;
            if (d.rev == 2 || numKeys != 0) {
                mScaleKeys.resize(numKeys);
                FOREACH (it, mScaleKeys) {
                    int i1, i2, i3;
                    Vector3 v1, v2;
                    d >> it->value >> i1 >> i2 >> i3 >> v1 >> v2 >> it->frame;
                }
            }
        }
        int splinebool;
        d >> splinebool;
        mScaleSpline = splinebool;
    }
    if (d.rev > 1) {
        d >> mFollowPath;
    } else {
        mFollowPath = RotKeys().empty() && TransKeys().size() > 1;
    }
    if (d.rev > 3)
        d >> mRotSlerp;
    if (d.rev > 6)
        d >> mRotSpline;
END_LOADS

void RndTransAnim::Print() {
    TheDebug << "   trans: " << mTrans << "\n";
    TheDebug << "   framesOwner: " << mKeysOwner << "\n";
    TheDebug << "   rotKeys: " << mRotKeys << "\n";
    TheDebug << "   transKeys: " << mTransKeys << "\n";
    TheDebug << "   scaleKeys: " << mScaleKeys << "\n";
    TheDebug << "   transSpline: " << mTransSpline << "\n";
    TheDebug << "   scaleSpline: " << mScaleSpline << "\n";
    TheDebug << "   rotSlerp: " << mRotSlerp << "\n";
    TheDebug << "   rotSpline: " << mRotSpline << "\n";
    TheDebug << "   repeatTrans: " << mRepeatTrans << "\n";
    TheDebug << "   followPath: " << mFollowPath << "\n";
}

#pragma endregion
#pragma region RndAnimatable

void RndTransAnim::SetFrame(float frame, float blend) {
    RndAnimatable::SetFrame(frame, blend);
    if (mTrans) {
        Transform tf(mTrans->LocalXfm());
        MakeTransform(frame, tf, false, blend);
        mTrans->SetLocalXfm(tf);
    }
}

float RndTransAnim::StartFrame() {
    return Min(TransKeys().FirstFrame(), RotKeys().FirstFrame(), ScaleKeys().FirstFrame());
}

float RndTransAnim::EndFrame() {
    return Max(TransKeys().LastFrame(), RotKeys().LastFrame(), ScaleKeys().LastFrame());
}

void RndTransAnim::SetKey(float frame) {
    if (mTrans) {
        TransKeys().Add(mTrans->LocalXfm().v, frame, true);
        Hmx::Matrix3 mtx;
        Normalize(mTrans->LocalXfm().m, mtx);
        RotKeys().Add(Hmx::Quat(mtx), frame, true);
        Vector3 vec;
        MakeScale(mTrans->LocalXfm().m, vec);
        ScaleKeys().Add(vec, frame, true);
    }
}

#pragma endregion
#pragma region RndTransAnim

void RndTransAnim::SetKeysOwner(RndTransAnim *o) {
    MILO_ASSERT(o, 0x2C);
    mKeysOwner = o;
}

void RndTransAnim::SetTrans(RndTransformable *trans) { mTrans = trans; }

void RndTransAnim::MakeTransform(float frame, Transform &tf, bool whole, float blend) {
    float f5 = frame;
    if (mKeysOwner != this) {
        mKeysOwner->MakeTransform(frame, tf, whole, blend);
    } else {
        Vector3 v4c;
        if (!mTransKeys.empty()) {
            Vector3 v58(0, 0, 0);
            if (mRepeatTrans) {
                int iac;
                float &backFrame = mTransKeys.back().frame;
                float &frontFrame = mTransKeys.front().frame;
                f5 = Limit(frontFrame, backFrame, frame, iac);
                Vector3 &frontVec = mTransKeys.front().value;
                Vector3 &backVec = mTransKeys.back().value;
                Subtract(backVec, frontVec, v58);
                v58 *= iac;
            }
            if (blend != 1.0f) {
                Vector3 v64;
                InterpVector(
                    mTransKeys, mTransSpline, f5, v64, mFollowPath ? &v4c : nullptr
                );
                if (mRepeatTrans) {
                    ::Add(v64, v58, v64);
                }
                Interp(tf.v, v64, blend, tf.v);
            } else {
                InterpVector(
                    mTransKeys, mTransSpline, f5, tf.v, mFollowPath ? &v4c : nullptr
                );
                if (mRepeatTrans) {
                    ::Add(tf.v, v58, tf.v);
                }
            }
        } else if (whole) {
            tf.v.Zero();
        }
        Vector3 v70;
        if (!mRotKeys.empty()) {
            Hmx::Quat q80;
            const Key<Hmx::Quat> *prev;
            const Key<Hmx::Quat> *next;
            float ref = 0;
            mRotKeys.AtFrame(f5, prev, next, ref);
            if (mRotSpline)
                QuatSpline(mRotKeys, prev, next, ref, q80);
            else {
                MILO_ASSERT(prev, 0x16D);
                if (mRotSlerp)
                    Interp(prev->value, next->value, ref, q80);
                else
                    FastInterp(prev->value, next->value, ref, q80);
            }
            if (blend != 1.0f) {
                if (!mScaleKeys.empty()) {
                    MakeScale(tf.m, v70);
                    tf.m.x *= 1.0f / v70.x;
                    tf.m.y *= 1.0f / v70.y;
                    tf.m.z *= 1.0f / v70.z;
                }
                Hmx::Quat q90(tf.m);
                if (mRotSlerp || mRotSpline) {
                    Interp(q90, q80, blend, q80);
                } else {
                    FastInterp(q90, q80, blend, q80);
                }
            }
            MakeRotMatrix(q80, tf.m);
        } else if (whole)
            tf.m.Identity();
        if (mFollowPath && !mTransKeys.empty()) {
            if (!mRotKeys.empty()) {
                MakeRotMatrix(v4c, tf.m.z, tf.m);
            } else {
                MakeRotMatrix(v4c, Vector3(0, 0, 1), tf.m);
            }
        }
        if (!mScaleKeys.empty()) {
            Vector3 v9c;
            InterpVector(mScaleKeys, mScaleSpline, f5, v9c, 0);
            if (blend != 1.0f) {
                Interp(v70, v9c, blend, v9c);
            }
            Scale(v9c, tf.m, tf.m);
        }
    }
}

#pragma endregion
#pragma region Handlers

#pragma endregion

DataNode RndTransAnim::OnSetTransSpline(const DataArray *da) {
    SetTransSpline(da->Int(2));
    return 0;
}

DataNode RndTransAnim::OnSetScaleSpline(const DataArray *da) {
    SetScaleSpline(da->Int(2));
    return 0;
}

DataNode RndTransAnim::OnSetRotSlerp(const DataArray *da) {
    SetRotSlerp(da->Int(2));
    return 0;
}

DataNode RndTransAnim::OnTrans(const DataArray *) { return mTrans.Ptr(); }

DataNode RndTransAnim::OnNumTransKeys(const DataArray *) { return TransKeys().NumKeys(); }

DataNode RndTransAnim::OnNumRotKeys(const DataArray *) { return RotKeys().NumKeys(); }

DataNode RndTransAnim::OnNumScaleKeys(const DataArray *) { return ScaleKeys().NumKeys(); }

DataNode RndTransAnim::OnAddTransKey(const DataArray *da) {
    TransKeys().Add(
        Vector3(da->Float(2), da->Float(3), da->Float(4)), da->Float(5), false
    );
    return 0;
}

DataNode RndTransAnim::OnAddScaleKey(const DataArray *da) {
    ScaleKeys().Add(
        Vector3(da->Float(2), da->Float(3), da->Float(4)), da->Float(5), false
    );
    return 0;
}

DataNode RndTransAnim::OnAddRotKey(const DataArray *da) {
    Vector3 vec(da->Float(2), da->Float(3), da->Float(4));
    vec *= DEG2RAD;
    RotKeys().Add(Hmx::Quat(vec), da->Float(5), false);
    return 0;
}

DataNode RndTransAnim::OnSplice(const DataArray *da) {
    SpliceKeys(da->Obj<RndTransAnim>(2), this, da->Float(3), da->Float(4));
    return 0;
}

DataNode RndTransAnim::OnRemoveRotKeys(const DataArray *da) {
    RotKeys().Remove(da->Float(2), da->Float(3));
    return 0;
}

DataNode RndTransAnim::OnRemoveTransKeys(const DataArray *da) {
    TransKeys().Remove(da->Float(2), da->Float(3));
    return 0;
}

DataNode RndTransAnim::OnLinearize(const DataArray *da) {
    float a, b;
    if (da->Size() > 7) {
        a = da->Float(6);
        b = da->Float(7);
    } else {
        a = b = 0;
    }
    LinearizeKeys(this, da->Float(2), da->Float(3) * DEG2RAD, da->Float(4), a, b);
    return 0;
}

DataNode RndTransAnim::OnSetTrans(const DataArray *da) {
    SetTrans(da->Obj<RndTransformable>(2));
    return 0;
}
