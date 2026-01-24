#include "rndobj/Mesh.h"
#include "Utl.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/BaseMaterial.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/MultiMesh.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"
#include "utl/Std.h"

PatchVerts gPatchVerts;
int MESH_REV_SEP_COLOR = 0x25;

RndMesh::RndMesh()
    : mMat(this), mGeomOwner(this, this), mBones(this), mMutable(0),
      mVolume(kVolumeTriangles), mBSPTree(nullptr), mMultiMesh(nullptr), mHasAOCalc(0),
      mKeepMeshData(0), mCompressedVerts(nullptr), mNumCompressedVerts(0) {
    unk180 = 0x26;
}

RndMesh::~RndMesh() {
    RELEASE(mBSPTree);
    RELEASE(mMultiMesh);
    ClearCompressedVerts();
}

BEGIN_HANDLERS(RndMesh)
    HANDLE(compare_edge_verts, OnCompareEdgeVerts)
    HANDLE(attach_mesh, OnAttachMesh)
    HANDLE(get_face, OnGetFace)
    HANDLE(set_face, OnSetFace)
    HANDLE(get_vert_pos, OnGetVertXYZ)
    HANDLE(set_vert_pos, OnSetVertXYZ)
    HANDLE(get_vert_norm, OnGetVertNorm)
    HANDLE(set_vert_norm, OnSetVertNorm)
    HANDLE(get_vert_uv, OnGetVertUV)
    HANDLE(set_vert_uv, OnSetVertUV)
    HANDLE(unitize_normals, OnUnitizeNormals)
    HANDLE(build_from_bsp, OnBuildFromBSP)
    HANDLE(point_collide, OnPointCollide)
    HANDLE(configure_mesh, OnConfigureMesh)
    HANDLE_EXPR(estimated_size_kb, EstimatedSizeKb())
    HANDLE_ACTION(instance_bones, InstanceGeomOwnerBones())
    HANDLE_EXPR(has_instanced_bones, HasInstancedBones())
    HANDLE_EXPR(has_bones, !mBones.empty())
    HANDLE_ACTION(delete_bones, DeleteBones(_msg->Int(2)))
    HANDLE_ACTION(burn_xfm, BurnXfm())
    HANDLE_ACTION(reset_normals, ResetNormals())
    HANDLE_ACTION(tessellate, Tessellate())
    HANDLE_ACTION(clear_ao, ClearAO())
    HANDLE_ACTION(clear_bones, CopyBones(nullptr))
    HANDLE_ACTION(copy_geom_from_owner, CopyGeometryFromOwner())
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

bool RndMesh::HasInstancedBones() {
    return mGeomOwner && !mBones.empty() && mGeomOwner->mBones.Owner() == mBones.Owner();
}

BEGIN_CUSTOM_PROPSYNC(RndMesh::Vert)
    SYNC_PROP(pos, o.pos)
    SYNC_PROP(norm, o.norm)
    SYNC_PROP(color, o.color)
    SYNC_PROP(alpha, o.color.alpha)
    SYNC_PROP(tex, o.tex)
END_CUSTOM_PROPSYNC

BEGIN_CUSTOM_PROPSYNC(RndBone)
    SYNC_PROP(bone, o.mBone)
    SYNC_PROP(offset, o.mOffset)
END_CUSTOM_PROPSYNC

bool PropSync(
    RndMesh ::VertVector &vec, DataNode &node, DataArray *prop, int i, PropOp op
) {
    if (op == kPropUnknown0x40)
        return false;
    else if (i == prop->Size()) {
        MILO_ASSERT(op == kPropSize, 0xA7D);
        node = (int)vec.size();
        return true;
    } else {
        RndMesh::Vert &vert = vec[prop->Int(i++)];
        if (i < prop->Size() || op & (kPropGet | kPropSet | kPropSize)) {
            if (vec.size() > 0) {
                if (op & kPropSet) {
                    vec.unkc = true;
                }
                return PropSync(vert, node, prop, i, op);
            } else {
                MILO_NOTIFY_ONCE(
                    "Cannot modify verts (check if keep_mesh_data is set on the mesh)"
                );
            }
            return true;
        } else {
            MILO_NOTIFY("Cannot add or remove verts of a mesh via property system");
        }
        return true;
    }
}

