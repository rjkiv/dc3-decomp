#include "hamobj/HamPartyJumpData.h"
#include "obj/Object.h"

HamPartyJumpData::HamPartyJumpData() {}
HamPartyJumpData::~HamPartyJumpData() {}

BEGIN_HANDLERS(HamPartyJumpData)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

bool PropSync(
    std::pair<int, int> &pPair, DataNode &node, DataArray *prop, int i, PropOp op
) {
    if (i == prop->Size())
        return true;
    else {
        Symbol sym = prop->Sym(i);
        {
            _NEW_STATIC_SYMBOL(from_measure)
            if (sym == _s)
                return PropSync(pPair.first, node, prop, i + 1, op);
        }
        {
            _NEW_STATIC_SYMBOL(to_measure)
            if (sym == _s)
                return PropSync(pPair.second, node, prop, i + 1, op);
        }
        return false;
    }
}

BEGIN_PROPSYNCS(HamPartyJumpData)
    SYNC_PROP(jumps, mJumps)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(HamPartyJumpData)
    SAVE_REVS(0, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mJumps.size();
    for (std::vector<std::pair<int, int> >::iterator it = mJumps.begin();
         it != mJumps.end();
         ++it) {
        bs << it->first;
        bs << it->second;
    }
END_SAVES

BEGIN_COPYS(HamPartyJumpData)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(HamPartyJumpData)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mJumps)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(HamPartyJumpData)
    LOAD_REVS(bs)
    ASSERT_REVS(0, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    int numJumps;
    bs >> numJumps;
    mJumps.resize(numJumps);
    for (std::vector<std::pair<int, int> >::iterator it = mJumps.begin();
         it != mJumps.end();
         ++it) {
        bs >> it->first;
        bs >> it->second;
    }
END_LOADS
