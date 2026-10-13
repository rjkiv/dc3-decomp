#include "char/CharBone.h"
#include "char/CharBoneDir.h"
#include "char/CharBones.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"

CharBone::CharBone()
    : mPositionContext(0), mScaleContext(0), mRotation(CharBones::TYPE_END),
      mRotationContext(0), mTarget(this), mWeights(), mTrans(this),
      mBakeOutAsTopLevel(0) {}

void CharBone::ClearContext(int mask) {
    mPositionContext &= ~mask;
    mScaleContext &= ~mask;
    mRotationContext &= ~mask;
}

BEGIN_HANDLERS(CharBone)
    HANDLE_ACTION(clear_context, ClearContext(_msg->Int(2)))
    HANDLE(get_context_flags, OnGetContextFlags)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(CharBone::WeightContext)
    SYNC_PROP(context, o.mContext)
    SYNC_PROP(weight, o.mWeight)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(CharBone)
    SYNC_PROP(position_context, mPositionContext)
    SYNC_PROP(scale_context, mScaleContext)
    SYNC_PROP(rotation, (int &)mRotation)
    SYNC_PROP(rotation_context, mRotationContext)
    SYNC_PROP(target, mTarget)
    SYNC_PROP(weights, mWeights)
    SYNC_PROP(trans, mTrans)
    SYNC_PROP(bake_out_as_top_level, mBakeOutAsTopLevel)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const CharBone::WeightContext &ctx) {
    bs << ctx.mContext;
    bs << ctx.mWeight;
    return bs;
}

BEGIN_SAVES(CharBone)
    SAVE_REVS(10, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mPositionContext;
    bs << mScaleContext;
    bs << mRotation;
    bs << mRotationContext;
    bs << mTarget;
    bs << mWeights;
    bs << mTrans;
    bs << mBakeOutAsTopLevel;
END_SAVES

BEGIN_COPYS(CharBone)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CharBone)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mRotationContext)
        COPY_MEMBER(mScaleContext)
        COPY_MEMBER(mPositionContext)
        COPY_MEMBER(mRotation)
        COPY_MEMBER(mTarget)
        COPY_MEMBER(mWeights)
        COPY_MEMBER(mTrans)
        COPY_MEMBER(mBakeOutAsTopLevel)
    END_COPYING_MEMBERS
END_COPYS

BinStream &operator>>(BinStream &d, CharBone::WeightContext &w) {
    d >> w.mContext >> w.mWeight;
    return d;
}

INIT_REVS(10, 0)

BEGIN_LOADS(CharBone)
    LOAD_REVS(bs)
    ASSERT_REVS(10, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    if (d.rev < 9) {
        RndTransformableRemover t;
        t.Load(d);
    }
    if (d.rev > 6) {
        d >> mPositionContext;
    } else {
        bool b;
        d >> b;
        mPositionContext = b;
    }
    if (d.rev > 6) {
        d >> mScaleContext;
    } else if (d.rev > 1) {
        bool b;
        d >> b;
        mScaleContext = b;
    }
    d >> (int &)mRotation;
    if (d.rev < 5) {
        int x;
        d >> x;
    }
    if (d.rev < 2) {
        mScaleContext = 0;
        mRotation = (CharBones::Type)(mRotation + 1);
    }
    if (d.rev < 5 && mRotation > CharBones::TYPE_END) {
        mRotation = CharBones::TYPE_END;
    }
    if (d.rev > 6) {
        d >> mRotationContext;
    } else {
        mRotationContext = mRotation != CharBones::TYPE_END;
    }
    if (d.rev > 2 && d.rev < 8) {
        int x;
        d >> x;
    }
    if (d.rev > 3) {
        d >> mTarget;
    }
    if (d.rev == 6) {
        int ctx;
        d >> ctx;
        if (mPositionContext != 0) {
            mPositionContext = ctx;
        }
        if (mScaleContext != 0) {
            mScaleContext = ctx;
        }
        if (mRotationContext != 0) {
            mRotationContext = ctx;
        }
    }
    if (d.rev > 7) {
        d >> mWeights;
    }
    if (d.rev > 8) {
        d >> mTrans;
    }
    if (d.rev > 9) {
        d >> mBakeOutAsTopLevel;
    }
END_LOADS

void CharBone::StuffBones(std::list<CharBones::Bone> &bones, int mask) const {
    if (mPositionContext & mask) {
        Symbol name = CharBones::ChannelName(Name(), CharBones::TYPE_POS);
        CharBones::Bone bone;
        bone.name = name;
        bone.weight = GetWeight(mask);
        bones.push_back(bone);
    }
    if (mScaleContext & mask) {
        Symbol name = CharBones::ChannelName(Name(), CharBones::TYPE_SCALE);
        CharBones::Bone bone;
        bone.name = name;
        bone.weight = GetWeight(mask);
        bones.push_back(bone);
    }
    if (mRotation != CharBones::TYPE_END && mRotationContext & mask) {
        Symbol name = CharBones::ChannelName(Name(), mRotation);
        CharBones::Bone bone;
        bone.name = name;
        bone.weight = GetWeight(mask);
        bones.push_back(bone);
    }
}

const CharBone::WeightContext *CharBone::FindWeight(int ctx) const {
    FOREACH (it, mWeights) {
        if (it->mContext & ctx) {
            return &*it;
        }
    }
    return nullptr;
}

float CharBone::GetWeight(int mask) const {
    const WeightContext *ctx = FindWeight(mask);
    if (ctx) {
        return ctx->mWeight;
    } else {
        return 1.0f;
    }
}

DataNode CharBone::OnGetContextFlags(DataArray *da) {
    CharBoneDir *dir = dynamic_cast<CharBoneDir *>(Dir());
    if (dir)
        return dir->GetContextFlags();
    else {
        MILO_NOTIFY("CharBone: No CharBoneDir for context flags.");
        return DataArrayPtr();
    }
}