BEGIN_PROPSYNCS(RndMesh)
    SYNC_PROP(mat, mMat)
    SYNC_PROP_MODIFY(geom_owner, mGeomOwner, if (!mGeomOwner) mGeomOwner = this)
    SYNC_PROP_BITFIELD(mutable, mGeomOwner->mMutable, 0xAA1)
    SYNC_PROP_SET(num_verts, Verts().size(), SetNumVerts(_val.Int()))
    SYNC_PROP_SET(num_faces, (int)Faces().size(), SetNumFaces(_val.Int()))
    SYNC_PROP_SET(volume, GetVolume(), SetVolume((Volume)_val.Int()))
    SYNC_PROP_SET(has_valid_bones, HasValidBones(nullptr), _val.Int())
    SYNC_PROP(bones, mBones)
    SYNC_PROP(has_ao_calculation, mHasAOCalc)
    SYNC_PROP_SET(keep_mesh_data, mKeepMeshData, SetKeepMeshData(_val.Int() > 0))
    SYNC_PROP(verts, Verts())
    SYNC_SUPERCLASS(RndTransformable)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const RndMesh::Face &face) {
    bs << face.v1 << face.v2 << face.v3;
    return bs;
}

BinStream &operator<<(BinStream &bs, const RndMesh::Vert &vert) {
    bs << vert.pos << vert.norm;
    bs << vert.color << vert.tex << vert.boneWeights << vert.boneIndices[0]
       << vert.boneIndices[1] << vert.boneIndices[2] << vert.boneIndices[3] << vert.unk50;
    return bs;
}

BinStream &operator<<(BinStream &bs, const RndBone &bone) {
    bs << bone.mBone << bone.mOffset;
    return bs;
}

BEGIN_SAVES(RndMesh)
    SAVE_REVS(0x26, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndTransformable)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mMat;
    bs << mGeomOwner << mMutable << mVolume << mBSPTree;
    SaveVertices(bs);
    bs << mFaces << mPatches << mBones;
    bs << mKeepMeshData;
    bs << mHasAOCalc;
END_SAVES

void RndMesh::CopyBones(const RndMesh *mesh) {
    if (mesh)
        mBones = mesh->mBones;
    else
        mBones.clear();
}

BEGIN_COPYS(RndMesh)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndTransformable)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(RndMesh)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMat)
        if (ty != kCopyFromMax)
            COPY_MEMBER(mKeepMeshData)
        if (ty == kCopyFromMax)
            mMutable |= c->mMutable;
        else
            COPY_MEMBER(mMutable)
        mHasAOCalc = false;
        if (ty == kCopyShallow || (ty == kCopyFromMax && c->mGeomOwner != c)) {
            mGeomOwner = c->mGeomOwner.Ptr();
            CopyBones(c);
        } else {
            CopyGeometry(c, ty != kCopyFromMax);
            if (ty != kCopyFromMax)
                COPY_MEMBER(mHasAOCalc);
        }
    END_COPYING_MEMBERS
    Sync(0xBF);
END_COPYS

BinStreamRev &operator>>(BinStreamRev &d, RndMesh::Vert &vert) {
    d.stream >> vert.pos;
    float y, z;
    if (d.rev != 10 && d.rev < 0x17) {
        d.stream >> y;
        d.stream >> z;
    }
    d.stream >> vert.norm;
    if (d.rev < MESH_REV_SEP_COLOR)
        d.stream >> vert.color;
    else
        d.stream >> vert.color;
    d.stream >> vert.tex;
    if (d.rev >= MESH_REV_SEP_COLOR) {
        d.stream >> vert.boneWeights;
    }
    if (d.rev != 10 && d.rev < 23) {
        vert.boneWeights.Set((1.0f - y) - z, y, z, 0);
    }
    if (d.rev < 0xB) {
        Vector2 v;
        d.stream >> v;
    }
    if (d.rev > 0x1C) {
        d.stream >> vert.boneIndices[0];
        d.stream >> vert.boneIndices[1];
        d.stream >> vert.boneIndices[2];
        d.stream >> vert.boneIndices[3];
    }
    if (d.rev > 0x1D) {
        d.stream >> vert.unk50;
    }
    return d;
}

