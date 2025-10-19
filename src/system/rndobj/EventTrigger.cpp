#include "rndobj/EventTrigger.h"
#include "math/Rand.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "obj/Msg.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/Anim.h"
#include "rndobj/PartLauncher.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"

DataArray *gSupportedEvents;

EventTrigger::EventTrigger()
    : mAnims(this), mSpawnedTasks(this), mProxyCalls(this), mSounds(this), mShows(this),
      mResetTriggers(this), mHideDelays(this), mNextLink(this), mPartLaunchers(this),
      unkd0(false), mAnimTrigger(kTriggerAnimNone), mAnimFrame(0), mEnabled(true),
      mEnabledAtStart(true), mWaiting(false), mHidden(this), mShown(this),
      mTriggered(false), mTriggerOrder(kTriggerRandom), mLastTriggerIndex(-1) {
    RegisterEvents();
}

bool EventTrigger::Replace(ObjRef *from, Hmx::Object *to) {
    FOREACH (it, mAnims) {
        if (from == &it->mAnim) {
            if (!it->mAnim.SetObj(to)) {
                mAnims.erase(it);
            }
            return true;
        }
    }
    FOREACH (it, mHideDelays) {
        if (from == &it->mHide) {
            if (!it->mHide.SetObj(to)) {
                mHideDelays.erase(it);
            }
            return true;
        }
    }
    FOREACH (it, mProxyCalls) {
        if ((from == &it->mEvent && !it->mEvent.SetObj(to))
            || (from == &it->mProxy && !it->mProxy.SetObj(to))) {
            mProxyCalls.erase(it);
            return true;
        }
    }
    return Hmx::Object::Replace(from, to);
}

BEGIN_HANDLERS(EventTrigger)
    HANDLE(trigger, OnTrigger)
    HANDLE_ACTION(enable, mEnabled = true)
    HANDLE_ACTION(disable, mEnabled = false)
    HANDLE_ACTION_IF(wait_for, mWaiting, Trigger())
    HANDLE(proxy_calls, OnProxyCalls)
    HANDLE_EXPR(supported_events, SupportedEvents())
    HANDLE_ACTION(basic_cleanup, BasicReset())
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void ResetAnim(EventTrigger::Anim &anim) {
    if (anim.mAnim) {
        anim.mRate = anim.mAnim->GetRate();
        anim.mStart = anim.mAnim->StartFrame();
        anim.mEnd = anim.mAnim->EndFrame();
        anim.mPeriod = 0.0f;
        anim.mScale = 1.0f;
        static Symbol range("range");
        static Symbol loop("loop");
        anim.mType = anim.mAnim->Loop() ? loop : range;
    }
}

BEGIN_CUSTOM_PROPSYNC(EventTrigger::Anim)
    SYNC_PROP_MODIFY(anim, o.mAnim, ResetAnim(o))
    SYNC_PROP(blend, o.mBlend)
    SYNC_PROP(wait, o.mWait)
    SYNC_PROP(delay, o.mDelay)
    SYNC_PROP(enable, o.mEnable)
    SYNC_PROP(rate, (int &)o.mRate)
    SYNC_PROP(start, o.mStart)
    SYNC_PROP(end, o.mEnd)
    SYNC_PROP(scale, o.mScale)
    SYNC_PROP(period, o.mPeriod)
    SYNC_PROP(type, o.mType)
END_CUSTOM_PROPSYNC

BEGIN_CUSTOM_PROPSYNC(EventTrigger::ProxyCall)
    SYNC_PROP_MODIFY(proxy, o.mProxy, o.mCall = "")
    SYNC_PROP(call, o.mCall)
    SYNC_PROP(event, o.mEvent)
END_CUSTOM_PROPSYNC

BEGIN_CUSTOM_PROPSYNC(EventTrigger::HideDelay)
    SYNC_PROP(hide, o.mHide)
    SYNC_PROP(delay, o.mDelay)
    SYNC_PROP(rate, o.mRate)
END_CUSTOM_PROPSYNC

#define SYNC_PROP_TRIGGER(s, member)                                                     \
    {                                                                                    \
        _NEW_STATIC_SYMBOL(s)                                                            \
        if (sym == _s) {                                                                 \
            if (!(_op & (kPropSize | kPropGet)))                                         \
                UnregisterEvents();                                                      \
            if (PropSync(member, _val, _prop, _i + 1, _op)) {                            \
                if (!(_op & (kPropSize | kPropGet))) {                                   \
                    RegisterEvents();                                                    \
                }                                                                        \
                return true;                                                             \
            } else {                                                                     \
                return false;                                                            \
            }                                                                            \
        }                                                                                \
    }

