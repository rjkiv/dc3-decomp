#include "hamobj/HamCamShot.h"
#include "HamCamShot.h"
#include "char/Character.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/HamDirector.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "rndobj/Draw.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "world/CameraShot.h"
#include "world/Dir.h"
#include <list>

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
BinStream &operator>>(BinStream &, const HamCamShot::Target &);

INIT_REVS(3, 0)

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

BEGIN_LOADS(HamCamShot)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(CamShot)
    d >> mTargets;
    d >> mZeroTime;
    d >> mMinTime;
    d >> mMaxTime;
    bs >> mNextShots;
    unk2f4 = mNextShots.size();
    int x;
    if (d.rev > 1) {
        bs.ReadEndian(&x, 4);
        mPlayerFlag = (HamPlayerFlags)x;
    }
    if (d.rev > 2) {
        bs >> mMasterAnims;
    }

    ResetNextShot();
END_LOADS

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
    FOREACH (it, mTargets) {
        Target &target = *it;
        if (!target.mTarget.Null()) {
            auto cache = CreateTargetCache(target.mTarget);
            Character *theChar = dynamic_cast<Character *>(cache->unk4);
            if (theChar) {
                theChar->SetSelfShadow(target.mSelfShadow);
                theChar->SetLodType((LODType)target.mForceLOD);
                static Message msg("play_group", 0, 0, 0, 0, 0);
                msg[0] = theChar;
                msg[1] = target.mAnimGroup;
                msg[2] = target.mFastForward / FramesPerUnit();
                msg[3] = Units();
                msg[4] = target.mForwardEvent;
                HandleType(msg);
                if (target.mEnvOverride) {
                    cache->unk8 = theChar->GetEnv();
                    theChar->SetEnv(target.mEnvOverride);
                }
            }
        }
    }
    Reteleport(Vector3::GetZero(), true, gNullStr);
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
    bool retval = true;
    MILO_ASSERT(!mNextShots.empty(), 0x166);
    auto it = unk2b4;
    if (it == 0) {
        unk2b4 = mNextShots.begin();
    } else {
        ++unk2b4;
        if (unk2b4 == 0) {
            retval = false;
            unk2b4 = it;
        }
    }
    return retval;
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

std::list<HamCamShot::TargetCache>::iterator HamCamShot::GetTargetCache(Symbol s) {
    FOREACH (it, sCache) {
        if (s == it->unksym) {
            return it;
        }
    }

    if (!TheLoadMgr.EditMode()) {
        MILO_NOTIFY(
            "%s creating target cache for %s, targets changed while playing camera",
            PathName(this),
            s
        );
    }

    return CreateTargetCache(s);
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
        theChar->SetTeleport(true);
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

HamCamShot *HamCamShot::InitialShot() {
    HamCamShot *shot = this;
    auto it = shot->Refs().begin();
    while (it != shot->Refs().end()) {
        HamCamShot *cur = dynamic_cast<HamCamShot *>((*it).RefOwner());
        if (cur) {
            FOREACH (it2, cur->mNextShots) {
                if ((*it2) == shot) {
                    shot = cur;
                    MILO_ASSERT(cur != this, 0x268);
                    it = shot->Refs().begin();
                    break;
                }
            }
        } else {
            it++;
        }
    }
    return shot;
}

bool HamCamShot::AreTargetsFlipped() const {
    static Symbol flip_camshot_targets("flip_camshot_targets");
    auto prop = TheHamProvider->Property(flip_camshot_targets);
    if (prop) {
        return prop->Int() != 0;
    } else {
        return false;
    }
}

Symbol HamCamShot::GetFlipTarget(Symbol s) const {
    static Symbol player0("player0");
    static Symbol player1("player1");
    static Symbol backup0("backup0");
    static Symbol backup1("backup1");
    if (s == player0) {
        return player1;
    } else if (s == player1) {
        return player0;
    } else if (s == backup0) {
        return backup1;
    } else if (s == backup1) {
        return backup0;
    }
    return s;
}

RndDrawable *HamCamShot::GetFlipCharacter(RndDrawable *drawable) {
    static Symbol player0("player0");
    static Symbol player1("player1");
    static Symbol backup0("backup0");
    static Symbol backup1("backup1");

    Symbol name = drawable->Name();
    if (!TheHamDirector) {
        return drawable;
    }

    if (name == player0) {
        return TheHamDirector->GetCharacter(1);
    } else if (name == player1) {
        return TheHamDirector->GetCharacter(0);
    } else if (name == backup0) {
        return TheHamDirector->GetBackup(1);
    } else if (name == backup1) {
        return TheHamDirector->GetBackup(0);
    }
    return drawable;
}

void HamCamShot::EndAnim() {
    if (mCurrentShot && mCurrentShot != this) {
        mCurrentShot->EndAnim();
        ResetNextShot();
    } else {
        FOREACH (it, mTargets) {
            Target &target = *it;
            if (!target.mTarget.Null()) {
                auto cacheIt = GetTargetCache(target.mTarget);
                if (target.mTeleport && target.mReturn && cacheIt->unk4) {
                    TeleportTarget(cacheIt->unk4, cacheIt->unkxfm, true);
                }
                Character *c = dynamic_cast<Character *>(cacheIt->unk4);
                if (c) {
                    c->SetLodType(kLODPerFrame);
                    if (target.mEnvOverride) {
                        c->SetEnv(cacheIt->unk8);
                    }
                }
                sCache.erase(cacheIt);
            }
        }
        EndAnims(mMasterAnims);
        CamShot::EndAnim();
    }
}

float HamCamShot::GetTotalDuration() {
    float f = mDuration;
    std::list<HamCamShot *> shots;
    ListNextShots(shots);
    FOREACH (it, shots) {
        f += (*it)->mDuration;
    }
    return f;
}

void HamCamShot::CreateFlippedShowHideList() {
    if (unk2f8.size() <= 0 && unk30c.size() <= 0 && unk320.size() <= 0
        && unk334.size() <= 0 && unk340.size() <= 0 && unk354.size() <= 0
        && unk368.size() <= 0 && unk37c.size() <= 0) {
        FOREACH (it, mHideList) {
            RndDrawable *drawable = *it;
            unk2f8.push_back(drawable);
            unk340.push_back(GetFlipCharacter(drawable));
        }

        FOREACH (it, mShowList) {
            RndDrawable *drawable = *it;
            unk30c.push_back(drawable);
            unk354.push_back(GetFlipCharacter(drawable));
        }

        FOREACH (it, mGenHideList) {
            RndDrawable *drawable = *it;
            unk320.push_back(drawable);
            unk368.push_back(GetFlipCharacter(drawable));
        }

        FOREACH (it, mGenHideVector) {
            RndDrawable *drawable = *it;
            unk334.push_back(drawable);
            unk37c.push_back(GetFlipCharacter(drawable));
        }
    }
}
