#include "hamobj/HamCamShot.h"
#include "HamCamShot.h"
#include "char/Character.h"
#include "flow/PropertyEventProvider.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/Symbol.h"
#include "world/CameraShot.h"

HamCamShot *gHamCamShot;
std::list<HamCamShot::TargetCache> HamCamShot::sCache;

HamCamShot::HamCamShot()
    : mTargets(this), mMinTime(0), mMaxTime(0), mZeroTime(0), mPlayerFlag(kHamPlayerOff),
      mNextShots(this), mCurrentShot(this), unk2cc(0), unk2d0(0), unk2d4(0), unk2d8(0),
      unk2dc(0), unk2dd(0), mMasterAnims(this), unk2f4(0), unk2f8(this), unk30c(this),
      unk320(this), unk340(this), unk354(this), unk368(this), unk388(false) {
    mNearPlane = 10;
    mFarPlane = 10000;
    unk2b4 = 0;
}

BEGIN_HANDLERS(HamCamShot)
    HANDLE(test_delta, OnTestDelta)
    HANDLE_EXPR(duration_seconds, GetTotalDurationSeconds())
    HANDLE_EXPR(duration, GetTotalDuration())
    HANDLE_ACTION(store, Store())
    HANDLE(add_target, AddTarget)
    HANDLE_EXPR(initial_shot, InitialShot())
    HANDLE_EXPR(num_shots, GetNumShots())
    HANDLE(allowable_next_shots, OnAllowableNextShots)
    HANDLE(list_all_next_shots, OnListAllNextShots)
    HANDLE_EXPR(find_target, FindTarget(_msg->Sym(2)))
    HANDLE(list_targets, OnListTargets)
    HANDLE_EXPR(get_original_size_next_shots, unk2f4)
    HANDLE_ACTION(flip_target_anim_groups, FlipTargetAnimGroups())
    HANDLE_SUPERCLASS(CamShot)
END_HANDLERS

#define SYNC_PROP_SET_TARGET_BIT(s, member)                                              \
    {                                                                                    \
        _NEW_STATIC_SYMBOL(s)                                                            \
        if (sym == _s) {                                                                 \
            if (_op == kPropSet) {                                                       \
                member = _val.Int();                                                     \
            } else {                                                                     \
                _val = member;                                                           \
            }                                                                            \
            return true;                                                                 \
        }                                                                                \
    }

BEGIN_CUSTOM_PROPSYNC(HamCamShot::Target)
    SYNC_PROP_SET(target, o.mTarget, o.UpdateTarget(_val.Sym(), gHamCamShot))
    SYNC_PROP(to, o.mTo)
    SYNC_PROP_MODIFY(anim_group, o.mAnimGroup, gHamCamShot->StartAnim())
    SYNC_PROP(fast_forward, o.mFastForward)
    SYNC_PROP(forward_event, o.mForwardEvent)
    SYNC_PROP_SET_TARGET_BIT(force_lod, o.mForceLOD)
    SYNC_PROP_SET_TARGET_BIT(teleport, o.mTeleport)
    SYNC_PROP_SET_TARGET_BIT(return, o.mReturn)
    SYNC_PROP_SET_TARGET_BIT(self_shadow, o.mSelfShadow)
    SYNC_PROP(env_override, o.mEnvOverride)
    SYNC_PROP_SET(target_ptr, gHamCamShot->FindTarget(o.mTarget), )
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(HamCamShot)
    gHamCamShot = this;
    SYNC_PROP(targets, mTargets)
    SYNC_PROP_SET(
        player_flag, (int &)mPlayerFlag, mPlayerFlag = (HamPlayerFlags)_val.Int()
    )
    SYNC_PROP(zero_time, mZeroTime)
    SYNC_PROP(min_time, mMinTime)
    SYNC_PROP(max_time, mMaxTime)
    SYNC_PROP_MODIFY(next_shots, mNextShots, CheckNextShots(); ResetNextShot();)
    SYNC_PROP(master_anims, mMasterAnims)
    SYNC_SUPERCLASS(CamShot)
