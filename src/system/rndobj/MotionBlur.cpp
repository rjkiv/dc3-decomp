#include "rndobj/MotionBlur.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "rndobj/Draw.h"
#include "rndobj/Group.h"
#include "rndobj/Mesh.h"
#include "rndobj/PostProc.h"
#include "utl/BinStream.h"

RndMotionBlur::RndMotionBlur() : mDrawList(this) {}

BEGIN_HANDLERS(RndMotionBlur)
    HANDLE(allowed_drawable, OnAllowedDrawable)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndMotionBlur)
    SYNC_PROP(draw_list, mDrawList)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndMotionBlur)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mDrawList;
END_SAVES

BEGIN_COPYS(RndMotionBlur)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(RndMotionBlur)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mDrawList)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(1, 0)

BEGIN_LOADS(RndMotionBlur)
    LOAD_REVS(bs);
    ASSERT_REVS(1, 0);
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndDrawable)
    bs >> mDrawList;
END_LOADS

void RndMotionBlur::DrawShowing() {
    RndPostProc *cur = RndPostProc::Current();
    if (cur) {
        for (ObjPtrList<RndDrawable>::iterator it = mDrawList.begin();
             it != mDrawList.end();
             ++it) {
            cur->QueueMotionBlurObject(*it);
        }
    }
}

DataNode RndMotionBlur::OnAllowedDrawable(const DataArray *da) {
    int allowcount = 0;
    for (ObjDirItr<RndDrawable> it(Dir(), true); it != 0; ++it) {
        if (CanMotionBlur(it))
            allowcount++;
    }
    DataArrayPtr ptr(new DataArray(allowcount));
    allowcount = 0;
    for (ObjDirItr<RndDrawable> it(Dir(), true); it != 0; ++it) {
        if (CanMotionBlur(it)) {
            ptr->Node(allowcount++) = &*it;
        }
    }
    return ptr;
}

bool RndMotionBlur::CanMotionBlur(RndDrawable *d) {
    return dynamic_cast<RndMesh *>(d) || dynamic_cast<RndDir *>(d)
        || dynamic_cast<RndGroup *>(d);
}
