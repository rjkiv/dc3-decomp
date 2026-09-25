#include "hamobj/HamCamShot.h"
#include "HamCamShot.h"
#include "char/Character.h"
#include "flow/PropertyEventProvider.h"
#include "hamobj/HamCharacter.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "math/Mtx.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/DataUtl.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/NetCacheMgr.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "world/CameraShot.h"
#include "world/Dir.h"
#include <climits>
#include <cstring>
#include <list>
#include <vector>
#include <float.h>

HamCamShot *gHamCamShot;
std::list<HamCamShot::TargetCache> HamCamShot::sCache;

HamCharacter *CharacterNameToCharacter(Symbol name) {
    static Symbol player0("player0");
    static Symbol player1("player1");
    static Symbol backup0("backup0");
    static Symbol backup1("backup1");

    if (name == player0) {
        return TheHamDirector->GetCharacter(0);
    } else if (name == player1) {
        return TheHamDirector->GetCharacter(1);
    }

    if (name == backup0) {
        return TheHamDirector->GetBackup(0);
    } else if (name == backup1) {
        return TheHamDirector->GetBackup(1);
    }

    return nullptr;
}

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

BinStream &operator<<(BinStream &bs, const HamCamShot::Target &target) {
    bs << target.mTarget;
    bs << target.mTeleport;
    bs << target.mTo;
    bs << target.mAnimGroup;
    bs << target.mReturn;
    bs << target.mFastForward;
    bs << target.mForwardEvent;
    bs << target.mSelfShadow;
    bs << target.unk68p4;
    bs << target.unk68p3;
    bs << target.mEnvOverride;
    unsigned char lod = target.mForceLOD;
    bs << lod;
    return bs;
}

BinStream &operator>>(BinStream &bs, HamCamShot::Target &target) {
    bs >> target.mTarget;
    char teleport;
    bs >> teleport;
    target.mTeleport = teleport;
    bs >> target.mTo;
    bs >> target.mAnimGroup;
    char ret;
    bs >> ret;
    target.mReturn = ret;
    bs >> target.mFastForward;
    bs >> target.mForwardEvent;
    char selfshadow;
    bs >> selfshadow;
    target.mSelfShadow = selfshadow;
    char p4;
    bs >> p4;
    target.unk68p4 = p4;
    char p3;
    bs >> p3;
    target.unk68p3 = p3;
    bs >> target.mEnvOverride;
    char lod;
    bs >> lod;
    target.mForceLOD = lod;
    return bs;
}

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
    d >> mNextShots;
    unk2f4 = mNextShots.size();
    int x;
    if (d.rev > 1) {
        int x;
        d >> x;
        mPlayerFlag = (HamPlayerFlags)x;
    }
    if (d.rev > 2) {
        d >> mMasterAnims;
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
    FOREACH (it, mMasterAnims) {
        children.push_back(*it);
    }
}