BinStream &operator>>(BinStreamRev &d, RndMesh::Face &face) {
    d.stream >> face.v1;
    d.stream >> face.v2;
    d.stream >> face.v3;
    if (d.rev < 1) {
        Vector3 v;
        d.stream >> v;
    }
    return d.stream;
}

BinStream &operator>>(BinStream &bs, RndBone &bone) {
    bs >> bone.mBone;
    bs >> bone.mOffset;
    return bs;
}

template <class T1, class T2>
BinStream &CachedRead(BinStream &, std::vector<T1, T2> &);

BEGIN_LOADS(RndMesh)
    LOAD_REVS(bs)
    ASSERT_REVS(0x26, 0)
    if (d.rev > 0x19) {
        Hmx::Object::Load(d.stream);
    }
    RndTransformable::Load(d.stream);
    RndDrawable::Load(d.stream);
    if (d.rev < 15) {
        ObjPtrList<Hmx::Object> oList(this);
        int dummy;
        d.stream >> dummy;
        d.stream >> oList;
    }
    int i22 = 0;
    if (d.rev < 0x14) {
        int ib8, ie8;
        d.stream >> ib8;
        d.stream >> ie8;
        if (ib8 == 0 || ie8 == 0) {
            i22 = 0;
        } else if (ib8 == 1) {
            i22 = 2;
        } else if (ie8 == 7) {
            i22 = 3;
        } else {
            i22 = 1;
        }
    }
    if (d.rev < 3) {
        int dummy;
        d.stream >> dummy;
    }
    d.stream >> mMat;
    if (d.rev > 0x1A && d.rev < 0x1C) {
        char buf[0x80];
        d.stream.ReadString(buf, 0x80);
        if (!mMat && buf[0] != '\0') {
            mMat = LookupOrCreateMat(buf, Dir());
        }
    }
    d.stream >> mGeomOwner;
    if (!mGeomOwner) {
        mGeomOwner = this;
    }
    if (d.rev < 0x14 && mMat && (i22 == 0 || mMat->GetZMode() != kZModeDisable)) {
        mMat->SetZMode((ZMode)i22);
    }
    if (d.rev < 0xD) {
        ObjOwnerPtr<RndMesh> mesh(this);
        d.stream >> mesh;
        if (mesh != mGeomOwner) {
            MILO_NOTIFY("Combining face and vert owner of %s", Name());
        }
    }
    if (d.rev < 0xF) {
        ObjPtr<RndTransformable> trans(this);
        d.stream >> trans;
        SetTransParent(trans, false);
        SetTransConstraint((Constraint)2, nullptr, false);
    }
    if (d.rev < 0xE) {
        ObjPtr<RndTransformable> trans1(this);
        ObjPtr<RndTransformable> trans2(this);
        d.stream >> trans1 >> trans2;
    }
    if (d.rev < 3) {
        Vector3 v;
        d.stream >> v;
    }
    if (d.rev < 0xF) {
        Sphere s;
        d.stream >> s;
        SetSphere(s);
    }
    if (d.rev > 4 && d.rev < 8) {
        bool b;
        d >> b;
    }
    if (d.rev > 5 && d.rev < 0x15) {
        String str;
        int x;
        d.stream >> str;
        d.stream >> x;
    }
    if (d.rev > 0xF) {
        d.stream >> mMutable;
    } else if (d.rev > 0xB) {
        bool b;
        d >> b;
        mMutable = b ? 31 : 0;
    }
    if (d.rev > 0x11) {
        d.stream >> (int &)mVolume;
    }
    if (d.rev > 0x12) {
        RELEASE(mBSPTree);
        d.stream >> mBSPTree;
    }
    if (d.rev > 6 && d.rev < 8) {
        bool b;
        d >> b;
    }
    if (d.rev > 8 && d.rev < 0xB) {
        int x;
        d.stream >> x;
    }
    LoadVertices(d);
    if (d.stream.Cached()) {
        CachedRead(d.stream, mFaces);
    } else {
        d >> mFaces;
    }
    if (d.rev > 4 && d.rev < 0x18) {
        int count;
        unsigned short s1, s2;
        d.stream >> count;
        for (; count != 0; count--) {
            d.stream >> s1;
            d.stream >> s2;
        }
    }
    if (d.rev > 0x17) {
        if (d.stream.Cached()) {
            CachedRead(d.stream, mPatches);
        } else {
            d >> mPatches;
        }
    } else if (d.rev > 0x15) {
        mPatches.clear();
        int count;
        unsigned int ui;
        bs >> count;
        for (; count != 0; count--) {
            std::vector<unsigned short> usvec;
            std::vector<unsigned int> uivec;
            d >> ui >> usvec >> uivec;
            mPatches.push_back(ui);
        }
    } else if (d.rev > 0x10)
        d >> mPatches;
    if (d.rev > 0x1C) {
        d >> mBones;
        int max = MaxBones();
        if (mBones.size() > max) {
            MILO_NOTIFY(
                "%s: exceeds bone limit (%d of %d)",
                PathName(this),
                mBones.size(),
                MaxBones()
            );
            mBones.resize(MaxBones());
        }
    } else if (d.rev > 0xD) {
        ObjPtr<RndTransformable> trans(this);
        d.stream >> trans;
        if (trans) {
            mBones.resize(4);
            if (d.rev > 0x16) {
                mBones[0].mBone = trans;
                bs >> mBones[1].mBone >> mBones[2].mBone >> mBones[3].mBone;
                bs >> mBones[0].mOffset >> mBones[1].mOffset >> mBones[2].mOffset
                    >> mBones[3].mOffset;
                if (d.rev < 0x19) {
                    for (Vert *it = mVerts.begin(); it != mVerts.end(); ++it) {
                        it->boneWeights.Set(
                            ((1.0f - it->boneWeights.x) - it->boneWeights.y)
                                - it->boneWeights.z,
                            it->boneWeights.x,
                            it->boneWeights.y,
                            it->boneWeights.z
                        );
                    }
                }
            } else {
                if (TransConstraint() == RndTransformable::kConstraintParentWorld) {
                    ObjPtr<RndTransformable> &bone = mBones[0].mBone;
                    bone = TransParent();
                } else {
                    mBones[0].mBone = this;
                }
                mBones[0].mOffset.Reset();
                mBones[1].mBone = trans;
                bs >> mBones[2].mBone >> mBones[1].mOffset >> mBones[2].mOffset;
                mBones[3].mBone = nullptr;
            }
            for (int i = 0; i < 4; i++) {
                if (!mBones[i].mBone) {
                    mBones.resize(i);
                    break;
                }
            }
        } else
            mBones.clear();
    }
    RemoveInvalidBones();
    if (d.rev > 0 && d.rev < 4) {
        std::vector<std::vector<unsigned short> > usvec;
        d >> usvec;
    }
    if (d.rev == 0) {
        bool bd4;
        int ic0, ic4, ic8, icc;
        d >> bd4 >> ic0 >> ic4 >> ic8;
        d >> icc;
    }
    if (d.rev == 0x12) {
        if (mGeomOwner == this) {
            SetVolume(mVolume);
            goto yes;
        }
    } else {
    yes:
        if (d.rev >= 0x1E)
            goto next;
    }
    if (mMat && mMat->NormalMap()) {
        MakeTangentsLate(this);
    }
