#include "world/Spotlight.h"
#include "Spotlight.h"
#include "SpotlightDrawer.h"
#include "math/Color.h"
#include "math/Mtx.h"
#include "math/Rot.h"
#include "math/Utl.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rnddx9/Mesh.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/Flare.h"
#include "rndobj/Group.h"
#include "rndobj/Mat.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "world/LightPreset.h"

RndEnviron *Spotlight::sEnviron;

#pragma region BeamDef

Spotlight::BeamDef::BeamDef(Hmx::Object *owner)
    : mBeam(nullptr), mIsCone(false), mLength(100), mTopRadius(4), mBottomRadius(30),
      mTopSideBorder(0.1), mBottomSideBorder(0.3), mBottomBorder(0.5), mOffset(0),
      mTargetOffset(0, 0), mBrighten(1), mExpand(1), mShape(), mNumSections(0),
      mNumSegments(0), mXSection(owner), mCutouts(owner), mMat(owner) {}

Spotlight::BeamDef::BeamDef(const Spotlight::BeamDef &def)
    : mBeam(0), mIsCone(def.mIsCone), mLength(def.mLength), mTopRadius(def.mTopRadius),
      mBottomRadius(def.mBottomRadius), mTopSideBorder(def.mTopSideBorder),
      mBottomSideBorder(def.mBottomSideBorder), mBottomBorder(def.mBottomBorder),
      mOffset(def.mOffset), mTargetOffset(def.mTargetOffset), mBrighten(def.mBrighten),
      mExpand(def.mExpand), mShape(def.mShape), mNumSections(def.mNumSections),
      mNumSegments(def.mNumSegments),
      mXSection(def.mXSection.Owner(), def.mXSection.Ptr()), mCutouts(def.mCutouts),
      mMat(def.mMat.Owner(), def.mMat.Ptr()) {
    if (def.mBeam) {
        mBeam = Hmx::Object::New<RndMesh>();
        mBeam->Copy(def.mBeam, kCopyDeep);
    }
}

Spotlight::BeamDef::~BeamDef() { RELEASE(mBeam); }

void Spotlight::BeamDef::OnSetMat(RndMat *mat) {
    mMat = mat;
    if (mBeam)
        mBeam->SetMat(mMat);
}

void Spotlight::BeamDef::Save(BinStream &bs) const {
    bs << mIsCone;
    bs << mLength;
    bs << mBottomRadius;
    bs << mTopRadius;
    bs << mTopSideBorder;
    bs << mBottomSideBorder;
    bs << mBottomBorder;
    bs << mMat;
    bs << mOffset;
    bs << mTargetOffset;
    bs << mBrighten;
    bs << mXSection;
    bs << mExpand;
    bs << mShape;
    bs << mCutouts;
    bs << mNumSections;
    bs << mNumSegments;
}

void Spotlight::BeamDef::Load(BinStreamRev &d) {
    d >> mIsCone;
    d >> mLength;
    d >> mBottomRadius;
    d >> mTopRadius;
    d >> mTopSideBorder;
    d >> mBottomSideBorder;
    d >> mBottomBorder;
    d >> mMat;
    if (d.rev > 0x11 && d.rev < 0x13) {
        char name[0x80];
        d.stream.ReadString(name, 0x80);
    }
    d >> mOffset;
    if (d.rev < 10) {
        Vector4 v;
        d >> v;
    }
    d >> mTargetOffset;
    if (d.rev > 0x14) {
        d >> mBrighten;
        d >> mXSection;
    }
    if (d.rev > 0x17) {
        d >> mExpand;
    }
    if (d.rev > 0x1A) {
        d >> (int &)mShape;
    }
    if (d.rev > 0x18) {
        d >> mCutouts;
    }
    if (d.rev > 0x1F) {
        d >> mNumSections;
        d >> mNumSegments;
    }
}

