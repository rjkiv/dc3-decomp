#include "hamobj/HamRibbon.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/File.h"
#include "rndobj/Draw.h"
#include "rndobj/Mesh.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "utl/Loader.h"

HamRibbon::HamRibbon()
    : mLastSeconds(-1), mNumSides(4), mMat(this), mWidth(1), mDirty(1), mActive(1),
      mTransList(this), mNumSegments(0), mDecay(1), mFollowA(this), mFollowB(this),
      mFollowWeight(0), mTaper(0) {
    mMesh = Hmx::Object::New<RndMesh>();
    mCreateTranss = false;
}

HamRibbon::~HamRibbon() { RELEASE(mMesh); }

BEGIN_HANDLERS(HamRibbon)
    HANDLE_ACTION(expose_mesh, ExposeMesh())
    HANDLE_ACTION(create_transs, mCreateTranss = true)
    HANDLE_ACTION(reset, Reset())
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(HamRibbon)
    SYNC_PROP_SET(active, mActive, SetActive(_val.Int()))
    SYNC_PROP_MODIFY(num_sides, mNumSides, mDirty |= 1)
    SYNC_PROP_MODIFY(num_segments, mNumSegments, mDirty |= 1)
    SYNC_PROP_MODIFY(mat, mMat, mMesh->SetMat(mMat))
    SYNC_PROP_MODIFY(width, mWidth, mDirty |= 2)
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

INIT_REVS(1, 0)

BEGIN_LOADS(HamRibbon)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndDrawable)
    d >> mNumSides;
    d >> mMat;
    d >> mActive;
    d >> mWidth;
    d >> mNumSegments;
    d >> mFollowA;
    d >> mFollowB;
    d >> mFollowWeight;
    d >> mTaper;
    if (d.rev > 0) {
        d >> mDecay;
    }
    mMesh->SetMat(mMat);
    ConstructMesh();
    mDirty = 0;
END_LOADS

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
    if (mDirty & 1) {
        ConstructMesh();
        mDirty = 0;
    }
    UpdateChase();
    mDirty = 0;
}

void HamRibbon::DrawShowing() {
    if (mActive || TheLoadMgr.EditMode()) {
        mMesh->DrawShowing();
    }
}

void HamRibbon::Reset() { mHistory.clear(); }

void HamRibbon::ExposeMesh() {
    if (!mMesh->Dir()) {
        mMesh->SetName(MakeString("%s_mesh.mesh", FileGetBase(Name())), Dir());
    }
}

void HamRibbon::SetActive(bool active) {
    if (mActive != active) {
        mHistory.clear();
        mLastSeconds = -1;
    }
    mActive = active;
}