END_PROPSYNCS

BinStream &operator<<(BinStream &, const HamCamShot::Target &);

BEGIN_SAVES(HamCamShot)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(CamShot)
    bs << mTargets;
    bs << mZeroTime;
    bs << mMinTime;
    bs << mMaxTime;
    bs << mNextShots;
    bs << mPlayerFlag;
    bs << mMasterAnims;
END_SAVES

BEGIN_COPYS(HamCamShot)
    COPY_SUPERCLASS(CamShot)
    CREATE_COPY(HamCamShot)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mTargets)
        COPY_MEMBER(mZeroTime)
        COPY_MEMBER(mMinTime)
        COPY_MEMBER(mMaxTime)
        COPY_MEMBER(mNextShots)
        COPY_MEMBER(mPlayerFlag)
        COPY_MEMBER(mMasterAnims)
        ResetNextShot();
    END_COPYING_MEMBERS
END_COPYS

void HamCamShot::StartAnim() {
    if (mCurrentShot && mCurrentShot != this) {
        mCurrentShot->EndAnim();
    }
    UpdateTargetsFlipped();
    ResetNextShot();
    CamShot::StartAnim();
    StartAnims(mMasterAnims);
    for (ObjList<Target>::iterator it = mTargets.begin(); it != mTargets.end(); ++it) {
        if (!it->mTarget.Null()) {
            std::list<TargetCache>::iterator cache = CreateTargetCache(it->mTarget);
            Character *theChar = dynamic_cast<Character *>(cache->unk4);
            if (theChar) {
                theChar->SetSelfShadow(it->mSelfShadow);
                theChar->SetLodType((LODType)it->mForceLOD);
                static Message msg("play_group", 0, 0, 0, 0, 0);
                msg[0] = theChar;
                msg[1] = it->mAnimGroup;
                msg[2] = it->mFastForward / FramesPerUnit();
                msg[3] = Units();
                msg[4] = it->mForwardEvent;
                HandleType(msg);
                if (it->mEnvOverride) {
                    theChar->SetEnv(it->mEnvOverride);
                }
            }
        }
    }
    Reteleport(Vector3::ZeroVec(), true, gNullStr);
    unk2d8 = GetTotalDuration();
    static Message camshot_changed("camshot_changed");
    TheHamProvider->Export(camshot_changed, true);
    sCache.clear();
}

void HamCamShot::ListAnimChildren(std::list<RndAnimatable *> &children) const {
    CamShot::ListAnimChildren(children);
    for (ObjPtrList<RndAnimatable>::iterator it = mMasterAnims.begin();
         it != mMasterAnims.end();
         ++it) {
        children.push_back(*it);
    }
}

bool HamCamShot::TargetTeleportTransform(Symbol s, Transform &xfm) {
    for (ObjList<Target>::iterator it = mTargets.begin(); it != mTargets.end(); ++it) {
        Target &cur = *it;
        if (cur.mTeleport && s == cur.mTarget) {
            xfm = cur.mTo;
            return true;
        }
    }
    return false;
}

bool HamCamShot::IterateNextShot() {
    MILO_ASSERT(!mNextShots.empty(), 0x166);
    if (unk2b4 == 0) {
        unk2b4 = mNextShots.begin();
        return true;
    } else if (unk2b4 != mNextShots.end())
        return true;
    else
        return false;
}

void HamCamShot::Target::Store(HamCamShot *shot) {
    if (!mTarget.Null()) {
        std::list<TargetCache>::iterator it = shot->CreateTargetCache(mTarget);
        if (it->unk4) {
            mTo = it->unk4->LocalXfm();
        }
        HamCamShot::sCache.erase(it);
    }
}

void HamCamShot::Target::UpdateTarget(Symbol s, HamCamShot *shot) {
    if (mTarget != s) {
        mTarget = s;
        mAnimGroup = "";
    }
    Store(shot);
}