next:
    if (d.rev < 0x1F) {
        SetZeroWeightBones();
    }
    if (d.rev > 0x23) {
        d >> mKeepMeshData;
    }
    if (d.rev < MESH_REV_SEP_COLOR && IsSkinned()) {
        for (Vert *it = mVerts.begin(); it != mVerts.end(); ++it) {
            it->boneWeights.Set(
                it->color.red, it->color.green, it->color.blue, it->color.alpha
            );
            it->color.Zero();
        }
    }
    if (d.rev > 0x25) {
        d >> mHasAOCalc;
    }
    Sync(0xBF);
END_LOADS

TextStream &operator<<(TextStream &ts, RndMesh::Volume v) {
    if (v == RndMesh::kVolumeEmpty)
        ts << "Empty";
    else if (v == RndMesh::kVolumeTriangles)
        ts << "Triangles";
    else if (v == RndMesh::kVolumeBSP)
        ts << "BSP";
    else if (v == RndMesh::kVolumeBox)
        ts << "Box";
    return ts;
}

void RndMesh::Print() {
    TheDebug << "   mat: " << mMat << "\n";
    TheDebug << "   geomOwner: " << mGeomOwner << "\n";
    TheDebug << "   mutable: " << mMutable << "\n";
    TheDebug << "   volume: " << mVolume << "\n";
    TheDebug << "   bones: TODO\n";
    TheDebug << "   geometry: TODO\n";
}

