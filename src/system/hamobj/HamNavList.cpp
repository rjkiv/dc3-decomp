#include "hamobj/HamNavList.h"
#include "HamNavList.h"
#include "gesture/SkeletonUpdate.h"
#include "synth/Sound.h"
#include "utl/Std.h"

HamNavList::HamNavList()
    : unk60(0), unk70(this, this), unkc4(1), unkc8(0), unkcc(this), unke4(this),
      unkfc(this), unk114(this), unk12c(this), unk140(this), unk154(0), unk155(0),
      unk156(0), unk157(0), unk158(0), unk15c(0, 10, 10), unk170(0, 10, 0), unk184(0),
      unk188(0), unk18c(0), unk190(this, &unk70), unk1e4(0), unk1e5(0), unk1e6(1),
      unk1e7(1), unk1e8(1), unk1e9(1), unk1ec(0), unk1f0(0), unk1f8(-1), unk1fd(0),
      unk1fe(0) {
    unk70.SetSpeed(0);
    unk70.SetSelected(0, -1, true);
    SetRate(k30_fps_ui);
}

HamNavList::~HamNavList() {
    DeleteAll(unk64);
    SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
    if (handle.HasCallback(this)) {
        handle.RemoveCallback(this);
    }
    // delete unk184;
    // delete unk188;
    if (unkcc) {
        Sound *slideSound = unkcc->SlideSound();
        if (slideSound)
            slideSound->Stop(nullptr, false);
    }
}
