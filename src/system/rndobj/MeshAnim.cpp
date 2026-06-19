#include "rndobj/MeshAnim.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Anim.h"

#pragma region Hmx::Object

RndMeshAnim::RndMeshAnim() : mMesh(this), mKeysOwner(this, this) {}

bool RndMeshAnim::Replace(ObjRef *from, Hmx::Object *to) {
    if (&mKeysOwner == from) {
        if (mKeysOwner == this) {
            mKeysOwner = this;
        } else {
            RndMeshAnim *meshTo = dynamic_cast<RndMeshAnim *>(to);
            if (meshTo) {
                mKeysOwner = meshTo->KeysOwner();
            } else {
                mKeysOwner = this;
            }
        }
        return true;
    } else {
        return Hmx::Object::Replace(from, to);
    }
}

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
        mKeysOwner = m->mKeysOwner.Ptr();
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

struct GetVertPoint {
    Vector3 &get(RndMesh::Vert *v) { return v->pos; }
};
struct GetVertNormal {
    Vector3 &get(RndMesh::Vert *v) { return v->norm; }
};
struct GetVertTex {
    Vector2 &get(RndMesh::Vert *v) { return v->tex; }
};
struct GetVertColor {
    Hmx::Color &get(RndMesh::Vert *v) { return v->color; }
};

template <class T1, class T2>
void InterpVertData(
    const std::vector<T1> &a,
    const std::vector<T1> &b,
    float ref,
    RndMesh::VertVector &v,
    float blend
) {
    MILO_ASSERT(a.size() == b.size(), 0x135);
    auto aIt = a.begin();
    auto bIt = b.begin();
    auto vIt = v.begin();
    auto itEnd = a.end();
    T2 getter;
    if (a.size() > v.size()) {
        itEnd = itEnd - (a.size() - v.size());
    }

    if (ref == 0) {
        if (blend != 1) {
            for (; aIt != itEnd; ++aIt) {
                Interp(getter.get(vIt), *aIt, blend, getter.get(vIt));
                ++vIt;
            }
        } else {
            for (; aIt != itEnd; ++aIt) {
                getter.get(vIt) = *aIt;
                ++vIt;
            }
        }
    } else if (ref == 1) {
        if (blend != 1) {
            for (; aIt != itEnd; ++aIt) {
                Interp(getter.get(vIt), *bIt, blend, getter.get(vIt));
                ++bIt;
                ++vIt;
            }
        } else {
            for (; aIt != itEnd; ++aIt) {
                getter.get(vIt) = *bIt;
                ++bIt;
                ++vIt;
            }
        }
    } else if (blend != 1) {
        for (; aIt != itEnd; ++aIt) {
            T1 tmp;
            Interp(*aIt, *bIt, ref, tmp);
            Interp(getter.get(vIt), tmp, blend, getter.get(vIt));
            ++bIt;
            ++vIt;
        }
    } else {
        for (; aIt != itEnd; ++aIt) {
            Interp(*aIt, *bIt, ref, getter.get(vIt));
            ++bIt;
            ++vIt;
        }
    }
}

void RndMeshAnim::SetFrame(float frame, float blend) {
    RndAnimatable::SetFrame(frame, blend);
    if (mMesh) {
        if (!(mMesh->Mutable() & 0x1F)) {
            MILO_NOTIFY_ONCE("Mesh %s is animated but not mutable.\n", mMesh->Name());
        } else {
            int flags = 0;
            if (!VertPointsKeys().empty()) {
                const Key<std::vector<Vector3> > *prev;
                const Key<std::vector<Vector3> > *next;
                float ref = 0;
                VertPointsKeys().AtFrame(frame, prev, next, ref);
                InterpVertData<Vector3, GetVertPoint>(
                    prev->value, next->value, ref, mMesh->Verts(), blend
                );
                flags |= 0x1F;
            }
            if (!VertNormalsKeys().empty()) {
                const Key<std::vector<Vector3> > *prev;
                const Key<std::vector<Vector3> > *next;
                float ref = 0;
                VertNormalsKeys().AtFrame(frame, prev, next, ref);
                InterpVertData<Vector3, GetVertNormal>(
                    prev->value, next->value, ref, mMesh->Verts(), blend
                );
                flags |= 0x1F;
            }
            if (!VertTexsKeys().empty()) {
                const Key<std::vector<Vector2> > *prev;
                const Key<std::vector<Vector2> > *next;
                float ref = 0;
                VertTexsKeys().AtFrame(frame, prev, next, ref);
                InterpVertData<Vector2, GetVertTex>(
                    prev->value, next->value, ref, mMesh->Verts(), blend
                );
                flags |= 0x1F;
            }
            if (!VertColorsKeys().empty()) {
                const Key<std::vector<Hmx::Color> > *prev;
                const Key<std::vector<Hmx::Color> > *next;
                float ref = 0;
                VertColorsKeys().AtFrame(frame, prev, next, ref);
                InterpVertData<Hmx::Color, GetVertColor>(
                    prev->value, next->value, ref, mMesh->Verts(), blend
                );
                flags |= 0x1F;
            }
            if (flags) {
                mMesh->Sync(flags);
            }
        }
    }
}

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

void RndMeshAnim::ShrinkVerts(int num) {
    FOREACH (it, VertPointsKeys()) {
        it->value.resize(num);
    }
    FOREACH (it, VertNormalsKeys()) {
        it->value.resize(num);
    }
    FOREACH (it, VertTexsKeys()) {
        it->value.resize(num);
    }
    FOREACH (it, VertColorsKeys()) {
        it->value.resize(num);
    }
}

void fakemeshanimlmao(std::vector<Hmx::Color> &c) { c.push_back(Hmx::Color(1, 1, 1)); }