void RndMesh::UpdateSphere() {
    Sphere s;
    if (mBones.empty()) {
        MakeWorldSphere(s, true);
        Transform tf;
        FastInvert(WorldXfm(), tf);
        Multiply(s, tf, s);
    } else
        s.Zero();
    RndDrawable::SetSphere(s);
}

float RndMesh::GetDistanceToPlane(const Plane &p, Vector3 &v) {
    if (Verts().empty())
        return 0;
    else {
        const Transform &world = WorldXfm();
        Vector3 v58;
        Multiply(Verts()[0].pos, world, v58);
        v = v58;
        float dot = p.Dot(v);
        for (Vert *it = Verts().begin(); it != Verts().end(); ++it) {
            Multiply(it->pos, world, v58);
            float dotted = p.Dot(v58);
            if (std::fabs(dotted) < std::fabs(dot)) {
                dot = dotted;
                v = v58;
            }
        }
        return dot;
    }
}

bool RndMesh::MakeWorldSphere(Sphere &s, bool b) {
    if (b) {
        if (mShowing) {
            Box box;
            CalcBox(this, box);
            Vector3 v68;
            CalcBoxCenter(v68, box);
            s.Set(v68, 0);
            const Transform &worldXfm = WorldXfm();
            for (Vert *it = Verts().begin(); it != Verts().end(); ++it) {
                Vector3 v50;
                Multiply(it->pos, worldXfm, v50);
                Vector3 v5c;
                Subtract(v50, s.center, v5c);
                s.radius = Max(s.GetRadius(), Dot(v5c, v5c));
            }
            s.radius = sqrtf(s.GetRadius());
            return true;
        }
    } else if (mSphere.GetRadius()) {
        Multiply(mSphere, WorldXfm(), s);
        return true;
    }
    return false;
}

void RndMesh::Mats(std::list<RndMat *> &mats, bool) {
    if (mMat) {
        mMat->SetShaderOpts(GetDefaultMatShaderOpts(this, mMat));
        mats.push_back(mMat);
    }
}

// RndDrawable *RndMesh::CollideShowing(const Segment &seg, float &f, Plane &pl) {
//     Segment sega0;
//     Transform tf58;
//     sLastCollide = -1;
//     if (IsSkinned() || sRawCollide)
//         sega0 = seg;
//     else {
//         FastInvert(WorldXfm(), tf58);
//         Multiply(seg.start, tf58, sega0.start);
//         Multiply(seg.end, tf58, sega0.end);
//     }
//     if (mGeomOwner->mBSPTree) {
//         if (Intersect(sega0, mGeomOwner->mBSPTree, f, pl) && f) {
//             Multiply(pl, WorldXfm(), pl);
//             return this;
//         }
//     } else {
//         if (GetVolume() == kVolumeTriangles) {
//             bool b1 = false;
//             f = 1.0f;
//             FOREACH (it, Faces()) {
//                 const Vert &vert0 = Verts(it->v1);
//                 const Vert &vert1 = Verts(it->v2);
//                 const Vert &vert2 = Verts(it->v3);
//                 Triangle tri;
//                 if (IsSkinned() && !sRawCollide) {
//                     tri.Set(
//                         SkinVertex(vert0, nullptr),
//                         SkinVertex(vert1, nullptr),
//                         SkinVertex(vert2, nullptr)
//                     );
//                 } else
//                     tri.Set(vert0.pos, vert1.pos, vert2.pos);
//                 float fintersect;
//                 if (Intersect(sega0, tri, false, fintersect)) {
//                     Interp(sega0.start, sega0.end, fintersect, sega0.end);
//                     f *= fintersect;
//                     pl.Set(tri.origin, tri.frame.z);
//                     b1 = true;
//                     sLastCollide = (it - Faces().begin());
//                 }
//             }
//             if (b1) {
//                 if (!sRawCollide)
//                     Multiply(pl, WorldXfm(), pl);
//                 return this;
//             }
//         }
//     }
//     return 0;
// }