Vector2 Spotlight::BeamDef::NGRadii() const {
    Vector2 v;
    float vx = mTopRadius * mExpand;
    float vy = mBottomRadius * mExpand;
    if (!mIsCone) {
        vy *= 1.0f - mBottomSideBorder * 0.7f;
        vx *= 1.0f - mTopSideBorder * 0.7f;
    }
    v.Set(vx, vy);
    return v;
}

#pragma endregion
#pragma region Spotlight

Spotlight::Spotlight()
    : mSpotMaterial(this), mFlare(Hmx::Object::New<RndFlare>()), mFlareEnabled(true),
      mFlareVisibilityTest(true), mFlareOffset(0), mSpotScale(30), mSpotHeight(0.25),
      mColor(1, 1, 1), mIntensity(1), mColorOwner(this, this), mLensSize(0),
      mLensOffset(0), mLensMaterial(this), mBeam(this), mSlaves(this),
      mLightCanMesh(this), mLightCanOffset(0), mTarget(this), unk2f0(true),
      mSpotTarget(this), unk308(-1e33), mTargetShadow(false), mLightCanSort(false),
      unk340(true), mDampingConstant(1), mAdditionalObjects(this),
      mAnimateColorFromPreset(true), mAnimateOrientationFromPreset(true), unk36e(false) {
    mFlare->SetTransParent(this, false);
    mFloorSpotXfm.Reset();
    unk170.Reset();
    mLightCanXfm.Reset();
    unk310.Identity();
    unk35c.Zero();
    unk370.Reset();
    mOrder = -1000;
}

Spotlight::~Spotlight() {
    CloseSlaves();
    RemoveFromLists(this);
    RELEASE(mFlare);
}

bool Spotlight::Replace(ObjRef *from, Hmx::Object *to) {
    if (&mColorOwner == from) {
        if (!mColorOwner.SetObj(to)) {
            mColorOwner = this;
        }
        return true;
    } else {
        return RndTransformable::Replace(from, to);
    }
}

