#include "gesture/IdentityInfo.h"
#include "gesture/GestureMgr.h"
#include "meta_ham/SkeletonIdentifier.h"

void IdentityInfo::PostUpdate() {
    if (unk0) {
        Identified(mEnrollmentIdx);
        unk0 = false;
    }

    if (unk9) {
        unk9 = false;
        static SkeletonEnrollmentChangedMsg msg;
        TheGestureMgr->Export(msg, true);
    }
}

void IdentityInfo::Identified(unsigned int i) {
    GestureMgr::sIdentityOpInProgress = false;
    int val = i;
    switch (val) {
    case -5:
        val = -2;
        break;
    case -4:
        val = -2;
        break;
    case -2:
        val = -1;
        break;
    case -1:
        val = -2;
        break;
    }

    SkeletonIdentifiedMsg msg(val, unkc);
    TheGestureMgr->Export(msg, true);
}