int RndMesh::CollidePlane(const Plane &plane) {
    int super = RndDrawable::CollidePlane(plane);
    if (super != 0)
        return super;
    Transform tf48;
    FastInvert(WorldXfm(), tf48);
    Plane pl58;
    Multiply(plane, tf48, pl58);
    if (GetVolume() == kVolumeTriangles) {
        if (Faces().empty())
            return -1;
        else {
            std::vector<Face>::iterator faceIt = Faces().begin();
            int faceColl = CollidePlane(*faceIt, pl58);
            if (faceColl == 0)
                return 0;
            else {
                ++faceIt;
                for (; faceIt != Faces().end(); ++faceIt) {
                    if (faceColl != CollidePlane(*faceIt, pl58)) {
                        return 0;
                    }
                }
                return faceColl;
            }
        }
    } else
        return 0;
}

void RndMesh::VertVector::operator=(const RndMesh::VertVector &c) {
    MILO_ASSERT(mCapacity == 0, 0x26D);
    MILO_ASSERT(c.mCapacity == 0, 0x26E);
    if (c.mNumVerts != mNumVerts) {
        delete mVerts;
        mNumVerts = c.mNumVerts;
        mVerts = new Vert[mNumVerts];
    }
    Vert *otherVerts = c.mVerts;
    Vert *otherEnd = &otherVerts[mNumVerts];
    Vert *myVerts = mVerts;
    while (otherVerts != otherEnd) {
        *myVerts++ = *otherVerts++;
    }
}

void RndMesh::VertVector::resize(int n) {
    if (mCapacity > 0) {
        MILO_ASSERT(n <= mCapacity, 0x227);
        mNumVerts = n;
    } else if (n == 0) {
        delete[] mVerts;
        mVerts = nullptr;
        mNumVerts = 0;
    } else if (n != mNumVerts) {
        Vert *oldverts = mVerts;
        Vert *oldit = oldverts;
        Vert *end = oldverts + Min(n, size());
        mVerts = new Vert[n];
        mNumVerts = n;

        Vert *newit = mVerts;
        while (oldit != end) {
            *newit++ = *oldit++;
        }

        delete[] oldverts;
    }
}

int RndMesh::EstimatedSizeKb() const {
    // sizeof(Vert) is 0x50 here
    // but the actual struct is size 0x60
    return (NumVerts() * 0x50 + NumFaces() * (int)sizeof(Face)) / 1024;
}

void RndMesh::ClearCompressedVerts() {
    RELEASE(mCompressedVerts);
    mNumCompressedVerts = 0;
}

void RndMesh::SetMat(RndMat *mat) { mMat = mat; }

void RndMesh::SetGeomOwner(RndMesh *m) {
    MILO_ASSERT(m, 0x1D7);
    mGeomOwner = m;
}

void RndMesh::SetKeepMeshData(bool keep) {
    if (keep != mKeepMeshData) {
        mKeepMeshData = keep;
        if (!mKeepMeshData) {
            mVerts.resize(0);
            ClearAndShrink(mFaces);
            ClearAndShrink(mPatches);
        }
    }
}

void RndMesh::SetNumVerts(int num) {
    Verts().resize(num);
    Sync(0x3F);
}

void RndMesh::SetNumFaces(int num) {
    mGeomOwner->mFaces.resize(num);
    Sync(0x3F);
}

void RndMesh::SetNumBones(int num) {
    if (num > MaxBones()) {
        MILO_NOTIFY("%s: exceeds bone limit of %d", PathName(this), MaxBones());
    }
    mBones.resize(num);
}

void RndMesh::ScaleBones(float f) {
    for (ObjVector<RndBone>::iterator it = mBones.begin(); it != mBones.end(); ++it) {
        it->mOffset.v *= f;
    }
}

void RndMesh::SetZeroWeightBones() {
    if (mBones.size() >= 2) {
        for (int i = 0; i < mVerts.size(); i++) {
            Vert &curvert = mVerts[i];
            if (curvert.boneWeights.y == 0)
                curvert.boneIndices[1] = curvert.boneIndices[0];
            if (curvert.boneWeights.z == 0)
                curvert.boneIndices[2] = curvert.boneIndices[0];
            if (curvert.boneWeights.w == 0)
                curvert.boneIndices[3] = curvert.boneIndices[0];
        }
    }
}

