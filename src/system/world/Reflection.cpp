#include "world/Reflection.h"
#include "char/Character.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Rnd.h"
#include "rndobj/Trans.h"

WorldReflection::WorldReflection()
    : mDraws(this), mLodChars(this), mVerticalStretch(1), unk138(false), mHideList(this),
      mShowList(this), unk164(this), unk178(this) {
    unk134 = ObjectDir::Main()->New<RndCam>("");
}

WorldReflection::~WorldReflection() { delete unk134; }

BEGIN_HANDLERS(WorldReflection)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(WorldReflection)
    SYNC_PROP(vertical_stretch, mVerticalStretch)
    SYNC_PROP(draws, mDraws)
    SYNC_PROP(lod_chars, mLodChars)
    SYNC_PROP(hide_list, mHideList)
    SYNC_PROP(show_list, mShowList)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(WorldReflection)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndTransformable)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mVerticalStretch;
    bs << mDraws;
    bs << mHideList;
    bs << mShowList;
    bs << mLodChars;
END_SAVES

BEGIN_COPYS(WorldReflection)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndTransformable)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(WorldReflection)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mVerticalStretch)
        COPY_MEMBER(mDraws)
        COPY_MEMBER(mHideList)
        COPY_MEMBER(mShowList)
        COPY_MEMBER(mLodChars)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(WorldReflection)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndTransformable)
    LOAD_SUPERCLASS(RndDrawable)
    bs >> mVerticalStretch;
    bs >> mDraws;
    if (d.rev > 1) {
        bs >> mHideList;
        bs >> mShowList;
    }
    if (d.rev > 2) {
        bs >> mLodChars;
    }
END_LOADS

void WorldReflection::DoHide() {
    unk164.clear();
    unk178.clear();
    for (ObjPtrList<RndDrawable>::iterator it = mHideList.begin(); it != mHideList.end();
         ++it) {
        RndDrawable *cur = *it;
        if (cur->Showing()) {
            cur->SetShowing(false);
            unk178.push_back(cur);
        }
    }
    for (ObjPtrList<RndDrawable>::iterator it = mShowList.begin(); it != mShowList.end();
         ++it) {
        RndDrawable *cur = *it;
        if (!cur->Showing()) {
            cur->SetShowing(true);
            unk164.push_back(cur);
        }
    }
}

void WorldReflection::UnHide() {
    for (ObjPtrList<RndDrawable>::iterator it = unk164.begin(); it != unk164.end();
         ++it) {
        (*it)->SetShowing(false);
    }
    for (ObjPtrList<RndDrawable>::iterator it = unk178.begin(); it != unk178.end();
         ++it) {
        (*it)->SetShowing(true);
    }
    unk164.clear();
    unk178.clear();
}

void WorldReflection::DoLOD(int i) {
    FOREACH (it, mLodChars) {
        Character *c = *it;
        if (c)
            c->SetLodType((LODType)i);
    }
}

void WorldReflection::DrawShowing() {
    START_AUTO_TIMER("world_reflect");
    if (unk138) {
        unk138 = true;
        RndCam *cur = RndCam::Current();
        unk134->Copy(RndCam::Current(), kCopyDeep);
        Transform tf48(WorldXfm());
        Transform tf78;
        Invert(tf48, tf78);
        Transform tfa8;
        tfa8.Reset();
        tfa8.m.z.z = -mVerticalStretch;
        Multiply(tf78, tfa8, tfa8);
        Multiply(tfa8, tf48, tfa8);
        Multiply(cur->WorldXfm(), tfa8, unk134->DirtyLocalXfm());
        unk134->Select();
        Rnd::DrawMode oldMode = TheRnd.GetDrawMode();
        TheRnd.SetDrawMode((Rnd::DrawMode)8);
        DoHide();
        DoLOD(1);
        FOREACH (it, mDraws) {
            RndDrawable *cur = *it;
            if (cur)
                cur->Draw();
        }
        DoLOD(-1);
        UnHide();
        TheRnd.SetDrawMode(oldMode);
        cur->Select();
        unk138 = false;
    }
}
