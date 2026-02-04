#include "rndobj/MeshAnim.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"

#pragma region Hmx::Object

RndMeshAnim::RndMeshAnim() : mMesh(this), mKeysOwner(this, this) {}

BEGIN_HANDLERS(RndMeshAnim)
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_EXPR(num_verts, NumVerts())
    HANDLE_ACTION(shrink_verts, ShrinkVerts(_msg->Int(2)))
    HANDLE_ACTION(shrink_keys, ShrinkKeys(_msg->Int(2)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(RndMeshAnim)
    SYNC_PROP(mesh, mMesh)
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(RndMeshAnim)
    SAVE_REVS(2, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndAnimatable)
    bs << mMesh;
    bs << mVertPointsKeys;
    bs << mVertNormalsKeys;
    bs << mVertTexsKeys;
    bs << mVertColorsKeys;
    bs << mKeysOwner;
END_SAVES

BEGIN_COPYS(RndMeshAnim)
    CREATE_COPY_AS(RndMeshAnim, m)
    MILO_ASSERT(m, 0xD8);
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndAnimatable)
    COPY_MEMBER_FROM(m, mMesh)
    if (ty == kCopyShallow || (ty == kCopyFromMax && m->mKeysOwner != m)) {
        COPY_MEMBER_FROM(m, mKeysOwner)
    } else {
        MILO_ASSERT(m->mKeysOwner != this, 0xE5);
        mKeysOwner = this;
        mVertPointsKeys = m->mKeysOwner->mVertPointsKeys;
        mVertNormalsKeys = m->mKeysOwner->mVertNormalsKeys;
        mVertTexsKeys = m->mKeysOwner->mVertTexsKeys;
        mVertColorsKeys = m->mKeysOwner->mVertColorsKeys;
    }
END_COPYS

INIT_REVS(2, 0)

BEGIN_LOADS(RndMeshAnim)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    if (d.rev > 0)
        LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndAnimatable)
    d >> mMesh;
    d >> mVertPointsKeys;
    if (d.rev > 1)
        d >> mVertNormalsKeys;
    d >> mVertTexsKeys;
    d >> mVertColorsKeys;
    d >> mKeysOwner;
    if (!mKeysOwner)
        mKeysOwner = this;
END_LOADS

void RndMeshAnim::Print() {
    TheDebug << "   mesh: " << mMesh << "\n";
    TheDebug << "   keysOwner: " << mKeysOwner << "\n";
    TheDebug << "   vertPointsKeys: " << mVertPointsKeys << "\n";
    TheDebug << "   vertNormalsKeys: " << mVertNormalsKeys << "\n";
    TheDebug << "   vertTexsKeys: " << mVertTexsKeys << "\n";
    TheDebug << "   vertColorsKeys: " << mVertColorsKeys << "\n";
}

#pragma endregion
#pragma region RndAnimatable

float RndMeshAnim::EndFrame() {
    float end = VertPointsKeys().LastFrame();
    end = Max(end, VertNormalsKeys().LastFrame());
    end = Max(end, VertTexsKeys().LastFrame());
    end = Max(end, VertColorsKeys().LastFrame());
    return end;
}

#pragma endregion
#pragma region RndMeshAnim

int RndMeshAnim::NumVerts() {
    int num = 0;
    if (VertPointsKeys().size() != 0) {
        MaxEq<int>(num, VertPointsKeys().size());
    }
    if (VertNormalsKeys().size() != 0) {
        MaxEq<int>(num, VertNormalsKeys().size());
    }
    if (VertTexsKeys().size() != 0) {
        MaxEq<int>(num, VertTexsKeys().size());
    }
    if (VertColorsKeys().size() != 0) {
        MaxEq<int>(num, VertColorsKeys().size());
    }
    return num;
}

void RndMeshAnim::ShrinkKeys(int num) {
    if (VertPointsKeys().size() != 0) {
        VertPointsKeys().resize(num);
    }
    if (VertNormalsKeys().size() != 0) {
        VertNormalsKeys().resize(num);
    }
    if (VertTexsKeys().size() != 0) {
        VertTexsKeys().resize(num);
    }
    if (VertColorsKeys().size() != 0) {
        VertColorsKeys().resize(num);
    }
}