void RndMesh::ResetNormals() {
    if (mGeomOwner != this) {
        MILO_NOTIFY("Must be geom owner to reset normals");
    } else {
        ::ResetNormals(this);
    }
}

void RndMesh::Tessellate() {
    if (mGeomOwner != this) {
        MILO_NOTIFY("Must be geom owner to tessellate");
    } else {
        TessellateMesh(this);
    }
}

void RndMesh::ClearAO() {
    if (mGeomOwner != this) {
        MILO_NOTIFY("Must be geom owner to clear AO");
    } else {
        ::ClearAO(this);
    }
}

int RndMesh::GetBoneIndex(const RndTransformable *t) {
    for (int i = 0; i < mBones.size(); i++) {
        if (mBones[i].mBone == t) {
            return i;
        }
    }
    return -1;
}

void RndMesh::Sync(int flags) {
    if (mKeepMeshData) {
        flags |= 0x200;
    }
    OnSync(flags);
    if (flags | 0x1F) {
        Verts().unkc = 0;
    }
}

bool RndMesh::HasValidBones(unsigned int *boneIdx) const {
    int idx = 0;
    for (ObjVector<RndBone>::const_iterator it = mBones.begin(); it != mBones.end();
         ++it, ++idx) {
        if (!it->mBone) {
            if (boneIdx)
                *boneIdx = idx;
            return false;
        }
    }
    if (boneIdx)
        *boneIdx = mBones.size();
    return true;
}

void RndMesh::BurnXfm() {
    if (mGeomOwner != this) {
        MILO_NOTIFY("Must be geom owner to burn xfm");
    } else {
        for (std::list<RndTransformable *>::iterator it = mChildren.begin();
             it != mChildren.end();
             ++it) {
            Transform xfm;
            Multiply((*it)->LocalXfm(), mLocalXfm, xfm);
            (*it)->SetLocalXfm(xfm);
        }
        ::BurnXfm(this, false);
    }
}

void RndMesh::SetBone(int idx, RndTransformable *bone, bool b3) {
    mBones[idx].mBone = bone;
    if (b3) {
        Transform xfm;
        Invert(bone->WorldXfm(), xfm);
        Multiply(WorldXfm(), xfm, mBones[idx].mOffset);
    }
}

RndMultiMesh *RndMesh::CreateMultiMesh() {
    RndMesh *owner = mGeomOwner;
    if (!owner->mMultiMesh) {
        owner->mMultiMesh = Hmx::Object::New<RndMultiMesh>();
        owner->mMultiMesh->SetMesh(owner);
    }
    owner->mMultiMesh->Instances().resize(0);
    return owner->mMultiMesh;
}

void RndMesh::RemoveInvalidBones() {
    for (ObjVector<RndBone>::iterator it = mBones.begin(); it != mBones.end();) {
        if (it->mBone)
            ++it;
        else
            it = mBones.erase(it);
    }
}

void RndMesh::CopyGeometryFromOwner() {
    if (mGeomOwner != this) {
        CopyGeometry(mGeomOwner, true);
        Sync(0x3F);
    }
}

void RndMesh::CopyGeometry(const RndMesh *mesh, bool b2) {
    mGeomOwner = this;
    mVerts = mesh->mGeomOwner->mVerts;
    mFaces = mesh->mGeomOwner->mFaces;
    mPatches = mesh->mGeomOwner->mPatches;
    if (b2)
        SetVolume(mesh->mGeomOwner->mVolume);
    mBones = mesh->mBones;
}

DataNode RndMesh::OnPointCollide(const DataArray *da) {
    BSPNode *tree = GetBSPTree();
    Vector3 v(da->Float(2), da->Float(3), da->Float(4));
    MultiplyTranspose(v, WorldXfm(), v);
    return tree && Intersect(v, tree);
}

DataNode RndMesh::OnBuildFromBSP(const DataArray *a) {
    BuildFromBSP(this);
    return 0;
}

DataNode RndMesh::OnAttachMesh(const DataArray *da) {
    RndMesh *m = da->Obj<RndMesh>(2);
    AttachMesh(this, m);
    delete m;
    return 0;
}

