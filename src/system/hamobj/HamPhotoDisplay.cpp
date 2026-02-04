#include "hamobj/HamPhotoDisplay.h"
#include "gesture/GestureMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "rndobj/Env.h"
#include "rndobj/Mesh.h"
#include "utl/BinStream.h"

HamPhotoDisplay::HamPhotoDisplay() : mMesh1(this), mMesh2(this), mIndex1(0), mIndex2(0) {}

BEGIN_HANDLERS(HamPhotoDisplay)
    HANDLE_SUPERCLASS(RndDir)
END_HANDLERS

BEGIN_PROPSYNCS(HamPhotoDisplay)
    SYNC_PROP(mesh1, mMesh1)
    SYNC_PROP(mesh2, mMesh2)
    SYNC_PROP_SET(
        index1, mIndex1, if (_val.Type() == kDataInt) mIndex1 = _val.Int();
        else mIndex1 = _val.Float();
    )
    SYNC_PROP_SET(
        index2, mIndex2, if (_val.Type() == kDataInt) mIndex2 = _val.Int();
        else mIndex2 = _val.Float();
    )
    SYNC_SUPERCLASS(RndDir)
END_PROPSYNCS

BEGIN_SAVES(HamPhotoDisplay)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(RndDir)
    if (!IsProxy()) {
        bs << mMesh1 << mMesh2;
    }
END_SAVES

BEGIN_COPYS(HamPhotoDisplay)
    COPY_SUPERCLASS(RndDir)
    CREATE_COPY(HamPhotoDisplay)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMesh1)
        COPY_MEMBER(mMesh2)
    END_COPYING_MEMBERS
END_COPYS

void HamPhotoDisplay::Init() { REGISTER_OBJ_FACTORY(HamPhotoDisplay); }

INIT_REVS(1, 0)

void HamPhotoDisplay::PreLoad(BinStream &bs) {
    LOAD_REVS(bs);
    ASSERT_REVS(1, 0);
    RndDir::PreLoad(d.stream);
    d.PushRev(this);
}

void HamPhotoDisplay::PostLoad(BinStream &bs) {
    BinStreamRev d(bs, bs.PopRev(this));
    RndDir::PostLoad(d.stream);
    if (!IsProxy() || d.rev < 1) {
        d >> mMesh1;
        d >> mMesh2;
    }
}

void HamPhotoDisplay::DrawShowing() {
    if (!mDraws.empty()) {
        RndEnvironTracker tracker(mEnv, &WorldXfm().v);
        FOREACH (it, mDraws) {
            if (*it == mMesh1) {
                DrawPhotoMesh(mMesh1, 0);
            } else if (*it == mMesh2) {
                DrawPhotoMesh(mMesh2, 1);
            } else {
                (*it)->Draw();
            }
        }
    }
}

void HamPhotoDisplay::DrawPhotoMesh(RndMesh *mesh, int i2) {
    if (TheGestureMgr && TheGestureMgr->GetLiveCameraInput()) {
        RndMat *snapshot =
            TheGestureMgr->GetLiveCameraInput()->GetSnapshot(i2 == 0 ? mIndex1 : mIndex2);
        mesh->SetMat(snapshot);
    }
    mesh->Draw();
}