bool HamCamShot::TargetTeleportTransform(Symbol s, Transform &xfm) {
    FOREACH (it, mTargets) {
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
    sCache.push_front(TargetCache());
    sCache.front().unksym = s;
    sCache.front().unk4 = FindTarget(s);
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
    FOREACH (it, mTargets) {
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
    FOREACH (it, shots) {
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
    FOREACH (it, shots) {
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
    auto it = shot->Refs().Begin();
    while (it != shot->Refs().End()) {
        HamCamShot *cur = dynamic_cast<HamCamShot *>((*it).RefOwner());
        if (cur) {
            FOREACH (it2, cur->mNextShots) {
                if ((*it2) == shot) {
                    shot = cur;
                    MILO_ASSERT(cur != this, 0x268);
                    it = shot->Refs().Begin();
                    break;
                }
            }
        } else {
            it = shot->Refs().Next(it);
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

void HamCamShot::SetFrame(float frame, float blend) {
    if (!unk2dd) {
        SetPreFrame(frame, blend);
    }
    float f = frame;
    bool b = frame < mDuration || mNextShots.empty();
    if (!b) {
        frame -= unk2cc + mDuration;
    }

    if (CheckShotOver(f)) {
        SetShotOver();
    }
    if (this == mCurrentShot) {
        CamShot::SetFrame(frame, blend);
    } else {
        FOREACH (it, mAnims) {
            RndAnimatable *anim = *it;
            anim->SetFrame(frame, 1.0f);
        }
        mCurrentShot->SetFrameEx(frame, blend);
        RndAnimatable::SetFrame(f, blend);
    }
    SetFrames(mMasterAnims, f);
    unk2dd = false;
}

void HamCamShot::SetPreFrame(float frame, float blend) {
    unk2dd = true;
    bool b = frame < mDuration || mNextShots.empty();
    if (b) {
        if (mCurrentShot != this) {
            ResetNextShot();
        }
    } else {
        frame -= mDuration;
        while (frame < unk2cc && unk2b4 != mNextShots.begin()) {
            --unk2b4;
            unk2d0 = (*unk2b4)->GetTotalDuration();
            unk2cc -= unk2d0;
            mCurrentShot = *unk2b4;
        }
        frame -= unk2cc;
        while (frame >= unk2d0) {
            if (IterateNextShot()) {
                frame -= unk2d0;
                unk2cc += unk2d0;
                if (mCurrentShot) {
                    mCurrentShot->EndAnim();
                }
                mCurrentShot = *unk2b4;
                mCurrentShot->StartAnim();
                unk2d0 = (*unk2b4)->GetTotalDuration();
            } else {
                unk2d0 = FLT_MAX;
            }
        }
    }
    if (mCurrentShot != this) {
        mCurrentShot->SetPreFrame(frame, 1.0f);
    }
}

void HamCamShot::Reteleport(const Vector3 &v3, bool b2, Symbol s3) {
    bool unkb = unk388;
    FOREACH (it, mTargets) {
        Target &t11 = *it;
        Target &t6 = unkb ? *GetFlipTarget(&t11) : t11;
        if (!t11.mTarget.Null() && (!b2 || t11.mTeleport)
            && (s3 == gNullStr || s3 == t11.mTarget)) {
            auto cacheItr = CreateTargetCache(t11.mTarget);
            if (cacheItr->unk4) {
                cacheItr->unkxfm = cacheItr->unk4->LocalXfm();
                Transform tfe0 = t6.mTo;
                static Symbol player0("player0");
                static Symbol player1("player1");
                static Symbol backup0("backup0");
                static Symbol backup1("backup1");
                static Symbol DC_PLAYER_FREESTYLE("DC_PLAYER_FREESTYLE");
                static Symbol INTRO_QUICK("INTRO_QUICK");
                static Symbol INTRO_PLAYLIST("INTRO_PLAYLIST");
                if (TheGameData->SidesSwapped() && (mPlayerFlag == 2 || mPlayerFlag == 3)
                    && (t11.mTarget == player0 || t11.mTarget == player1
                        || t11.mTarget == backup0 || t11.mTarget == backup1)) {
                    static Symbol AUTHORED_CAM_CATS("AUTHORED_CAM_CATS");
                    DataArray *macro = DataGetMacro(AUTHORED_CAM_CATS);
                    if ((macro && macro->Contains(mCategory))
                        || mCategory == DC_PLAYER_FREESTYLE || mCategory == INTRO_QUICK
                        || mCategory == INTRO_PLAYLIST) {
                        Symbol s10;
                        if (t6.mTarget == player0) {
                            s10 = player1;
                        } else if (t6.mTarget == player1) {
                            s10 = player0;
                        } else if (t6.mTarget == backup0) {
                            s10 = backup1;
                        } else if (t6.mTarget == backup1) {
                            s10 = backup0;
                        }
                        FOREACH (t, mTargets) {
                            if (t->mTarget == s10) {
                                tfe0 = t->mTo;
                                break;
                            }
                        }
                    }
                }

                Transform tf120;
                Multiply(tfe0, WorldXfm(), tf120);
                tf120.v += v3;
                TeleportTarget(cacheItr->unk4, tf120, false);
            }
        }
    }
    sCache.clear();
}

void HamCamShot::FlipTargetAnimGroups() {
    static Symbol player0("player0");
    static Symbol player1("player1");
    auto it = mTargets.begin();
    for (; it != mTargets.end() && it->mTarget != player0; ++it)
        ;
    auto it2 = mTargets.begin();
    for (; it2 != mTargets.end() && it2->mTarget != player1; ++it2)
        ;
    if (it != mTargets.end()) {
        it->mTarget = player1;
    }
    if (it2 != mTargets.end()) {
        it2->mTarget = player0;
    }
}

void HamCamShot::UpdateTargetsFlipped() {
    static Symbol gameplay_mode("gameplay_mode");
    static Symbol dance_battle("dance_battle");
    bool targetsFlipped = AreTargetsFlipped();
    bool isDanceBattle = (TheHamProvider->Property(gameplay_mode)->Sym() == dance_battle);

    if (TheHamDirector) {
        TheHamDirector->SetPhraseMetersFlipped(targetsFlipped);
    }

    WorldDir *world = TheHamDirector ? TheHamDirector->GetVenueWorld() : nullptr;

    if (world) {
        static Symbol game_stage("game_stage");
        static Symbol intro("intro");
        static Symbol crew_battle_intro("crew_battle_intro");
        static Symbol crewbattle_intro("crewbattle_intro");
        static Symbol BattleIntro("BattleIntro");

        if (isDanceBattle && targetsFlipped) {
            if (TheHamProvider->Property(game_stage)->Sym() == intro) {
                MILO_LOG("Camshot %s\n", Name());
                int targetIdx = 0;
                FOREACH (it, mTargets) {
                    Target &t = *it;
                    HamCharacter *character = CharacterNameToCharacter(t.mTarget);

                    ObjectDir *clips =
                        character ? character->Find<ObjectDir>("clips") : nullptr;

                    if (((mPlayerFlag == kHamPlayer1 && targetIdx % 2 == 0)
                         || (mPlayerFlag == kHamPlayer0 && targetIdx % 2 == 1))
                        && character) {
                        if (clips) {
                            Hmx::Object *findIntro =
                                clips->Find<Hmx::Object>("crewbattle_intro", false);

                            if (findIntro) {
                                t.mAnimGroup = crewbattle_intro;
                            } else {
                                findIntro =
                                    clips->Find<Hmx::Object>("BattleIntro", false);
                                if (findIntro) {
                                    t.mAnimGroup = BattleIntro;
                                } else {
                                    findIntro = clips->Find<Hmx::Object>(
                                        "crew_battle_intro", false
                                    );
                                    if (findIntro) {
                                        t.mAnimGroup = crew_battle_intro;
                                    }
                                }
                            }
                        }
                    }

                    MILO_LOG(
                        "   Target %d: character = \'%s\' clips = \'%s\' animGroup = \'%s\'\n",
                        targetIdx,
                        character ? character->Name() : "NULL",
                        clips ? clips->ProxyFile().c_str() : "NULL",
                        t.mAnimGroup
                    );
                    targetIdx++;
                }
            }
        }
    }

    if (targetsFlipped != unk388) {
        unk388 = targetsFlipped;
        CreateFlippedShowHideList();

        WorldDir *worldDir = TheHamDirector ? TheHamDirector->GetVenueWorld() : nullptr;

        if (worldDir) {
            FOREACH (it, mKeyframes) {
                CamShotFrame &frame = *it;
                std::vector<RndTransformable *> targets;
                FOREACH (it2, frame.mTargets) {
                    RndTransformable *target = *it2;
                    const char *name = target->Name();
                    char buf[256];
                    strcpy(buf, name);

                    RndTransformable *transform = target;
                    if (targetsFlipped) {
                        if (strstr(name, "player0") && mPlayerFlag == kHamPlayer0) {
                            buf[6] = '1';
                            transform = worldDir->Find<RndTransformable>(buf);
                        } else if (strstr(name, "player1") && mPlayerFlag == kHamPlayer1) {
                            buf[6] = '0';
                            transform = worldDir->Find<RndTransformable>(buf);
                        }
                    } else {
                        if (strstr(name, "player0") && mPlayerFlag == kHamPlayer1) {
                            buf[6] = '1';
                            transform = worldDir->Find<RndTransformable>(buf);
                        } else if (strstr(name, "player1") && mPlayerFlag == kHamPlayer0) {
                            buf[6] = '0';
                            transform = worldDir->Find<RndTransformable>(buf);
                        }
                    }
                    targets.push_back(transform);
                }
                frame.mTargets.clear();
                FOREACH (it2, targets) {
                    frame.mTargets.push_back(*it2);
                }
            }
        }

        if (targetsFlipped) {
            mShowList = unk354;
            mHideList = unk340;
            mGenHideList = unk368;
            mGenHideVector = unk37c;
        } else {
            mShowList = unk30c;
            mHideList = unk2f8;
            mGenHideList = unk320;
            mGenHideVector = unk334;
        }
    }
}

DataNode HamCamShot::OnAllowableNextShots(const DataArray *a) {
    DataArrayPtr ptr;
    for (ObjDirItr<HamCamShot> it(Dir(), true); it != nullptr; ++it) {
        if (this != it) {
            if (mNextShots.find(it) == nullptr) {
                std::list<HamCamShot *> camshots;
                it->ListNextShots(camshots);
                if (std::find(camshots.begin(), camshots.end(), this) == camshots.end()) {
                    ptr->Insert(ptr->Size(), &*it);
                }
            }
        }
    }
    static DataNode &propNode = DataVariable("milo_prop_path");
    if (propNode.Type() == kDataArray && propNode.Array()->Size() == 2) {
        auto it = NextItr(mNextShots.begin(), propNode.Array()->Int(1));
        ptr->Insert(ptr->Size(), *it);
    }
    return ptr;
}

HamCamShot::Target *HamCamShot::GetFlipTarget(Target *target) {
    Symbol temp = target->mTarget;
    Symbol flipTarget = GetFlipTarget(temp);
    if (temp != flipTarget) {
        FOREACH (it, mTargets) {
            if (it->mTarget == flipTarget) {
                return &*it;
            }
        }
    }
    return target;
}
