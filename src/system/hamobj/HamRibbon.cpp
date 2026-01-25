#include "hamobj/HamRibbon.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "os/File.h"
#include "rndobj/Draw.h"
#include "rndobj/Mesh.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "utl/Loader.h"

HamRibbon::HamRibbon()
    : unk4c(-1), mNumSides(4), mMat(this), mWidth(1), unk70(1), mActive(1), unk78(this),
      mNumSegments(0), mDecay(1), mFollowA(this), mFollowB(this), mFollowWeight(0),
      mTaper(0) {
    mMesh = Hmx::Object::New<RndMesh>();
    unk48 = false;
}

HamRibbon::~HamRibbon() { RELEASE(mMesh); }

BEGIN_HANDLERS(HamRibbon)
    HANDLE_ACTION(expose_mesh, ExposeMesh())
    HANDLE_ACTION(create_transs, unk48 = true)
    HANDLE_ACTION(reset, Reset())
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(HamRibbon)
    SYNC_PROP_SET(active, mActive, SetActive(_val.Int()))
    SYNC_PROP_MODIFY(num_sides, mNumSides, unk70 |= 1)
    SYNC_PROP_MODIFY(num_segments, mNumSegments, unk70 |= 1)
    SYNC_PROP_MODIFY(mat, mMat, mMesh->SetMat(mMat))
    SYNC_PROP_MODIFY(width, mWidth, unk70 |= 2)
    SYNC_PROP(follow_a, mFollowA)
    SYNC_PROP(follow_b, mFollowB)
    SYNC_PROP(follow_weight, mFollowWeight)
    SYNC_PROP(taper, mTaper)
    SYNC_PROP(decay, mDecay)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(RndPollable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(HamRibbon)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mNumSides;
    bs << mMat;
    bs << mActive;
    bs << mWidth;
    bs << mNumSegments;
    bs << mFollowA;
    bs << mFollowB;
    bs << mFollowWeight;
    bs << mTaper;
    bs << mDecay;
END_SAVES

BEGIN_COPYS(HamRibbon)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(HamRibbon)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mNumSides)
        COPY_MEMBER(mMat)
        COPY_MEMBER(mActive)
        COPY_MEMBER(mWidth)
        COPY_MEMBER(mNumSegments)
        COPY_MEMBER(mFollowA)
        COPY_MEMBER(mFollowB)
        COPY_MEMBER(mFollowWeight)
        COPY_MEMBER(mTaper)
        COPY_MEMBER(mDecay)
    END_COPYING_MEMBERS
END_COPYS

void HamRibbon::Poll() {
    if (unk70 & 1) {
        ConstructMesh();
        unk70 = 0;
    }
    UpdateChase();
    unk70 = 0;
}

void HamRibbon::DrawShowing() {
    if (mActive || TheLoadMgr.EditMode()) {
        mMesh->DrawShowing();
    }
}

void HamRibbon::Reset() { unk8c.clear(); }

void HamRibbon::ExposeMesh() {
    if (!mMesh->Dir()) {
        mMesh->SetName(MakeString("%s_mesh.mesh", FileGetBase(mMesh->Name())), Dir());
    }
}

void HamRibbon::SetActive(bool active) {
    if (mActive != active) {
        unk8c.clear();
        unk4c = -1;
    }
    mActive = active;
}

void HamRibbon::ConstructMesh() {
    unk78.DeleteAll();
    if (mNumSegments > 0) {
        mMesh->SetLocalXfm(Transform::IDXfm());
        mMesh->SetNumBones(mNumSegments);
        for (int i = 0; i < mNumSegments; i++) {
            RndTransformable *t = Hmx::Object::New<RndTransformable>();
            t->SetLocalXfm(Transform::IDXfm());
            mMesh->SetBone(i, t, true);
            unk78.push_back(t);
        }
        mMesh->Verts().resize(mNumSides * mNumSegments * 2);
        mMesh->Faces().resize(mNumSides * mNumSegments * 2);

        // Generate faces
        for (int seg = 0; seg < mNumSegments; seg++) {
            for (int side = 0; side < mNumSides; side++) {
                int baseIdx = (seg * mNumSides + side) * 2;
                int nextSide = (side + 1) % mNumSides;
                int nextIdx = (seg * mNumSides + nextSide) * 2;

                int faceIdx = (seg * mNumSides + side) * 2;
                mMesh->Faces()[faceIdx].v1 = baseIdx;
                mMesh->Faces()[faceIdx].v2 = nextIdx;
                mMesh->Faces()[faceIdx].v3 = baseIdx + 1;

                mMesh->Faces()[faceIdx + 1].v1 = baseIdx + 1;
                mMesh->Faces()[faceIdx + 1].v2 = nextIdx;
                mMesh->Faces()[faceIdx + 1].v3 = nextIdx + 1;
            }
        }

        // Generate vertices
        float angleStep = 6.2831855f / mNumSides;
        float radius = mWidth * 0.5f;
        float uStep = 1.0f / mNumSides;
        Vector3 zeroVec(0, 0, 0);

        for (int seg = 0; seg < mNumSegments; seg++) {
            for (int side = 0; side < mNumSides; side++) {
                float angle = side * angleStep;
                float u = side * uStep;
                float cosA = std::cos(angle);
                float sinA = std::sin(angle);

                for (int v = 0; v < 2; v++) {
                    int vertIdx = (seg * mNumSides + side) * 2 + v;

                    Transform xfm;
                    xfm = Transform::IDXfm();

                    float scale = (v == 0) ? 1.0f : (0.5f - u);
                    Vector3 pos(sinA * radius * scale, 0, cosA * radius * scale);
                    Multiply(pos, xfm, pos);

                    mMesh->Verts()[vertIdx].pos = pos;

                    if (v == 0) {
                        Vector3 norm;
                        Subtract(pos, zeroVec, norm);
                        Normalize(norm, norm);
                        mMesh->Verts()[vertIdx].norm = norm;
                    }

                    mMesh->Verts()[vertIdx].boneWeights.x = (float)side / mNumSegments;
                    mMesh->Verts()[vertIdx].boneWeights.y = u;
                }
            }
        }

        mMesh->Sync(0x3f);
    }
}