DataNode RndMesh::OnGetVertNorm(const DataArray *da) {
    Vert *v;
    s32 index = da->Int(2);
    MILO_ASSERT(index >= 0 && index < mVerts.size(), 0x858);
    v = &mVerts[index];
    *da->Var(3) = v->norm.x;
    *da->Var(4) = v->norm.y;
    *da->Var(5) = v->norm.z;
    return 0;
}

DataNode RndMesh::OnSetVertNorm(const DataArray *da) {
    Vert *v;
    s32 index = da->Int(2);
    MILO_ASSERT(index >= 0 && index < mVerts.size(), 0x863);
    v = &mVerts[index];
    v->norm.x = da->Float(3);
    v->norm.y = da->Float(4);
    v->norm.z = da->Float(5);
    Sync(31);
    return 0;
}

DataNode RndMesh::OnGetVertXYZ(const DataArray *da) {
    Vert *v;
    s32 index = da->Int(2);
    MILO_ASSERT(index >= 0 && index < mVerts.size(), 0x86F);
    v = &mVerts[index];
    *da->Var(3) = v->pos.x;
    *da->Var(4) = v->pos.y;
    *da->Var(5) = v->pos.z;
    return 0;
}

DataNode RndMesh::OnSetVertXYZ(const DataArray *da) {
    Vert *v;
    s32 index = da->Int(2);
    MILO_ASSERT(index >= 0 && index < mVerts.size(), 0x87A);
    v = &mVerts[index];
    v->pos.x = da->Float(3);
    v->pos.y = da->Float(4);
    v->pos.z = da->Float(5);
    Sync(31);
    return 0;
}

DataNode RndMesh::OnGetVertUV(const DataArray *da) {
    Vert *v;
    s32 index = da->Int(2);
    MILO_ASSERT(index >= 0 && index < mVerts.size(), 0x886);
    v = &mVerts[index];
    *da->Var(3) = v->tex.x;
    *da->Var(4) = v->tex.y;
    return 0;
}

DataNode RndMesh::OnSetVertUV(const DataArray *da) {
    Vert *v;
    s32 index = da->Int(2);
    MILO_ASSERT(index >= 0 && index < mVerts.size(), 0x890);
    v = &mVerts[index];
    v->tex.x = da->Float(3);
    v->tex.y = da->Float(4);
    Sync(31);
    return 0;
}

DataNode RndMesh::OnGetFace(const DataArray *da) {
    Face *f;
    int index = da->Int(2);
    MILO_ASSERT(index >= 0 && index < mFaces.size(), 0x89B);
    f = &mFaces[index];
    *da->Var(3) = f->v1;
    *da->Var(4) = f->v2;
    *da->Var(5) = f->v3;
    return 0;
}

DataNode RndMesh::OnSetFace(const DataArray *da) {
    Face *f;
    int index = da->Int(2);
    MILO_ASSERT(index >= 0 && index < mFaces.size(), 0x8A6);
    f = &mFaces[index];
    f->v1 = da->Int(3);
    f->v2 = da->Int(4);
    f->v3 = da->Int(5);
    Sync(32);
    return 0;
}

DataNode RndMesh::OnUnitizeNormals(const DataArray *da) {
    for (Vert *it = Verts().begin(); it != Verts().end(); ++it) {
        Normalize(it->norm, it->norm);
    }
    return 0;
}

DataNode RndMesh::OnConfigureMesh(const DataArray *da) {
    static Symbol configurable_mesh("configurable_mesh");
    if (Type() != configurable_mesh)
        MILO_NOTIFY("Can't configure nonconfigurable mesh %s\n", Name());
    else {
        static Symbol left("left");
        static Symbol right("right");
        static Symbol height("height");
        float fleft = Property(left, true)->Float();
        float fright = Property(right, true)->Float();
        float fheight = Property(height, true)->Float();
        Vector3 v54(fleft, 0, fheight);
        Vector3 v60(fleft, 0, 0);
        Vector3 v6c(fright, 0, 0);
        Vector3 v78(fright, 0, fheight);
        mVerts[0].pos = v54;
        mVerts[1].pos = v60;
        mVerts[2].pos = v6c;
        mVerts[3].pos = v78;
        Sync(0x3F);
    }
    return 0;
}