std::list<HamCamShot::TargetCache>::iterator HamCamShot::CreateTargetCache(Symbol s) {
    TargetCache cache;
    sCache.push_back(cache);
    cache.unksym = s;
    cache.unk4 = FindTarget(s);
    return sCache.begin();
}

void HamCamShot::Store() {
    for (ObjList<Target>::iterator it = mTargets.begin(); it != mTargets.end(); ++it) {
        it->Store(this);
    }
}

DataNode HamCamShot::AddTarget(DataArray *target) {
    MILO_ASSERT(target->Size() != 2, 0x213);
    mTargets.push_back(Target(this));
    mTargets.back().mTarget = target->Sym(2);
    mTargets.back().Store(this);
    return 0;
}

DataNode HamCamShot::OnTestDelta(DataArray *a) {
    float f = a->Float(2);
    return (mMinTime == 0 || f >= mMinTime) && (mMaxTime == 0 || f <= mMaxTime);
}

DataNode HamCamShot::OnListTargets(const DataArray *a) {
    static Message msg("list_targets");
    DataNode handled = HandleType(msg);
    if (handled.Type() != kDataUnhandled) {
        return handled.Array();
    } else {
        return ObjectList(Dir(), "Trans", true);
    }
}

DataNode HamCamShot::OnListAllNextShots(const DataArray *a) {
    std::list<HamCamShot *> shots;
    ListNextShots(shots);
    DataArrayPtr ptr;
    for (std::list<HamCamShot *>::iterator it = shots.begin(); it != shots.end(); ++it) {
        ptr->Insert(ptr->Size(), *it);
    }
    return ptr;
}

RndTransformable *HamCamShot::FindTarget(Symbol target) {
    static Message msg("find_target", 0);
    msg[0] = target;
    DataNode handled = HandleType(msg);
    if (handled.Type() != kDataUnhandled) {
        return handled.Obj<RndTransformable>();
    } else {
        return Dir()->Find<RndTransformable>(target.Str(), false);
    }
}

void HamCamShot::TeleportTarget(RndTransformable *trans, const Transform &xfm, bool b3) {
    trans->SetLocalXfm(xfm);
    Character *theChar = dynamic_cast<Character *>(trans);
    if (theChar) {
        static Message msg("teleport_char", 0, 0);
        msg[0] = trans;
        msg[1] = b3;
        HandleType(msg);
    }
}

void HamCamShot::ResetNextShot() {
    unk2b4 = 0;
    mCurrentShot = this;
    unk2cc = 0;
    unk2d0 = 0;
}

bool HamCamShot::ListNextShots(std::list<HamCamShot *> &shots) {
    if (unk2dc) {
        MILO_NOTIFY("%s infinite camera shot loop detected!", PathName(this));
        return false;
    } else {
        unk2dc = true;
        for (ObjPtrList<HamCamShot>::iterator it = mNextShots.begin();
             it != mNextShots.end();
             it) {
            shots.push_back(*it);
            if (!(*it)->ListNextShots(shots)) {
                mNextShots.erase(it++);
            } else {
                ++it;
            }
        }
        unk2dc = false;
        return true;
    }
}

int HamCamShot::GetNumShots() {
    std::list<HamCamShot *> shots;
    ListNextShots(shots);
    return shots.size() + 1;
}

float HamCamShot::GetTotalDurationSeconds() {
    float dur = GetDurationSeconds();
    std::list<HamCamShot *> shots;
    ListNextShots(shots);
    for (std::list<HamCamShot *>::iterator it = shots.begin(); it != shots.end(); ++it) {
        dur += (*it)->GetDurationSeconds();
    }
    return dur;
}

void HamCamShot::CheckNextShots() {
    std::list<HamCamShot *> shots;
    ListNextShots(shots);
    if (TheLoadMgr.EditMode()) {
        unk2f4 = mNextShots.size();
    }
}

float HamCamShot::EndFrame() { return GetTotalDuration(); }

void HamCamShot::SetFrameEx(float frame, float blend) {
    unk2d4 = true;
    SetFrame(frame, blend);
    unk2d4 = false;
}