BEGIN_PROPSYNCS(EventTrigger)
    SYNC_PROP_TRIGGER(trigger_events, mTriggerEvents)
    SYNC_PROP_MODIFY(anims, mAnims, CheckAnims())
    SYNC_PROP(proxy_calls, mProxyCalls)
    SYNC_PROP(sounds, mSounds)
    SYNC_PROP(shows, mShows)
    SYNC_PROP(hide_delays, mHideDelays)
    SYNC_PROP(part_launchers, mPartLaunchers)
    SYNC_PROP_TRIGGER(enable_events, mEnableEvents)
    SYNC_PROP_TRIGGER(disable_events, mDisableEvents)
    SYNC_PROP(enabled, mEnabled)
    SYNC_PROP_MODIFY(enabled_at_start, mEnabledAtStart, mEnabled = mEnabledAtStart)
    SYNC_PROP_TRIGGER(wait_for_events, mWaitForEvents)
    SYNC_PROP_SET(next_link, mNextLink.Ptr(), SetNextLink(_val.Obj<EventTrigger>()))
    SYNC_PROP(trigger_order, (int &)mTriggerOrder)
    SYNC_PROP(triggers_to_reset, mResetTriggers)
    SYNC_PROP(anim_trigger, (int &)mAnimTrigger)
    SYNC_PROP(anim_frame, mAnimFrame)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const EventTrigger::HideDelay &e) {
    bs << e.mHide << e.mDelay << e.mRate;
    return bs;
}

BinStream &operator<<(BinStream &bs, const EventTrigger::Anim &e) {
    bs << e.mAnim << e.mBlend << e.mWait << e.mDelay;
    bs << e.mEnable;
    bs << e.mRate << e.mStart;
    bs << e.mEnd << e.mPeriod;
    bs << e.mType;
    bs << e.mScale;
    return bs;
}

BinStream &operator<<(BinStream &bs, const EventTrigger::ProxyCall &e) {
    bs << e.mProxy;
    bs << e.mCall;
    bs << e.mEvent;
    return bs;
}

BEGIN_SAVES(EventTrigger)
    SAVE_REVS(0x11, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndAnimatable)
    bs << mTriggerEvents << mAnims << mSounds << mShows << mHideDelays;
    bs << mEnableEvents << mDisableEvents << mWaitForEvents;
    bs << mNextLink << mProxyCalls << mTriggerOrder << mResetTriggers << mEnabledAtStart
       << mAnimTrigger << mAnimFrame;
    bs << mPartLaunchers;
END_SAVES

BEGIN_COPYS(EventTrigger)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    CREATE_COPY(EventTrigger)
    BEGIN_COPYING_MEMBERS
        UnregisterEvents();
        COPY_MEMBER(mTriggerEvents)
        COPY_MEMBER(mAnims)
        COPY_MEMBER(mSounds)
        COPY_MEMBER(mProxyCalls)
        COPY_MEMBER(mShows)
        COPY_MEMBER(mHideDelays)
        COPY_MEMBER(mEnableEvents)
        COPY_MEMBER(mDisableEvents)
        COPY_MEMBER(mWaitForEvents)
        COPY_MEMBER(mNextLink)
        COPY_MEMBER(mTriggerOrder)
        COPY_MEMBER(mResetTriggers)
        COPY_MEMBER(mEnabledAtStart)
        COPY_MEMBER(mAnimTrigger)
        COPY_MEMBER(mAnimFrame)
        COPY_MEMBER(mPartLaunchers)
        RegisterEvents();
        CleanupHideShow();
    END_COPYING_MEMBERS
END_COPYS

BinStream &operator>>(BinStream &bs, EventTrigger::HideDelay &hd) {
    bs >> hd.mHide >> hd.mDelay >> hd.mRate;
    return bs;
}

BinStreamRev &operator>>(BinStreamRev &d, EventTrigger::Anim &anim) {
    d.stream >> anim.mAnim >> anim.mBlend >> anim.mWait >> anim.mDelay;
    if (d.rev > 9) {
        d >> anim.mEnable >> (int &)anim.mRate >> anim.mStart;
        d >> anim.mEnd >> anim.mPeriod >> anim.mType >> anim.mScale;
    } else {
        ResetAnim(anim);
    }
    return d;
}