BEGIN_HANDLERS(Spotlight)
    HANDLE_ACTION(propogate_targeting_to_presets, PropogateToPresets(2))
    HANDLE_ACTION(propogate_coloring_to_presets, PropogateToPresets(1))
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(Spotlight)
    SYNC_PROP_MODIFY(length, mBeam.mLength, Generate())
    SYNC_PROP_MODIFY(top_radius, mBeam.mTopRadius, Generate())
    SYNC_PROP_MODIFY(bottom_radius, mBeam.mBottomRadius, Generate())
    SYNC_PROP_MODIFY(top_side_border, mBeam.mTopSideBorder, Generate())
    SYNC_PROP_MODIFY(bottom_side_border, mBeam.mBottomSideBorder, Generate())
    SYNC_PROP_MODIFY(bottom_border, mBeam.mBottomBorder, Generate())
    SYNC_PROP_SET(material, mBeam.mMat.Ptr(), mBeam.OnSetMat(_val.Obj<RndMat>()))
    SYNC_PROP_MODIFY(offset, mBeam.mOffset, Generate())
    SYNC_PROP_MODIFY(angle_offset, mBeam.mTargetOffset, Generate())
    SYNC_PROP_MODIFY(is_cone, mBeam.mIsCone, Generate())
    SYNC_PROP(brighten, mBeam.mBrighten)
    SYNC_PROP_MODIFY(expand, mBeam.mExpand, Generate())
    SYNC_PROP_MODIFY(shape, (int &)mBeam.mShape, Generate())
    SYNC_PROP(xsection, mBeam.mXSection)
    SYNC_PROP(cutouts, mBeam.mCutouts)
    SYNC_PROP_MODIFY(sections, mBeam.mNumSections, Generate())
    SYNC_PROP_MODIFY(segments, mBeam.mNumSegments, Generate())
    SYNC_PROP_MODIFY(light_can, mLightCanMesh, UpdateBounds())
    SYNC_PROP_MODIFY(light_can_offset, mLightCanOffset, UpdateBounds())
    SYNC_PROP(light_can_sort, mLightCanSort)
    SYNC_PROP_MODIFY(target, mTarget, UpdateTransforms())
    SYNC_PROP(target_shadow, mTargetShadow)
    SYNC_PROP_SET(flare_material, mFlare->GetMat(), mFlare->SetMat(_val.Obj<RndMat>()))
    SYNC_PROP(flare_size, mFlare->Sizes())
    SYNC_PROP(flare_range, mFlare->Range())
    SYNC_PROP_SET(flare_steps, mFlare->GetSteps(), mFlare->SetSteps(_val.Int()))
    SYNC_PROP_MODIFY(flare_offset, mFlareOffset, UpdateBounds())
    SYNC_PROP_MODIFY(flare_enabled, mFlareEnabled, UpdateFlare())
    SYNC_PROP_SET(
        flare_visibility_test, !mFlareVisibilityTest, SetFlareIsBillboard(!_val.Int())
    )
    SYNC_PROP_MODIFY(spot_target, mSpotTarget, UpdateBounds())
    SYNC_PROP_MODIFY(spot_scale, mSpotScale, UpdateBounds())
    SYNC_PROP_MODIFY(spot_height, mSpotHeight, UpdateBounds())
    SYNC_PROP_MODIFY(spot_material, mSpotMaterial, UpdateBounds())
    SYNC_PROP_SET(color, Color().Pack(), SetColor(_val.Int()))
    SYNC_PROP_SET(intensity, Intensity(), SetIntensity(_val.Float())) // fix this line
    SYNC_PROP(color_owner, mColorOwner)
    SYNC_PROP(damping_constant, mDampingConstant)
    SYNC_PROP_MODIFY(lens_size, mLensSize, UpdateBounds())
    SYNC_PROP_MODIFY(lens_offset, mLensOffset, UpdateBounds())
    SYNC_PROP_MODIFY(lens_material, mLensMaterial, UpdateBounds())
    SYNC_PROP(additional_objects, mAdditionalObjects)
    SYNC_PROP(slaves, mSlaves)
    SYNC_PROP(animate_orientation_from_preset, mAnimateOrientationFromPreset)
    SYNC_PROP(animate_color_from_preset, mAnimateColorFromPreset)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(RndPollable)
END_PROPSYNCS

void Spotlight::InitObject() {
    Hmx::Object::InitObject();
    Generate();
}

BEGIN_SAVES(Spotlight)
    SAVE_REVS(0x21, 0)
    SAVE_SUPERCLASS(RndPollable)
    SAVE_SUPERCLASS(RndDrawable)
    SAVE_SUPERCLASS(RndTransformable)
    bs << mSpotScale;
    bs << mSpotHeight;
    mBeam.Save(bs);
    bs << mLightCanMesh;
    bs << mTarget;
    bs << mSpotTarget;
    bs << mLightCanOffset;
    bs << mLightCanSort;
    bs << mColor;
    bs << mIntensity;
    bs << mSpotMaterial;
    bs << mDampingConstant;
    ObjPtr<RndMat> mat(this, mFlare->GetMat());
    bs << mat;
    bs << mFlare->Sizes();
    bs << mFlare->Range();
    bs << mFlare->GetSteps();
    bs << mFlareOffset;
    bs << mFlareEnabled;
    bs << mFlareVisibilityTest;
    bs << mLensSize;
    bs << mLensOffset;
    bs << mLensMaterial;
    bs << mAdditionalObjects;
    bs << mSlaves;
    bs << mTargetShadow;
    bs << mAnimateColorFromPreset;
    bs << mAnimateOrientationFromPreset;
    bs << mColorOwner;
END_SAVES

