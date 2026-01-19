#pragma once
#include "hamobj/RhythmDetector.h"
#include "math/Color.h"
#include "math/DoubleExponentialSmoother.h"
#include "obj/Object.h"
#include "rnddx9/Rnd.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Tex.h"
#include "rndobj/Trans.h"
#include "utl/MemMgr.h"

struct DepthBuffer3DAttachment {
    RndTransformable *obj; // 0x0
    int player; // 0x4
    int unk8;
    int unkc;
    int unk10;
    int unk14;
    int unk18;
    int unk1c;
    int unk20;
};

/** "Render the Kinect depth buffer as a 3D mesh" */
class DepthBuffer3D : public RndDrawable, public RndTransformable {
public:
    // Hmx::Object
    virtual ~DepthBuffer3D();
    OBJ_CLASSNAME(DepthBuffer3D);
    OBJ_SET_TYPE(DepthBuffer3D);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndDrawable
    virtual void DrawShowing();
    virtual void ListDrawChildren(std::list<RndDrawable *> &);
    // RndHighlightable
    virtual void Highlight() { RndDrawable::Highlight(); }

    OBJ_MEM_OVERLOAD(0x2F);
    static void Init();
    NEW_OBJ(DepthBuffer3D)

    void SetPlayerPalette(RndTex *);
    void AddAttachment(const DepthBuffer3DAttachment &);
    void SetGrooviness(float);
    void SetGrooviness(RhythmDetector *, RhythmDetector *);
    void ForceDrawSkeletonIndex(int, bool);

protected:
    DepthBuffer3D();

    void UpdateAttachment(DepthBuffer3DAttachment &, const Vector4 &, const Vector4 &);

    static LargeQuadRenderData mQuad;

    /** "draw old school depth buffer - 1 plane" */
    bool mDrawSheet; // 0x100
    /** "Whether Player 1 should be drawn in this DepthBuffer3D" */
    bool mDrawPlayer1; // 0x101
    /** "Whether Player 2 should be drawn in this DepthBuffer3D" */
    bool mDrawPlayer2; // 0x102
    /** "Whether non-players should be drawn in this DepthBuffer3D" */
    bool mDrawNonPlayers; // 0x103
    /** "enabled alters xbox rendering to display every voxel" */
    bool mDebugLayout; // 0x104
    /** "Color for non-player pixels (i.e. the background)" */
    Hmx::Color mNobodyColor; // 0x108
    /** "1D palette for player depth" */
    ObjPtr<RndTex> mPlayerPalette; // 0x118
    ObjPtr<RndTex> unk12c;
    float unk140;
    /** "Starting point for palette". Ranges from -1 to 1. */
    float mPlayerPaletteOffset; // 0x144
    /** "Scale the coordinate used to look up the palette value.
        If the scale is 2, you'll cycle through the palette twice as fast, and so on.".
        Ranges from -100 to 100. */
    float mPlayerPaletteScale; // 0x148
    /** "Some Mat properties are used to render the depth buffer" */
    ObjPtr<RndMat> mMinimalMat; // 0x14c
    /** "Mesh to draw" */
    ObjPtr<RndMesh> mMesh; // 0x160
    /** "Stretch the depth buffer along an exponential curve.
        1 is the default; values greater than 1 mean more distortion
        for objects closer to the Kinect camera.". Ranges from 0 to 10. */
    float mStretchNearCamera; // 0x174
    /** "Multiply palette alpha by this value.". Ranges from 0 to 1. */
    float mOpacity; // 0x178
    float unk17c;
    float unk180;
    int unk184;
    bool unk188;
    ObjPtr<RndTex> unk18c;
    int unk1a0;
    int unk1a4;
    int unk1a8;
    int unk1ac;
    int unk1b0;
    int unk1b4;
    int unk1b8;
    int unk1bc;
    /** "How many times to tile the mesh in the x-axis/y-axis" */
    Vector2 mTile; // 0x1c0
    /** "Voxel scalar" */
    float mScaleVoxel; // 0x1c8
    /** "Voxel gap scalar" */
    float mScaleVoxelGap; // 0x1cc
    /** "horizontal fisheye coefficient" */
    float mFishEyeX; // 0x1d0
    /** "vertical fisheye coefficient" */
    float mFishEyeY; // 0x1d4
    ObjPtr<RhythmDetector> unk1d8;
    ObjPtr<RhythmDetector> unk1ec;
    std::vector<DepthBuffer3DAttachment> unk200;
    DoubleExponentialSmoother unk20c;
    DoubleExponentialSmoother unk220;
    DoubleExponentialSmoother unk234;
    DoubleExponentialSmoother unk248;
    DoubleExponentialSmoother unk25c;
    DoubleExponentialSmoother unk270;
    /** "maximum uv zooming" */
    float mMaxZoom; // 0x284
    /** "maximum uv zooming" */
    float mMaxDepthZoom; // 0x288
    bool unk28c;
};
