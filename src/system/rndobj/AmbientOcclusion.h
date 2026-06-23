#pragma once
#include "math/Geo.h"
#include "math/kdTree.h"
#include "math/Vec.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Mesh.h"
#include "utl/MemMgr.h"

/** "Computes ambient occlusion and automatic tessellation.
    Also stores AO configuration options." */
class RndAmbientOcclusion : public Hmx::Object {
public:
    enum Quality {
        kQuality_Accurate = 0,
        kQuality_Fast = 1,
        kQuality_Max = 2
    };
    struct Edge {
        bool operator<(const Edge &) const;
    };

    // Hmx::Object
    virtual ~RndAmbientOcclusion();
    OBJ_CLASSNAME(AmbientOcclusion);
    OBJ_SET_TYPE(AmbientOcclusion);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    void Clean();
    void BuildTrees(Quality);
    void BuildObjectLists();
    bool IsSerializable(const RndMesh *) const;
    void CalculateAO(float *);
    void Tessellate(float *, float *);

    OBJ_MEM_OVERLOAD(0x15);
    NEW_OBJ(RndAmbientOcclusion)
    static void Init() { REGISTER_OBJ_FACTORY(RndAmbientOcclusion) }

protected:
    RndAmbientOcclusion();

    void OnCalculate(bool);
    void BuildSHCoeff(const Vector3 &, float *) const;
    bool IsValid_Mesh(const RndMesh *) const;
    bool IsValid_AOCast(const RndMesh *) const;
    bool IsValid_AOReceive(const RndMesh *) const;
    bool IsValid_Tessellate(const RndMesh *, const ObjectDir *) const;
    void TransformNormal(const Vector3 &, const Hmx::Matrix3 &, Vector3 &) const;
    void DumpObjList(const char *, const std::vector<RndMesh *> &) const;
    bool IsMeshAnimated(const RndMesh *) const;
    bool CanBurnXfm(const RndMesh *) const;
    void PreprocessMesh();
    void BurnTransform(RndMesh *, std::list<RndMesh *> &) const;

    DataNode OnGetValidObjects(DataArray *) const;
    DataNode OnGetRecvMeshes(DataArray *);

    /** "These objects will NOT cast shadows." */
    ObjPtrList<Hmx::Object> mDontCastAO; // 0x2c
    /** "These objects will NOT receive shadows." */
    ObjPtrList<Hmx::Object> mDontReceiveAO; // 0x40
    /** "These objects will be automatically tessellated." */
    ObjPtrList<Hmx::Object> mTessellate; // 0x54
    /** "Ignore transparent objects when casting shadows." */
    bool mIgnoreTransparent; // 0x68
    /** "Ignore prelit materials when receiving shadows." */
    bool mIgnorePrelit; // 0x69
    /** "Ignore hidden objects when casting or receiving shadows." */
    bool mIgnoreHidden; // 0x6a
    /** "Use the mesh normals when calculating. Otherwise,
        smoothed normals will be re-generated for each mesh." */
    bool mUseMeshNormals; // 0x6b
    /** "If true, this will cause back faces of triangles to be added.
        This will more than double the calculation time." */
    bool mIntersectBackFaces; // 0x6c
    /** "The maximum number of polys tessellation will generate.
        This is a multiplier of the current poly count.
        A value of 2 will generate a maximum of 2x the polys."
        Ranges from 2 to 32. */
    int mTessellateTriLimit; // 0x70
    /** "The error threshold for the ambient occlusion calculation.
        Error larger than this value will result in a triangle being split."
        Ranges from 0.1 to 15.0. */
    float mTessellateTriError; // 0x74
    /** "Triangles larger than this size will always be split."
        Ranges from 1 to 500. */
    float mTessellateTriLarge; // 0x78
    /** "Triangles smaller than this size will not be split any further."
        Ranges from 0.1 to 500. */
    float mTessellateTriSmall; // 0x7c
    std::vector<RndMesh *> mObjectsCast; // 0x80
    std::vector<RndMesh *> mObjectsReceive; // 0x8c
    std::vector<RndMesh *> mObjectsTessellate; // 0x98
    std::vector<Triangle> mTriList; // 0xa4
    kdTree<Triangle> *mTree; // 0xb0
    Quality mQuality; // 0xb4
    std::vector<Vector3> unkb8;
};