BEGIN_COPYS(Spotlight)
    COPY_SUPERCLASS(RndPollable)
    COPY_SUPERCLASS(RndTransformable)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(Spotlight)
    BEGIN_COPYING_MEMBERS
        if (ty != kCopyFromMax) {
            mFlare->Copy(c->mFlare, kCopyDeep);
            COPY_MEMBER(mFlareOffset)
            COPY_MEMBER(mLightCanMesh)
            COPY_MEMBER(mTarget)
            COPY_MEMBER(mSpotTarget)
            COPY_MEMBER(mSpotScale)
            COPY_MEMBER(mSpotHeight)
            SetColorIntensity(c->Color(), c->Intensity());
            COPY_MEMBER(mSpotMaterial)
            COPY_MEMBER(mDampingConstant)
            COPY_MEMBER(mLensSize)
            COPY_MEMBER(mLensOffset)
            COPY_MEMBER(mLensMaterial)
            COPY_MEMBER(mLightCanOffset)
            COPY_MEMBER(mLightCanSort)
            COPY_MEMBER(mFlareEnabled)
            COPY_MEMBER(mFlareVisibilityTest)
            UpdateFlare();
            COPY_MEMBER(mTargetShadow)
            COPY_MEMBER(mAnimateColorFromPreset)
            COPY_MEMBER(mAnimateOrientationFromPreset)
            COPY_MEMBER(mAdditionalObjects)
            COPY_MEMBER(mSlaves)
            COPY_MEMBER(mBeam.mIsCone)
            COPY_MEMBER(mBeam.mLength)
            COPY_MEMBER(mBeam.mBottomRadius)
            COPY_MEMBER(mBeam.mTopRadius)
            COPY_MEMBER(mBeam.mTopSideBorder)
            COPY_MEMBER(mBeam.mBottomSideBorder)
            COPY_MEMBER(mBeam.mBottomBorder)
            COPY_MEMBER(mBeam.mMat)
            COPY_MEMBER(mBeam.mTargetOffset)
            COPY_MEMBER(mBeam.mBrighten)
            COPY_MEMBER(mBeam.mExpand)
            COPY_MEMBER(mBeam.mShape)
            COPY_MEMBER(mBeam.mXSection)
            COPY_MEMBER(mBeam.mCutouts)
            COPY_MEMBER(mBeam.mOffset)
            COPY_MEMBER(mBeam.mNumSections)
            COPY_MEMBER(mBeam.mNumSegments)
            if (c->mBeam.mBeam) {
                mBeam.mBeam = Hmx::Object::New<RndMesh>();
                mBeam.mBeam->Copy(c->mBeam.mBeam, kCopyDeep);
            }
            Generate();
        }
    END_COPYING_MEMBERS
END_COPYS

BinStreamRev &operator>>(BinStreamRev &d, Spotlight::BeamDef &bd) {
    bd.Load(d);
    return d;
}

INIT_REVS(0x21, 0)

