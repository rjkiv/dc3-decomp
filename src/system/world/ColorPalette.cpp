#include "world/ColorPalette.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"

ColorPalette::ColorPalette() {}

BEGIN_HANDLERS(ColorPalette)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(ColorPalette)
    SYNC_PROP(colors, mColors)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(ColorPalette)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mColors;
END_SAVES

BEGIN_COPYS(ColorPalette)
    CREATE_COPY(ColorPalette)
    MILO_ASSERT(c, 0x4A);
    COPY_SUPERCLASS_FROM(Hmx::Object, c)
    COPY_MEMBER(mColors)
END_COPYS

BinStream &operator>>(BinStream &bs, ColorSet &cs) {
    bs >> cs.mPrimary;
    bs >> cs.mSecondary;
    return bs;
}

BEGIN_LOADS(ColorPalette)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    if (d.rev < 1) {
        std::vector<ColorSet> vec;
        bs >> vec;
        mColors.clear();
        for (std::vector<ColorSet>::iterator it = vec.begin(); it != vec.end(); ++it) {
            mColors.push_back(it->mPrimary);
        }
    } else
        bs >> mColors;
END_LOADS
