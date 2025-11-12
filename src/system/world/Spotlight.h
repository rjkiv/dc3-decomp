#pragma once
#include "math/Color.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "rndobj/Draw.h"
#include "rndobj/Env.h"
#include "rndobj/Flare.h"
#include "rndobj/Lit.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Poll.h"
#include "rndobj/Tex.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

/** "Represents a beam and floorspot for venue modeling" */
class Spotlight : public RndDrawable, public RndTransformable, public RndPollable {
public:
    struct BeamDef {
        enum Shape {
            kBeamConic = 0,
            kBeamRect = 1,
            kBeamSheet = 2,
            kBeamQuadXYZ = 3,
            kBeamQuadZ = 4
        };

        BeamDef(Hmx::Object *);
        ~BeamDef() { RELEASE(mBeam); }
        void OnSetMat(RndMat *);

        RndMesh *mBeam; // 0x0
        /** "Whether this is a beam or a cone" */
        bool mIsCone; // 0x4
        /** "Length of the beam/cone" */
        float mLength; // 0x8
        /** "Radius at the top of the beam/cone" */
        float mTopRadius; // 0xc
        /** "Radius at the bottom of the beam/cone" */
        float mBottomRadius; // 0x10
        /** "For beams, length of the side transparency border
            at the top of the beam" */
        float mTopSideBorder; // 0x14
        /** "For beams, length of the side transparency border
            at the bottom of the beam" */
        float mBottomSideBorder; // 0x18
        /** "Length of the bottom transparency border" */
        float mBottomBorder; // 0x1c
        /** "Offset of beam along trajectory" */
        float mOffset; // 0x20
        /** "Amount to offset beam rotation (in degrees)" */
        Vector2 mTargetOffset; // 0x24
        /** "raise or lower intensity compared to og beams" */
        float mBrighten; // 0x2c
        /** "expand or shrink the radii compared to og beams" */
        float mExpand; // 0x30
        /** "Shape of the beam" */
        Shape mShape; // 0x34 - enum Shape
        /** "Number of divisions along length" */
        int mNumSections; // 0x38
        /** "Number of divisions along width or around cone" */
        int mNumSegments; // 0x3c
        /** "cross section intensity override texture" */
        ObjPtr<RndTex> mXSection; // 0x40
        /** "Objects that create cutout shadow in the beam." */
        ObjPtrList<RndDrawable> mCutouts; // 0x4c
        /** "The material to use for the beam/cone" */
        ObjPtr<RndMat> mMat; // 0x68
    };
    // Hmx::Object
    virtual ~Spotlight();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(Spotlight);
    OBJ_SET_TYPE(Spotlight);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void InitObject();
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndHighlightable
    virtual void Highlight() { RndDrawable::Highlight(); }
    // RndDrawable
    virtual void UpdateSphere();
    virtual bool MakeWorldSphere(Sphere &, bool);
    virtual void Mats(std::list<class RndMat *> &, bool);
    virtual void DrawShowing();
    virtual void ListDrawChildren(std::list<RndDrawable *> &);
    virtual RndDrawable *CollideShowing(const Segment &, float &, Plane &);
    virtual int CollidePlane(const Plane &);
    virtual void UpdateBounds();
    // RndPollable
    virtual void Poll();

    static RndEnviron *sEnviron;

    static void Init();

    OBJ_MEM_OVERLOAD(0x22)
    NEW_OBJ(Spotlight)

    Hmx::Color &Color() { return mColorOwner->mColor; }
    float Intensity() const { return mColorOwner->mIntensity; }
    void SetFlareIsBillboard(bool);
    void SetIntensity(float);
    void SetColorIntensity(const Hmx::Color &c, float f);

    static void RemoveFromLists(Spotlight *);

protected:
    Spotlight();

    void Generate();
    void UpdateTransforms();
    void UpdateFlare();
    void SetColor(int);
    void CloseSlaves();
    void PropogateToPresets(int);

    static void BuildBoard();

    /** "Material to use for the floor spot" */
    ObjPtr<RndMat> mSpotMaterial; // 0x108
    RndFlare *mFlare; // 0x11c
    /** "Whether the flare is enabled (keyframed by light presets)" */
    bool mFlareEnabled; // 0x120
    /** "Whether the flare performs a visiblity test (or is always visible)" */
    bool mFlareVisibilityTest; // 0x121
    /** "Offset of flare along spotlight trajectory" */
    float mFlareOffset; // 0x124
    /** "Scale of the floor disc" */
    float mSpotScale; // 0x128
    /** "Height offset of the floor disc" */
    float mSpotHeight; // 0x12c
    Transform unk130; // 0x130
    Transform unk170; // 0x170
    /** "Color of the spotlight" */
    Hmx::Color mColor; // 0x1b0
    /** "Intensity of the spotlight" */
    float mIntensity; // 0x1c0
    /** "Master for light color and intensity" */
    ObjOwnerPtr<Spotlight> mColorOwner; // 0x1c4
    /** "Size of the lens billboard" */
    float mLensSize; // 0x1d8
    /** "Offset of the lens billboard" */
    float mLensOffset; // 0x1dc
    /** "Material to use for the lens" */
    ObjPtr<RndMat> mLensMaterial; // 0x1e0
    BeamDef mBeam; // 0x1f4
    ObjPtrList<RndLight> mSlaves; // 0x270
    /** "Optional light can mesh to use" */
    ObjPtr<RndMesh> mLightCanMesh; // 0x284
    Transform unk298; // 0x298
    /** "Offset of light can along beam trajectory" */
    float mLightCanOffset; // 0x2d8
    /** "Object to target spotlight.
        Note that it's easier to move a targetted spotlight
        in World space when in Milo." */
    ObjPtr<RndTransformable> mTarget; // 0x2dc
    bool unk2f0;
    /** "Reference object for floor height, uses spot target if not set" */
    ObjPtr<RndTransformable> mSpotTarget; // 0x2f4
    float unk308;
    /** "Whether the target should cast a shadow" */
    bool mTargetShadow; // 0x30c
    /** "Can't optimize render end of render batching of light can with others" */
    bool mLightCanSort; // 0x30d
    Hmx::Matrix3 unk310;
    bool unk340;
    /** "0-1, controls how fast spotlight moves to reach target" */
    float mDampingConstant; // 0x344
    /** "Additional objects that should be drawn by the spotlight." */
    ObjPtrList<RndDrawable> mAdditionalObjects; // 0x348
    Vector3 unk35c;
    /** "Whether this spotlight coloring should be animated by light presets." */
    bool mAnimateColorFromPreset; // 0x36c
    /** "Whether this spotlight position/rotation should be animated by light presets." */
    bool mAnimateOrientationFromPreset; // 0x36d
    bool unk36e; // 0x36e
    Hmx::Quat unk370;
};