BEGIN_LOADS(Spotlight)
    LOAD_REVS(bs)
    ASSERT_REVS(0x21, 0)
    if (d.rev < 9) {
        MILO_FAIL("Unsupported spotlight version");
    } else {
        RndPollable::Load(bs);
        RndDrawable::Load(bs);
        RndTransformable::Load(bs);
        bs >> mSpotScale;
        bs >> mSpotHeight;
        if (d.rev > 0x16) {
            mBeam.Load(d);
        } else {
            ObjVector<BeamDef> beams(this);
            d >> beams;
            MILO_ASSERT(beams.size() <= 1, 0xCD);
            if (beams.size() != 0) {
                mBeam = beams[0];
            } else {
                mBeam.mLength = 0;
            }
        }
        if (d.rev > 0x15) {
            d >> mLightCanMesh;
        } else {
            ObjPtr<RndGroup> group(this);
            d >> group;
            ConvertGroupToMesh(group);
        }
        if (!mTarget.Load(bs, false, 0)) {
            unk2f0 = false;
        }
        if (d.rev > 0x1C) {
            d >> mSpotTarget;
        }
        d >> mLightCanOffset;
        if (d.rev > 0x1E) {
            d >> mLightCanSort;
        }
        d >> mColor;
        mColor.alpha = 1;
        if (d.rev > 9) {
            d >> mIntensity;
        }
        d >> mSpotMaterial;
        if (d.rev > 0x11 && d.rev < 0x13) {
            char buf[0x80];
            bs.ReadString(buf, 0x80);
            if (!mSpotMaterial && buf[0] != '\0') {
                mSpotMaterial = LookupOrCreateMat(buf, Dir());
            }
        }
        d >> mDampingConstant;
        if (d.rev < 0x21) {
            Symbol s;
            d >> s;
        }
        if (d.rev > 10) {
            ObjPtr<RndMat> mat(this);
            d >> mat;
            mFlare->SetMat(mat);
            if (d.rev > 0x11 && d.rev < 0x13) {
                char buf[0x80];
                bs.ReadString(buf, 0x80);
                if (!mat && buf[0] != '\0') {
                    mat = LookupOrCreateMat(buf, Dir());
                    mFlare->SetMat(mat);
                }
            }
            d >> mFlare->Sizes();
            d >> mFlare->Range();
            int steps;
            d >> steps;
            mFlare->SetSteps(steps);
            d >> mFlareOffset;
        }
        if (d.rev > 0xD) {
            d >> mFlareEnabled;
        }
        if (d.rev > 0xE) {
            d >> mFlareVisibilityTest;
        }
        UpdateFlare();
        if (d.rev > 0xB) {
            d >> mLensSize;
            d >> mLensOffset;
            d >> mLensMaterial;
        }
        if (d.rev > 0x11 && d.rev < 0x13) {
            char buf[0x80];
            bs.ReadString(buf, 0x80);
            if (!mLensMaterial && buf[0] != '\0') {
                mLensMaterial = LookupOrCreateMat(buf, Dir());
            }
        }
        if (d.rev > 0xC) {
            d >> mAdditionalObjects;
        }
        if (d.rev > 0x1B) {
            d >> mSlaves;
        }
        if (d.rev > 0xF) {
            d >> mTargetShadow;
        }
        if (d.rev > 0x19) {
            d >> mAnimateColorFromPreset;
            d >> mAnimateOrientationFromPreset;
        } else if (d.rev > 0x10) {
            d >> mAnimateColorFromPreset;
            mAnimateOrientationFromPreset = mAnimateColorFromPreset;
        }
        if (d.rev > 0x1D) {
            d >> mColorOwner;
            if (!mColorOwner) {
                mColorOwner = this;
            }
        }
        Generate();
    }
END_LOADS

void Spotlight::UpdateSphere() {
    Sphere s;
    MakeWorldSphere(s, true);
    Transform xfm;
    FastInvert(WorldXfm(), xfm);
    Multiply(s, xfm, s);
    SetSphere(s);
}

bool Spotlight::MakeWorldSphere(Sphere &s, bool b) {
    if (b) {
        s.Zero();
        if (mBeam.mBeam) {
            Sphere s28;
            if (mBeam.mBeam->MakeWorldSphere(s28, true)) {
                s.GrowToContain(s28);
            }
        }
        if (DoFloorSpot()) {
            MILO_ASSERT(sDiskMesh, 0x2FD);
            Sphere s38;
            sDiskMesh->SetWorldXfm(mFloorSpotXfm);
            if (sDiskMesh->MakeWorldSphere(s38, true)) {
                s.GrowToContain(s38);
            }
        }
        if (mFlare) {
            Sphere s48;
            if (mFlare->MakeWorldSphere(s48, true)) {
                s.GrowToContain(s48);
            }
        }
        if (mLightCanMesh) {
            Sphere s58;
            mLightCanMesh->SetWorldXfm(mLightCanXfm);
            if (mLightCanMesh->MakeWorldSphere(s58, true)) {
                s.GrowToContain(s58);
            }
        }
        return true;
    } else if (mSphere.GetRadius()) {
        Multiply(mSphere, WorldXfm(), s);
        return true;
    } else
        return false;
}

