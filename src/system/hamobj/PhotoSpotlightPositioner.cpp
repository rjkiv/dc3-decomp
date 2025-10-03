#include "hamobj/PhotoSpotlightPositioner.h"
#include "obj/Object.h"

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

BEGIN_LOADS(PhotoSpotlightPositioner)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    bs >> mPlayer;
    bs >> mSpotlight;
    bs >> mRefImage;
END_LOADS

void PhotoSpotlightPositioner::Init() { REGISTER_OBJ_FACTORY(PhotoSpotlightPositioner); }

Vector3 PhotoSpotlightPositioner::GetImagePos(Vector2 v2) const {
    return mRefImage->WorldXfm().v;
}