BinStreamRev &operator>>(BinStreamRev &d, EventTrigger::ProxyCall &pc) {
    d >> pc.mProxy;
    d >> pc.mCall;
    if (d.rev > 10) {
        pc.mEvent.Load(d.stream, true, pc.mProxy);
    }
    return d;
}

void RemoveNullEvents(std::list<Symbol> &vec) {
    std::list<Symbol>::iterator it = vec.begin();
    while (it != vec.end()) {
        if (it->Null())
            it = vec.erase(it);
        else
            it++;
    }
}

BEGIN_LOADS(EventTrigger)
    LOAD_REVS(bs)
    ASSERT_REVS(0x11, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    if (d.rev > 0xF) {
        LOAD_SUPERCLASS(RndAnimatable)
    }
    UnregisterEvents();
    std::list<EventTrigger *> triggers;
    if (d.rev > 9)
        d >> mTriggerEvents;
    else if (d.rev > 6) {
        mTriggerEvents.clear();
        Symbol sym;
        d >> sym;
        if (!sym.Null())
            mTriggerEvents.push_back(sym);
    }
    if (d.rev > 6) {
        d >> mAnims >> mSounds >> mShows;
    }

    if (d.rev > 0xC)
        d >> mHideDelays;
    else if (d.rev > 8) {
        mHideDelays.clear();
        int count;
        d >> count;
        mHideDelays.resize(count);
        for (ObjList<HideDelay>::iterator it = mHideDelays.begin();
             it != mHideDelays.end();
             ++it) {
            d >> it->mHide >> it->mDelay;
        }
    } else if (d.rev > 6) {
        ObjPtrList<RndDrawable> drawList(this);
        d >> drawList;
        mHideDelays.clear();
        for (auto it = drawList.begin(); it != drawList.end(); ++it) {
            mHideDelays.push_back();
            mHideDelays.back().mHide = *it;
        }
    } else {
        ObjPtr<Hmx::Object> objPtr(this);
        d >> objPtr;
        unsigned int count;
        d >> count;
        EventTrigger *curTrig = this;
        String str(FileGetBase(Name()));
        bool oldMode = TheLoadMgr.EditMode();
        TheLoadMgr.SetEditMode(true);
        while (count-- != 0) {
            curTrig->LoadOldEvent(
                d, objPtr, count != 0 || curTrig != this ? str.c_str() : nullptr, Dir()
            );
            if (count != 0) {
                curTrig = new EventTrigger();
                triggers.push_back(curTrig);
            }
        }
        TheLoadMgr.SetEditMode(oldMode);
    }
    if (d.rev > 2) {
        d >> mEnableEvents;
        d >> mDisableEvents;
    }
    if (d.rev > 5)
        d >> mWaitForEvents;
    if (d.rev > 6)
        d >> mNextLink;
    if (d.rev < 10) {
        RemoveNullEvents(mEnableEvents);
        RemoveNullEvents(mDisableEvents);
        RemoveNullEvents(mWaitForEvents);
    }
    if (d.rev < 7) {
        std::list<EventTrigger *>::iterator it;
        for (it = triggers.begin(); it != triggers.end(); ++it) {
            (*it)->mEnableEvents = mEnableEvents;
            (*it)->mDisableEvents = mDisableEvents;
            (*it)->mWaitForEvents = mWaitForEvents;
        }
    }
    if (d.rev > 7)
        d >> mProxyCalls;
    if (d.rev > 0xB) {
        int i = 0;
        d >> i;
        mTriggerOrder = (TriggerOrder)i;
    }
    if (d.rev > 0xD)
        d >> mResetTriggers;
    if (d.rev > 0xE)
        d >> mEnabledAtStart;
    if (d.rev > 0xF) {
        d >> (int &)mAnimTrigger >> mAnimFrame;
    }
    if (d.rev > 0x10)
        d >> mPartLaunchers;
    CleanupEventCase(mTriggerEvents);
    CleanupEventCase(mEnableEvents);
    CleanupEventCase(mDisableEvents);
    CleanupEventCase(mWaitForEvents);
    RegisterEvents();
    CleanupHideShow();
    ConvertParticleTriggerType();
END_LOADS

void EventTrigger::SetName(const char *cc, class ObjectDir *dir) {
    UnregisterEvents();
    Hmx::Object::SetName(cc, dir);
    RegisterEvents();
}

void EventTrigger::StartAnim() {
    mFrame = kHugeFloat;
    if (mAnimTrigger == kTriggerAnimStart) {
        Trigger();
    }
}

void EventTrigger::EndAnim() {
    mFrame = kHugeFloat;
    if (mAnimTrigger == kTriggerAnimEnd) {
        Trigger();
    } else if (mAnimTrigger == kTriggerAnimStart) {
        BasicReset();
    }
}

void EventTrigger::SetFrame(float frame, float blend) {
    float oldframe = mFrame;
    RndAnimatable::SetFrame(frame, blend);
    if (mAnimTrigger == kTriggerAnimFrame && oldframe < mAnimFrame
        && mFrame >= mAnimFrame) {
        Trigger();
    }
}

void EventTrigger::Trigger() {
    mWaiting = false;
    if (!mNextLink) {
        TriggerSelf();
    } else {
        EventTrigger *it = this;
        int num = 1;
        while ((it = it->mNextLink) != nullptr) {
            num++;
        }
        switch (mTriggerOrder) {
        case kTriggerRandom:
            num = RandomInt(0, num);
            break;
        case kTriggerSequence:
            num = (mLastTriggerIndex + 1) % num;
            break;
        default:
            MILO_NOTIFY("Unknown trigger order for %s", Name());
            break;
        }
        mLastTriggerIndex = num;
        it = this;
        while (num-- != 0) {
            it = it->mNextLink;
        }
        it->TriggerSelf();
    }
}

void EventTrigger::BasicReset() {
    mSpawnedTasks.DeleteAll();
    for (ObjPtrList<RndDrawable>::iterator it = mShown.begin(); it != mShown.end();
         ++it) {
        (*it)->SetShowing(false);
    }
    for (ObjPtrList<RndDrawable>::iterator it = mHidden.begin(); it != mHidden.end();
         ++it) {
        (*it)->SetShowing(true);
    }
    CleanupHideShow();
    for (ObjList<ProxyCall>::iterator it = mProxyCalls.begin(); it != mProxyCalls.end();
         ++it) {
        if (it->mProxy && it->mEvent) {
            it->mEvent->BasicReset();
        }
    }
    for (ObjPtrList<Sequence>::iterator it = mSounds.begin(); it != mSounds.end(); ++it) {
        (*it)->Stop(false);
    }
    if (TypeDef()) {
        static Message reset("reset");
        Hmx::Object::Handle(reset, false);
    }
    if (mNextLink) {
        mNextLink->BasicReset();
    }
}

DataArray *EventTrigger::SupportedEvents() {
    DataArray *cfg;
    if (Type() == "endgame_action") {
        cfg = SystemConfig(
            "objects", "EventTrigger", "types", "endgame_action", "supported_events"
        );
    } else {
        cfg = SystemConfig("objects", "EventTrigger", "supported_events");
    }
    gSupportedEvents = cfg->Array(1);
    return gSupportedEvents;
}

void EventTrigger::RegisterEvents() {
    Hmx::Object *src = Dir();
    if (src) {
        static Symbol trigger("trigger");
        for (std::list<Symbol>::iterator it = mTriggerEvents.begin();
             it != mTriggerEvents.end();
             ++it) {
            src->AddSink(this, *it, trigger);
        }
        static Symbol enable("enable");
        for (std::list<Symbol>::iterator it = mEnableEvents.begin();
             it != mEnableEvents.end();
             ++it) {
            src->AddSink(this, *it, enable);
        }
        static Symbol disable("disable");
        for (std::list<Symbol>::iterator it = mDisableEvents.begin();
             it != mDisableEvents.end();
             ++it) {
            src->AddSink(this, *it, disable);
        }
        static Symbol wait_for("wait_for");
        for (std::list<Symbol>::iterator it = mWaitForEvents.begin();
             it != mWaitForEvents.end();
             ++it) {
            src->AddSink(this, *it, wait_for);
        }
        mEnabled = mEnabledAtStart;
    }
}

void EventTrigger::UnregisterEvents() {
    Hmx::Object *src = Dir();
    if (src) {
        for (std::list<Symbol>::iterator it = mTriggerEvents.begin();
             it != mTriggerEvents.end();
             ++it) {
            src->RemoveSink(this, *it);
        }
        for (std::list<Symbol>::iterator it = mEnableEvents.begin();
             it != mEnableEvents.end();
             ++it) {
            src->RemoveSink(this, *it);
        }
        for (std::list<Symbol>::iterator it = mDisableEvents.begin();
             it != mDisableEvents.end();
             ++it) {
            src->RemoveSink(this, *it);
        }
        for (std::list<Symbol>::iterator it = mWaitForEvents.begin();
             it != mWaitForEvents.end();
             ++it) {
            src->RemoveSink(this, *it);
        }
    }
}

void EventTrigger::CleanupEventCase(std::list<Symbol> &syms) {
    for (std::list<Symbol>::iterator it = syms.begin(); it != syms.end(); ++it) {
        const char *lightStr = strstr(it->Str(), "lighting_");
        if (lightStr) {
            String str(*it);
            str.ToLower();
            *it = str.c_str();
        }
    }
}

DataNode EventTrigger::OnTrigger(DataArray *) {
    if (mEnabled) {
        if (!mWaitForEvents.empty()) {
            mWaiting = true;
        } else
            Trigger();
    }
    return 0;
}

EventTrigger::Anim::Anim(Hmx::Object *o)
    : mAnim(o), mBlend(0), mDelay(0), mWait(0), mEnable(0), mRate(k30_fps), mStart(0),
      mEnd(0), mPeriod(0), mScale(1) {
    static Symbol range("range");
    mType = range;
}

EventTrigger::ProxyCall::ProxyCall(Hmx::Object *o) : mProxy(o), mEvent(o) {}
EventTrigger::HideDelay::HideDelay(Hmx::Object *o) : mHide(o, 0), mDelay(0), mRate(0) {}

void EventTrigger::SetNextLink(EventTrigger *trig) {
    for (EventTrigger *it = trig; it != nullptr; it = it->mNextLink) {
        if (it == this) {
            MILO_NOTIFY(
                "Setting next link causes infinite loop, setting next_link to NULL"
            );
            mNextLink = nullptr;
            return;
        }
    }
    mNextLink = trig;
}

void EventTrigger::CleanupHideShow() {
    mTriggered = false;
    mShown.clear();
    mHidden.clear();
}

void EventTrigger::Init() {
    REGISTER_OBJ_FACTORY(EventTrigger);
    DataRegisterFunc("cleanup_triggers", Cleanup);
}

void EventTrigger::LoadOldAnim(BinStream &bs, RndAnimatable *anim) {
    Anim eventAnim(this);
    bs >> (int &)eventAnim.mRate;
    int i48;
    bs >> i48;
    bs >> eventAnim.mStart;
    bs >> eventAnim.mEnd;
    bs >> eventAnim.mPeriod;
    bs >> eventAnim.mBlend;
    bool b88;
    bs >> b88;
    bs >> eventAnim.mDelay;
    if (i48 == 0) {
        eventAnim.mType = "range";
    } else if (b88) {
        eventAnim.mType = "dest";
    } else {
        eventAnim.mType = "loop";
    }
    if (anim) {
        eventAnim.mAnim = anim;
        mAnims.push_back(eventAnim);
    }
}

void EventTrigger::ConvertParticleTriggerType() {
    if (!unkd0) {
        if (Type() == "particle_trigger") {
            MILO_NOTIFY(
                "Converting particle trigger %s to standard EventTrigger; should re-save %s",
                Name(),
                Dir()->GetPathName()
            );
            DataArray *propArr = Property("systems", true)->Array();
            for (int i = 0; i < propArr->Size(); i++) {
                RndPartLauncher *p = propArr->Obj<RndPartLauncher>(i);
                if (p) {
                    mPartLaunchers.push_back(p);
                }
            }
            SetTypeDef(nullptr);
        }
    }
    unkd0 = true;
}

DataNode EventTrigger::OnProxyCalls(DataArray *) {
    DataArray *miloArr = DataVariable("milo_prop_path").Array();
    DataNode node2 = miloArr->Node(2);
    miloArr->Node(2) = Symbol("proxy");
    Hmx::Object *propDir = Property(miloArr, true)->Obj<ObjectDir>();
    miloArr->Node(2) = node2;

    DataArrayPtr ptr(new DataArray(0x200));
    int idx = 0;
    ptr->Node(idx++) = Symbol();
    if (propDir) {
        const DataArray *tdef = propDir->TypeDef();
        if (tdef) {
            for (int i = 1; i < tdef->Size(); i++) {
                DataArray *curArr = tdef->Array(i);
                if (curArr->Size() > 1 && curArr->Type(1) == kDataCommand) {
                    ptr->Node(idx++) = curArr->Sym(0);
                }
            }
        }
    }
    ptr->Resize(idx);
    return ptr;
}