void Spotlight::ListDrawChildren(std::list<RndDrawable *> &draws) {
    if (mLightCanMesh)
        draws.push_back(mLightCanMesh);
    FOREACH (it, mAdditionalObjects) {
        draws.push_back(*it);
    }
}

RndDrawable *Spotlight::CollideShowing(const Segment &s, float &f, Plane &p) {
    if (mLightCanMesh) {
        mLightCanMesh->SetWorldXfm(mLightCanXfm);
        bool showing = mLightCanMesh->Showing();
        mLightCanMesh->SetShowing(true);
        bool collide = mLightCanMesh->Collide(s, f, p);
        mLightCanMesh->SetShowing(showing);
        if (collide) {
            return this;
        }
    }
    return nullptr;
}

int Spotlight::CollidePlane(const Plane &pl) {
    if (mLightCanMesh) {
        mLightCanMesh->SetWorldXfm(mLightCanXfm);
        bool oldshowing = mLightCanMesh->Showing();
        mLightCanMesh->SetShowing(true);
        int coll = mLightCanMesh->CollidePlane(pl);
        mLightCanMesh->SetShowing(oldshowing);
        if (coll)
            return coll;
    }
    return -1;
}

void Spotlight::UpdateBounds() {
    UpdateTransforms();
    UpdateSphere();
}

void Spotlight::SetFlareIsBillboard(bool b) {
    mFlareVisibilityTest = b;
    UpdateFlare();
}

void Spotlight::SetColor(int packed) { SetColorIntensity(packed, Intensity()); }
void Spotlight::SetIntensity(float f) { SetColorIntensity(Color(), f); }

void Spotlight::SetColorIntensity(const Hmx::Color &c, float f) {
    mColorOwner->mColor = c;
    mColorOwner->mIntensity = f;
}

void Spotlight::Init() {
    REGISTER_OBJ_FACTORY(Spotlight)
    sEnviron = Hmx::Object::New<RndEnviron>();
    BuildBoard();
}

void Spotlight::BuildBoard() {
    MILO_ASSERT(!sDiskMesh, 0x42E);
    sDiskMesh = Hmx::Object::New<RndMesh>();
    RndMesh::VertVector &verts = sDiskMesh->Verts();
    std::vector<RndMesh::Face> &faces = sDiskMesh->Faces();
    verts.resize(4);
    faces.resize(2);

    verts[0].pos.Set(-0.5, -0.5, 0);
    verts[0].color = Hmx::Color(1, 1, 1);
    verts[0].tex.Set(0, 0);

    verts[1].pos.Set(0.5, -0.5, 0);
    verts[1].color = Hmx::Color(1, 1, 1);
    verts[1].tex.Set(1, 0);

    verts[2].pos.Set(-0.5, 0.5, 0);
    verts[2].color = Hmx::Color(1, 1, 1);
    verts[2].tex.Set(0, 1);

    verts[3].pos.Set(0.5, 0.5, 0);
    verts[3].color = Hmx::Color(1, 1, 1);
    verts[3].tex.Set(1, 1);

    faces[0].Set(0, 1, 2);
    faces[1].Set(1, 3, 2);
    sDiskMesh->Sync(0x13F);
    DxMesh *dxDiskMesh = static_cast<DxMesh *>(sDiskMesh);
    dxDiskMesh->GetMultimeshFaces();
    sDiskMesh->UpdateSphere();
}

void Spotlight::UpdateFlare() {
    if (!mFlareEnabled) {
        mFlare->SetVisible(false);
        mFlare->SetPointTest(false);
    } else if (mFlareVisibilityTest) {
        mFlare->SetVisible(true);
        mFlare->SetPointTest(false);
    } else
        mFlare->SetPointTest(true);
}

