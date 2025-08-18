#include "rndobj/PollAnim.h"

#include "obj/Object.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Poll.h"
#include "utl/BinStream.h"

float RndPollAnim::EndFrame() {
    float frame = 0.0f;
    for (ObjPtrList<RndAnimatable>::iterator it = mAnims.begin(); it != mAnims.end();
         ++it) {
        float thisendframe = (*it)->EndFrame();
        if (frame < thisendframe)
            frame = thisendframe;
    }
    return frame;
}

void RndPollAnim::Save(BinStream &bs) {
    bs << 0;
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndAnimatable)
    SAVE_SUPERCLASS(RndPollable)
    bs << mAnims;
}

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

RndPollAnim::RndPollAnim() : mAnims(this, kObjListNoNull) {}

BEGIN_LOADS(RndPollAnim)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndAnimatable)
    LOAD_SUPERCLASS(RndPollable)
    bs >> mAnims;
END_LOADS

BEGIN_COPYS(RndPollAnim)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    COPY_SUPERCLASS(RndPollable)
    CREATE_COPY(RndPollAnim)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mAnims)
    END_COPYING_MEMBERS
END_COPYS
