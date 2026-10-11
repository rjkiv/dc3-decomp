#include "gesture/IdentityInfo.h"
#include "gesture/GestureMgr.h"
#include "meta_ham/SkeletonIdentifier.h"

void IdentityInfo::PostUpdate() {
    if (mJustIdentified) {
        Identified(mEnrollmentIndex);
        mJustIdentified = false;
    }

    if (mEnrollmentUpdated) {
        mEnrollmentUpdated = false;
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

    SkeletonIdentifiedMsg msg(val, mSkeletonIdx);
    TheGestureMgr->Export(msg, true);
}
