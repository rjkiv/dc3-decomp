#include "hamobj/CrazeHollaback.h"
#include "obj/Object.h"

CrazeHollaback::CrazeHollaback() {}

BEGIN_HANDLERS(CrazeHollaback)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CrazeHollaback)
    SYNC_PROP(music_range, mMusicRange)
    SYNC_PROP(play_range, mPlayRange)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CrazeHollaback)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mMusicRange.start;
    bs << mMusicRange.end;
    bs << mPlayRange.start;
    bs << mPlayRange.end;
END_SAVES

BEGIN_COPYS(CrazeHollaback)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(CrazeHollaback)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMusicRange.start)
        COPY_MEMBER(mMusicRange.end)
        COPY_MEMBER(mPlayRange.start)
        COPY_MEMBER(mPlayRange.end)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(CrazeHollaback)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    bs >> mMusicRange.start;
    bs >> mMusicRange.end;
    bs >> mPlayRange.start;
    bs >> mPlayRange.end;
END_LOADS
