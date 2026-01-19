#include "gesture/SkeletonDir.h"
#include "SkeletonClip.h"
#include "SkeletonDir.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "gesture/JointUtl.h"
#include "gesture/LiveCameraInput.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonUpdate.h"
#include "gesture/SkeletonViz.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Dir.h"
#include "ui/PanelDir.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"

SkeletonDir::SkeletonDir() : mMiloInitted(0), mTestClip(this) {}

SkeletonDir::~SkeletonDir() {
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    if (handle.HasCallback(this)) {
        handle.RemoveCallback(this);
    }
}

BEGIN_HANDLERS(SkeletonDir)
    HANDLE_ACTION(live_camera, SetSkeletonClip(nullptr))
    HANDLE_ACTION(print_skeleton, PrintSkeleton())
    HANDLE_SUPERCLASS(PanelDir)
END_HANDLERS

BEGIN_PROPSYNCS(SkeletonDir)
    SYNC_PROP_SET(test_clip, mTestClip.Ptr(), SetSkeletonClip(_val.Obj<SkeletonClip>()))
    SYNC_SUPERCLASS(PanelDir)
END_PROPSYNCS

BEGIN_SAVES(SkeletonDir)
    SAVE_REVS(4, 0)
    SAVE_SUPERCLASS(PanelDir)
    if (!IsProxy())
        bs << mTestClip;
END_SAVES

BEGIN_COPYS(SkeletonDir)
    COPY_SUPERCLASS(PanelDir)
    CREATE_COPY(SkeletonDir)
    BEGIN_COPYING_MEMBERS
        SetSkeletonClip(c->mTestClip);
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(SkeletonDir)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

void SkeletonDir::PreLoad(BinStream &bs) {
    LOAD_REVS(bs);
    ASSERT_REVS(4, 0);
    PanelDir::PreLoad(bs);
    bs.PushRev(packRevs(d.altRev, d.rev), this);
}

void SkeletonDir::PostLoad(BinStream &bs) {
    BinStreamRev d(bs, bs.PopRev(this));
    PanelDir::PostLoad(bs);
    if (!IsProxy()) {
        d >> mTestClip;
        if (d.rev < 4) {
            bool b40;
            if (d.rev > 0) {
                d >> b40;
            }
            if (d.rev > 1) {
                d >> b40;
                int x;
                d >> x;
                d >> x;
                d >> x;
                d >> x;
                d >> x;
            }
            if (d.rev > 2) {
                d >> b40;
            }
        }
    }
    mMiloInitted = true;
}

void SkeletonDir::DrawShowing() {
    PanelDir::DrawShowing();
    if (TheLoadMgr.EditMode()) {
        SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
        CameraInput *input = handle.GetCameraInput();
        ObjDirItr<SkeletonViz> it(this, true);
        if (it && it->Showing()) {
            Skeleton *skel = TheGestureMgr->GetActiveSkeleton();
            if (skel) {
                it->Visualize(*input, *skel, &handle.Callbacks(), false);
            }
        }
    }
}

void SkeletonDir::RemovingObject(Hmx::Object *obj) {
    PanelDir::RemovingObject(obj);
    if (obj == mTestClip) {
        SetSkeletonClip(nullptr);
    }
}

void SkeletonDir::StartAnim() { SetSkeletonClip(mTestClip); }

void SkeletonDir::MiloUpdate() {
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    if (!handle.HasCallback(this)) {
        handle.AddCallback(this);
    }
    if (!IsProxy())
        SetSkeletonClip(mTestClip);
    mMiloInitted = false;
}

void SkeletonDir::MiloInit() {
    MILO_ASSERT(TheLoadMgr.EditMode(), 0xE8);
    mMiloInitted = true;
}

void SkeletonDir::Poll() {
    RndDir::Poll();
    if (TheLoadMgr.EditMode() && mMiloInitted) {
        MiloUpdate();
    }
}

SkeletonClip *SkeletonDir::TestClip() const { return mTestClip; }

void SkeletonDir::PrintSkeleton() const {
    if (TheGestureMgr) {
        Skeleton *skel = TheGestureMgr->GetActiveSkeleton();
        if (skel) {
            MILO_LOG("bound: %i\n", skel->IsTracked());
            MILO_LOG("joints:\n");
            for (int i = 0; i < kNumJoints; i++) {
                TheDebug << "\t" << i << ":\t" << JointName((SkeletonJoint)i) << "\t\t"
                         << skel->TrackedJoints()[i].mJointPos[kCoordCamera] << "\n";
            }
        }
    } else {
        MILO_NOTIFY("GestureMgr is not initialized");
    }
}

void SkeletonDir::SetSkeletonClip(SkeletonClip *clip) {
    MILO_ASSERT(TheGestureMgr, 0x4E);
    mTestClip = clip;
    CameraInput *input = LiveCameraInput::sInstance;
    if (mTestClip) {
        input = mTestClip;
    }
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    handle.SetCameraInput(input);
}
