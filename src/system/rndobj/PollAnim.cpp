#include "rndobj/PollAnim.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "rndobj/Anim.h"
#include "rndobj/Poll.h"
#include "utl/BinStream.h"

#pragma region Hmx::Object

RndPollAnim::RndPollAnim() : mAnims(this) {}

BEGIN_HANDLERS(RndPollAnim)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndPollAnim)
    SYNC_PROP(anims, mAnims)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(RndPollable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndPollAnim)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndAnimatable)
    SAVE_SUPERCLASS(RndPollable)
    bs << mAnims;
END_SAVES

BEGIN_COPYS(RndPollAnim)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    COPY_SUPERCLASS(RndPollable)
    CREATE_COPY(RndPollAnim)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mAnims)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(0, 0)

BEGIN_LOADS(RndPollAnim)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndAnimatable)
    LOAD_SUPERCLASS(RndPollable)
    bs >> mAnims;
END_LOADS

#pragma endregion
#pragma region RndAnimatable

float RndPollAnim::EndFrame() {
    float frame = 0;
    FOREACH (it, mAnims) {
        frame = Max(frame, (*it)->EndFrame());
    }
    return frame;
}

void RndPollAnim::ListAnimChildren(std::list<RndAnimatable *> &children) const {
    FOREACH (it, mAnims) {
        children.push_back(*it);
    }
}

#pragma endregion
#pragma region RndPollable

void RndPollAnim::Poll() {
    FOREACH (it, mAnims) {
        RndAnimatable *cur = *it;
        float frame = 0;
        switch (cur->GetRate()) {
        case RndAnimatable::k30_fps:
            frame = 30.0f * TheTaskMgr.Seconds(TaskMgr::kRealTime);
            break;
        case RndAnimatable::k480_fpb:
            frame = 480.0f * TheTaskMgr.Beat();
            break;
        case RndAnimatable::k30_fps_ui:
            frame = 30.0f * TheTaskMgr.UISeconds();
            break;
        case RndAnimatable::k1_fpb:
            frame = TheTaskMgr.Beat();
            break;
        case RndAnimatable::k30_fps_tutorial:
            frame = 30.0f * TheTaskMgr.TutorialSeconds();
            break;
        case RndAnimatable::k15_fpb:
            frame = 15.0f * TheTaskMgr.Beat();
            break;
        }
        cur->SetFrame(frame, 1);
    }
}

void RndPollAnim::Enter() {
    RndPollable::Enter();
    FOREACH (it, mAnims) {
        (*it)->StartAnim();
    }
}

void RndPollAnim::Exit() {
    RndPollable::Exit();
    FOREACH (it, mAnims) {
        (*it)->EndAnim();
    }
}

#pragma endregion
