#include "hamobj/BustAMoveData.h"
#include "obj/Object.h"
#include "utl/BinStream.h"

BustAMoveData::BustAMoveData() {}
BustAMoveData::~BustAMoveData() {}

BEGIN_CUSTOM_PROPSYNC(BAMPhrase)
    SYNC_PROP(count, o.count)
    SYNC_PROP(bars, o.bars)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(BustAMoveData)
    SYNC_PROP(phrases, mPhrases)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const BAMPhrase &bp) {
    bs << bp.count;
    bs << bp.bars;
    return bs;
}

BEGIN_SAVES(BustAMoveData)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mPhrases;
END_SAVES

BEGIN_COPYS(BustAMoveData)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(BustAMoveData)
    BEGIN_COPYING_MEMBERS
        mPhrases.clear();
        mPhrases.insert(mPhrases.begin(), c->mPhrases.begin(), c->mPhrases.end());
    END_COPYING_MEMBERS
END_COPYS

BinStreamRev &operator>>(BinStreamRev &bs, BAMPhrase &bp) {
    bs >> bp.count;
    bs >> bp.bars;
    return bs;
}

BEGIN_LOADS(BustAMoveData)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    bsrev >> mPhrases;
END_LOADS