void HamRibbon::ConstructMesh() {
    mTransList.DeleteAll();
    if (mNumSegments > 0) {
        mMesh->SetLocalXfm(Transform::GetIdentity());
        mMesh->SetNumBones(mNumSegments);
        for (int i = 0; i < mNumSegments; i++) {
            RndTransformable *t = Hmx::Object::New<RndTransformable>();
            t->SetLocalXfm(Transform::GetIdentity());
            mMesh->SetBone(i, t, true);
            mTransList.push_back(t);
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
                    xfm = Transform::GetIdentity();

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

void HamRibbon::UpdateChase() {
    if (!mFollowA) {
        return;
    }
    float seconds = TheTaskMgr.Seconds(TaskMgr::kTaskTRVideo);

    if (seconds < mLastSeconds) {
        mHistory.erase(mHistory.begin(), mHistory.end());
    }
    unsigned int numAdded = 0;

    if (mActive) {
        Vector3 back = mFollowA->WorldXfm().v;

        if (mFollowB) {
            Interp(back, mFollowB->WorldXfm().v, mFollowWeight, back);
        }
        unsigned int numExpired = 0;
        unsigned int historySize = mHistory.size();

        Key<Transform> *volatile *historyStart =
            reinterpret_cast<Key<Transform> *volatile *>(&mHistory);

        for (unsigned int i = 0; i < historySize; i++) {
            unsigned int offset = i * sizeof(Key<Transform>);
            unsigned int base = reinterpret_cast<unsigned int>(*historyStart);
            Key<Transform> *entry = reinterpret_cast<Key<Transform> *>(offset + base);

            if (entry->frame >= seconds - mDecay) {
                break;
            }
            numExpired++;
        }

        for (unsigned int i = numExpired; i < mHistory.size(); i++) {
            Key<Transform> *base = *historyStart;

            unsigned int dstOffset = (i - numExpired) * sizeof(Key<Transform>);

            Key<Transform> *dst = reinterpret_cast<Key<Transform> *>(
                dstOffset + reinterpret_cast<unsigned int>(base)
            );

            *dst = base[i];
        }
        mHistory.resize(mHistory.size() - numExpired);

        Key<Transform> key;
        key.frame = 0.0f;
        key.value = Transform::GetIdentity();

        if (mHistory.size() == 0) {
            key.frame = seconds;
            key.value.v = back;
            mHistory.push_back(key);
        } else {
            float minDistanceSq = mWidth * mWidth * 0.125f;
            float step = mDecay / (float)mNumSegments;

            while (seconds > mHistory.back().frame + step) {
                Key<Transform> *last = &*(mHistory.end() - 1);
                float lastFrame = last->frame;
                key.frame = lastFrame + step;
                Interp(last->value.v, back, step / (seconds - lastFrame), key.value.v);
                float dz = last->value.v.z - key.value.v.z;
                float dx = last->value.v.x - key.value.v.x;
                float dy = last->value.v.y - key.value.v.y;
                float distanceSq = dy * dy + (dx * dx + dz * dz);

                if (distanceSq < minDistanceSq) {
                    last->frame = key.frame;
                } else {
                    mHistory.push_back(key);
                    numAdded++;
                }
            }
        }
    }

    int start = (int)mHistory.size() - (int)numAdded;
    Vector3 bisect;
    float angle;

    for (int i = start; (unsigned int)i < mHistory.size(); i++) {
        if (i == 0) {
            continue;
        }

        Key<Transform> *volatile *orientationStart =
            reinterpret_cast<Key<Transform> *volatile *>(&mHistory);
        unsigned int offset = (unsigned int)i * sizeof(Key<Transform>);
        unsigned int base = reinterpret_cast<unsigned int>(*orientationStart);
        Key<Transform> *cur = reinterpret_cast<Key<Transform> *>(offset + base);
        Key<Transform> *prev = cur - 1;
        Vector3 &currentPos = cur->value.v;
        const Vector3 &previousPos = prev->value.v;
        Vector3 dir;
        dir.x = currentPos.x - previousPos.x;
        dir.y = currentPos.y - previousPos.y;
        dir.z = currentPos.z - previousPos.z;
        Normalize(dir, dir);
        angle = -1.0f;

        if (i > 2) {
            const Vector3 &olderPos = (cur - 2)->value.v;
            Vector3 lastDir;
            lastDir.x = previousPos.x - olderPos.x;
            lastDir.y = previousPos.y - olderPos.y;
            lastDir.z = previousPos.z - olderPos.z;
            float dot = lastDir.x * dir.x;
            dot += lastDir.z * dir.z;
            dot += lastDir.y * dir.y;
            angle = acosf(Clamp(0.0f, 1.0f, dot));
            Vector3 back = lastDir;
            Scale(back, -1.0f, back);
            Interp(dir, back, 0.5f, bisect);
            Normalize(bisect, bisect);
        }
        static Vector3 sUp(0.0f, 0.0f, 1.0f);
        Transform localXfm;
        Invert(prev->value, localXfm);
        Vector3 toLocal;
        Multiply(currentPos, localXfm, toLocal);
        Transform lookAt = Transform::GetIdentity();
        lookAt.LookAt(toLocal, sUp);
        Transform t;
        Multiply(lookAt, prev->value.m, t);
        Normalize(t.m, t.m);
        t.v = currentPos;

        if (angle != -1.0f) {
            Hmx::Matrix3 inv;
            Invert(t.m, inv);
            float bx = bisect.x;
            float by = bisect.y;
            float bz = bisect.z;
            const volatile Hmx::Matrix3 &vInv = inv;
            float localX = vInv.y.x * by;
            float localZ = by * vInv.y.z;
            float localY = vInv.y.y * by;
            localX += inv.x.x * bx;
            localZ += inv.x.z * bx;
            localY += inv.x.y * bx;
            localX += inv.z.x * bz;
            localZ += bz * vInv.z.z;
            localY += vInv.z.y * bz;

            bisect.z = localZ;
            bisect.y = localY;
            bisect.x = Clamp(0.0f, 1.0f, localX);

            float localAngle = acosf(bisect.x);
            float invCos = 1.0f / cosf(angle * 0.5f);
            float doubledAngle = localAngle * 2.0f;
            float c = cosf(doubledAngle);

            Hmx::Matrix3 skew;
            skew.x.z = sinf(doubledAngle) * (1.0f - invCos) * 0.5f;
            skew.x.y = 0.0f;
            skew.y.x = 0.0f;
            skew.y.y = 1.0f;
            skew.y.z = 0.0f;
            skew.z.y = 0.0f;
            skew.z.x = skew.x.z;
            float correction = invCos - 1.0f;
            skew.x.x = (c + 1.0f) * correction * 0.5f + 1.0f;
            skew.z.z = (1.0f - c) * correction * 0.5f + 1.0f;
            Multiply(skew, t.m, t.m);
        }
        cur->value.m = t.m;
    }
    UpdateMesh();
    mLastSeconds = seconds;
}