bool Spotlight::DoFloorSpot() const {
    return mSpotMaterial && GetFloorSpotTarget()
        && GetFloorSpotTarget()->WorldXfm().m.y.z;
}

void Spotlight::CalculateDirection(RndTransformable *target, Hmx::Matrix3 &mtx) {
    MILO_ASSERT(target, 0x2CE);
    Vector3 v20;
    Subtract(target->WorldXfm().v, WorldXfm().v, v20);
    Vector3 v2c;
    Cross(v20, Vector3(1.0f, 0.0f, 0.0f), v2c);
    Normalize(v2c, v2c);
    MakeRotMatrix(v20, v2c, mtx);
}

void Spotlight::SetFlareEnabled(bool b) {
    mFlareEnabled = b;
    UpdateFlare();
}

void Spotlight::CloseSlaves() {
    FOREACH (it, mSlaves) {
        if (*it)
            (*it)->SetShadowOverride(nullptr);
    }
}

void Spotlight::UpdateSlaves() {
    if (mSlaves.empty())
        return;
    else {
        FOREACH (it, mSlaves) {
            RndLight *lit = *it;
            Transform tf40(WorldXfm());
            if (lit->TransParent()) {
                Transform tf70;
                Invert(lit->TransParent()->WorldXfm(), tf70);
                Multiply(WorldXfm(), tf70, tf40);
            }
            lit->SetLocalXfm(tf40);
            lit->SetShadowOverride(&mBeam.mCutouts);
            lit->SetShowing(Showing());
        }
    }
}

void Spotlight::CheckFloorSpotTransform() {
    if (DoFloorSpot()) {
        if (GetFloorSpotTarget()->WorldXfm().v.z != unk308) {
            UpdateFloorSpotTransform(WorldXfm());
        }
    }
}

void Spotlight::ConvertGroupToMesh(RndGroup *grp) {
    if (grp) {
        int count = 0;
        std::vector<RndDrawable *>::const_iterator it = grp->Draws().begin();
        std::vector<RndDrawable *>::const_iterator itEnd = grp->Draws().end();
        for (; it != itEnd; it++) {
            RndMesh *cur = dynamic_cast<RndMesh *>(*it);
            if (cur) {
                count++;
                if (!mLightCanMesh)
                    mLightCanMesh = cur;
            }
        }
        if (count > 1) {
            MILO_NOTIFY(
                "Multiple meshes (%d) found converting light can group %s to mesh",
                count,
                grp->Name()
            );
        }
    }
}

void Spotlight::PropogateToPresets(int i) {
    for (ObjDirItr<LightPreset> it(Dir(), false); it != nullptr; ++it) {
        it->SetSpotlight(this, i);
    }
}

void Spotlight::Generate() {
    if (!mBeam.mBeam || TheLoadMgr.EditMode()) {
        RELEASE(mBeam.mBeam);
        if (mBeam.HasLength()) {
            if (SpotlightDrawer::DrawNGSpotlights()) {
                BuildNGShaft(mBeam);
            } else if (mBeam.IsCone()) {
                BuildCone(mBeam);
            } else {
                BuildBeam(mBeam);
            }
        }
        UpdateBounds();
        UpdateSphere();
    }
}

void Spotlight::BuildNGShaft(Spotlight::BeamDef &def) {
    switch (def.mShape) {
    case BeamDef::kBeamRect:
        BuildNGCone(def, 4);
        break;
    case BeamDef::kBeamSheet:
        BuildNGSheet(def);
        break;
    case BeamDef::kBeamQuadXYZ:
        BuildNGQuad(def, RndTransformable::kConstraintBillboardXYZ);
        break;
    case BeamDef::kBeamQuadZ:
        BuildNGQuad(def, RndTransformable::kConstraintBillboardZ);
        break;
    default:
        int num = def.mNumSegments;
        if (def.mNumSegments <= 3) {
            num = 10;
        }
        BuildNGCone(def, num);
        break;
    }
}
