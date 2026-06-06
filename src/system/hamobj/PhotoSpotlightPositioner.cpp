#include "hamobj/PhotoSpotlightPositioner.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "hamobj/HamGameData.h"
#include "obj/Object.h"
#include "utl/Loader.h"

PhotoSpotlightPositioner::PhotoSpotlightPositioner()
    : mPlayer(0), mSpotlight(this), mRefImage(this) {}
PhotoSpotlightPositioner::~PhotoSpotlightPositioner() {}

BEGIN_HANDLERS(PhotoSpotlightPositioner)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(PhotoSpotlightPositioner)
    SYNC_PROP(player, mPlayer)
    SYNC_PROP(spotlight, mSpotlight)
    SYNC_PROP(ref_image, mRefImage)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(PhotoSpotlightPositioner)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mPlayer;
    bs << mSpotlight;
    bs << mRefImage;
END_SAVES

BEGIN_COPYS(PhotoSpotlightPositioner)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(PhotoSpotlightPositioner)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mPlayer)
        COPY_MEMBER(mSpotlight)
        COPY_MEMBER(mRefImage)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(1, 0)

BEGIN_LOADS(PhotoSpotlightPositioner)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mPlayer;
    d >> mSpotlight;
    d >> mRefImage;
END_LOADS

void PhotoSpotlightPositioner::Init() { REGISTER_OBJ_FACTORY(PhotoSpotlightPositioner); }

Vector3 PhotoSpotlightPositioner::GetImagePos(Vector2 v2) const {
    Transform xcopy;
    Vector3 ret;
    memcpy(&xcopy, &mRefImage->WorldXfm(), sizeof(Transform));
    ret.y = 0.0f;
    ret.x = -((1.0f - v2.x) * xcopy.m.x.x - (xcopy.m.x.x / 2.0f + xcopy.v.x));
    ret.z = -(v2.y * xcopy.m.z.z - (xcopy.m.z.z / 2.0f + xcopy.v.z));
    return ret;
}

void PhotoSpotlightPositioner::Poll() {
    Skeleton *my_skeleton = TheGestureMgr->GetSkeletonByTrackingID(
        TheGameData->Player(mPlayer)->GetSkeletonTrackingID()
    );
    if (mSpotlight != nullptr && !TheLoadMgr.EditMode()) {
        if (my_skeleton != nullptr) {
            Vector2 rfootpos, lfootpos;
            my_skeleton->ScreenPos(kJointFootRight, rfootpos);
            my_skeleton->ScreenPos(kJointFootLeft, lfootpos);
            Vector2 imageposbounds;
            imageposbounds.y = Max(lfootpos.y, rfootpos.y);
            imageposbounds.x = (lfootpos.x + rfootpos.x) / 2;
            Vector3 imgpos = GetImagePos(imageposbounds);
            mSpotlight->SetWorldPos(imgpos);
        } else {
            mSpotlight->SetWorldPos(GetImagePos(Vector2(-10.0f, -10.0f)));
        }
    }
}
