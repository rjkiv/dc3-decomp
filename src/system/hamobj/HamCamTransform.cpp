#include "hamobj/HamCamTransform.h"
#include "hamobj/HamCamShot.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Poll.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/Std.h"
#include "world/CameraShot.h"

HamCamTransform::HamCamTransform() : mAreas(this) {}

HamCamTransform::~HamCamTransform() { ClearOldCrowds(); }

BEGIN_HANDLERS(HamCamTransform)
    HANDLE_ACTION(update_camshots, Setup(true))
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(TransformCrowd)
    SYNC_PROP(crowd, o.mCrowd)
    SYNC_PROP(crowd_rotate, (int &)o.mCrowdRotate)
END_CUSTOM_PROPSYNC

BEGIN_CUSTOM_PROPSYNC(TransformArea)
    SYNC_PROP(area, o.mArea)
    SYNC_PROP(camshots, o.mCamshots)
    SYNC_PROP(anims, o.mAnims)
    SYNC_PROP(crowds, o.mCrowds)
    SYNC_PROP(flow, o.mFlow)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(HamCamTransform)
    SYNC_PROP(areas, mAreas)
    SYNC_SUPERCLASS(RndPollable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

INIT_REVS(3, 0)

BEGIN_SAVES(HamCamTransform)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mAreas;
END_SAVES

BEGIN_LOADS(HamCamTransform)
    LOAD_REVS(bs)
    ASSERT_REVS(3, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mAreas;
END_LOADS

BEGIN_COPYS(HamCamTransform)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(HamCamTransform)
    BEGIN_COPYING_MEMBERS
        mAreas.clear();
        for (int i = 0; i != c->mAreas.size(); i++) {
            mAreas.push_back(TransformArea(this, c->mAreas[i]));
        }
    END_COPYING_MEMBERS
END_COPYS

BinStream &operator>>(BinStreamRev &d, ObjVector<TransformArea> &);

BinStream &operator<<(BinStream &bs, const TransformCrowd &c) {
    c.Save(bs);
    return bs;
}

void TransformCrowd::Save(BinStream &bs) const { bs << mCrowd << mCrowdRotate; }

BinStream &operator>>(BinStream &bs, TransformCrowd &c) {
    c.Load(bs);
    return bs;
}

void TransformCrowd::Load(BinStream &bs) {
    bs >> mCrowd;
    bs >> (BinStreamEnum<CrowdRotate> &)mCrowdRotate;
}

TransformArea::TransformArea(Hmx::Object *owner)
    : mArea(owner), mCamshots(owner), mAnims(owner), mCrowds(owner), mFlow(owner) {}

TransformArea::TransformArea(Hmx::Object *owner, const TransformArea &other)
    : mArea(other.mArea), mCamshots(other.mCamshots), mAnims(other.mAnims),
      mCrowds(other.mCrowds), mFlow(other.mFlow) {}

BinStream &operator<<(BinStream &bs, const TransformArea &a) {
    a.Save(bs);
    return bs;
}

BinStream &operator>>(BinStreamRev &d, TransformArea &a) {
    a.Load(d);
    return d.stream;
}

void TransformArea::Save(BinStream &bs) const {
    bs << mArea;
    bs << mCamshots;
    bs << mAnims;
    bs << mCrowds;
    bs << mFlow;
}

void TransformArea::Load(BinStreamRev &d) {
    d >> mArea;
    mCamshots.Load(d.stream, false, nullptr, true);
    d >> mAnims;
    if (d.rev > 1) {
        d.stream >> mCrowds;
    }
    if (d.rev > 2) {
        d >> mFlow;
    }
}

void HamCamTransform::Enter() { Setup(false); }

void HamCamTransform::ClearOldCrowds() {
    for (int i = 0; i != mAreas.size(); i++) {
        TransformArea &area = mAreas[i];
        if (area.mArea) {
            auto &camShots = area.mCamshots;
            FOREACH (it, camShots) {
                auto camShot = *it;
                if (camShot) {
                    camShot->ClearCrowds();
                }
            }
        }
    }
}

void HamCamTransform::Setup(bool b) {
    ClearOldCrowds();
    for (int i = 0; i != mAreas.size(); i++) {
        TransformArea &area = mAreas[i];
        if (area.mArea) {
            FOREACH (it, area.mCamshots) {
                HamCamShot *camShot = *it;
                if (camShot) {
                    camShot->SetTransParent(area.mArea, false);
                    FOREACH (it2, area.mAnims) {
                        if (*it2) {
                            camShot->AddAnim(*it2);
                        }
                    }

                    if (area.mFlow) {
                        area.mFlow->Activate();
                    }

                    bool check = false;
                    FOREACH (it2, area.mCrowds) {
                        CamShotCrowd crowd(camShot);
                        crowd.unk24 = camShot;
                        crowd.mCrowd = (*it2).mCrowd;
                        crowd.mCrowdRotate = (*it2).mCrowdRotate;
                        camShot->AddCrowd(crowd);
                        if (!check && (*it2).mCrowd) {
                            check = true;
                        }
                    }
                }
            }
        }
    }
    if (b && TheLoadMgr.EditMode()) {
        DataNode node(DataReadString("milo cur_anim"));
        node.Array()->Release();
        DataNode result = node.Array()->Execute();
        if (result.Type() == kDataObject) {
            HamCamShot *shot = result.Obj<HamCamShot>();
            if (shot) {
                shot->StartAnim();
                shot->SetFrame(shot->GetFrame(), 1.0f);
            }
        }
    }
}
