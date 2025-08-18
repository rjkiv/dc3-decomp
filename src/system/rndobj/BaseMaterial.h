#pragma once
#include "math/Color.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Object.h"
#include "rndobj/CubeTex.h"
#include "rndobj/Fur.h"
#include "rndobj/Tex.h"
#include "utl/BinStream.h"

enum Cull {
    /** "No culling.  User sees both front and back of polygon." */
    kCullNone = 0,
    /** "Only the front face is drawn.  The back face of the polygon is not drawn." */
    kCullRegular = 1,
    /** "The back face of polygone is drawn, but not the front." */
    kCullBackwards = 2
};

enum ShaderVariation {
    kShaderVariationNone = 0,
    kShaderVariationSkin = 1,
    kShaderVariationHair = 2,
    kShaderVariationWorldProjection = 3
};

enum StencilMode {
    kStencilIgnore = 0,
    kStencilWrite = 1,
    kStencilTest = 2,
};

enum TexGen {
    /** "use vertex UV unchanged" */
    kTexGenNone = 0,
    /** "transform vertex UV about center with stage xfm" */
    kTexGenXfm = 1,
    /** "sphere map that rotates around object with camera, xfm is direction of map, fast
     * on gs slow on cpu, flips at poles" */
    kTexGenSphere = 2,
    /** "project from direction of stage xfm in world coords" */
    kTexGenProjected = 3,
    /** "like Xfm but about origin rather than center" */
    kTexGenXfmOrigin = 4,
    /** "reflection map, like sphere map but perspective correct and does not flip, fast
     * on cpu but slow on gs" */
    kTexGenEnviron = 5,
};

enum TexWrap {
    /** "UVs outside the range [0,1] are clamped" */
    kTexWrapClamp = 0,
    /** "The image repeats itself across the surface" */
    kTexWrapRepeat = 1,
    /** "texels outside the UV range [0,1] are black" */
    kTexBorderBlack = 2,
    /** "texels outside the UV range [0,1] are white" */
    kTexBorderWhite = 3,
    /** "The image repeats itself, but is flipped every other repetition" */
    kTexWrapMirror = 4
};

enum ZMode {
    /** "always draw but don't update z-buffer" */
    kZModeDisable = 0,
    /** "draw and update z-buffer if closer than z-buffer" */
    kZModeNormal = 1,
    /** "draw if closer than or equal z-buffer but don't update z-buffer. Often used
     * with SrcAlpha or Add blending so those objects don't occlude other similar
     * objects" */
    kZModeTransparent = 2,
    /** "always draw and update z-buffer" */
    kZModeForce = 3,
    /** "draw and update z-buffer if closer than or equal to z-buffer" */
    kZModeDecal = 4,
};

struct MatPerfSettings {
    MatPerfSettings()
        : mRecvProjLights(false), mRecvPointCubeTex(false), mPS3ForceTrilinear(false) {}
    void Save(BinStream &) const;
    void LoadOld(BinStreamRev &);
    void Load(BinStream &);

    /** "Check this option to allow the material to receive projected lighting" */
    bool mRecvProjLights;
    /** "Check this option to allow the material to receive projected cube maps from a
     * point light" */
    bool mRecvPointCubeTex;
    /** "Force trilinear filtering of diffuse map (PS3 only)" */
    bool mPS3ForceTrilinear;
};

// size 0x1f8
class BaseMaterial : public Hmx::Object {
public:
    enum Blend {
        /** "Don't show this material at all; just show the frame buffer" */
        kBlendDest = 0,
        /** "Don't blend this material at all" */
        kBlendSrc = 1,
        /** "Output is material + frame buffer" */
        kBlendAdd = 2,
        /** "Output is (material x mat alpha) + (frame buffer x (1 - mat alpha))" */
        kBlendSrcAlpha = 3,
        /** "Output is (material x mat alpha) + frame buffer" */
        kBlendSrcAlphaAdd = 4,
        /** "Output is frame buffer - material" */
        kBlendSubtract = 5,
        /** "Output is frame buffer x material" */
        kBlendMultiply = 6,
        /** "Output is material + (frame buffer x (1 - mat alpha)" */
        kPreMultAlpha = 7,
        /** "Lightens the frame buffer based on the lightness of the material" */
        kScreen = 8,
        /** "Compares the material and frame buffer and picks the lightest value, per
         * channel"
         */
        kLighten = 9,
        /** "Compares the material and frame buffer and picks the darkest value, per
         * channel"
         */
        kDarken = 10
    };

    virtual ~BaseMaterial();
    OBJ_CLASSNAME(BaseMaterial);
    OBJ_SET_TYPE(BaseMaterial);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    const DataNode *GetDefaultPropVal(Symbol);
    BaseMaterial *NextPass() const { return mNextPass; }
    bool IsNextPass(BaseMaterial *m);

protected:
    BaseMaterial();
    bool PropValDifferent(Symbol, BaseMaterial *);

    DataNode OnIsDefaultPropVal(const DataArray *);
    DataNode OnAllowedNextPass(const DataArray *);
    DataNode OnAllowedNormalMap(const DataArray *);

    static void SetDefaultMat(BaseMaterial *);

    /** "Base material color" */
    Hmx::Color mColor; // 0x2c
    /** "Modulate with environment ambient and lights" */
    bool mUseEnviron; // 0x3c
    /** "Use vertex color and alpha for base or ambient" */
    bool mPrelit; // 0x3d
    /** "Base texture map, modulated with color and alpha" */
    ObjPtr<RndTex> mDiffuseTex; // 0x40
    ObjPtr<RndTex> mDiffuseTex2; // 0x54
    /** "Double the intensity of base map" */
    bool mIntensify; // 0x68
    /** "Texture mapping mode" */
    TexWrap mTexWrap; // 0x6c
    /** "How to generate texture coordinates" */
    TexGen mTexGen; // 0x70
    /** "Transform for coordinate generation" */
    Transform mTexXfm; // 0x74
    /** "How to blend poly into screen" */
    Blend mBlend; // 0xb4
    /** "How to read and write z-buffer" */
    ZMode mZMode; // 0xb8
    /** "Cut zero alpha pixels from z-buffer" */
    bool mAlphaCut; // 0xbc
    /** "Write pixel alpha to screen" */
    bool mAlphaWrite; // 0xbd
    bool mForceAlphaWrite; // 0xbe
    /** "Alpha level below which gets cut". Ranges from 0 to 255. */
    int mAlphaThreshold; // 0xc0
    /** "Next material for object" */
    ObjPtr<BaseMaterial> mNextPass; // 0xc4

    // Next gen:
    /** "Next-generation graphics settings
        Does not apply to particles" */

    // Shader capabilities:
    /** "Defines the capabilities of the shader generated using this material" */

    /** "Is the Mat lit with point lights?" */
    bool mPointLights; // 0xd8
    /** "Is the Mat affected by fog?" */
    bool mFog; // 0xd9
    /** "Is the Mat affected its Environment's fade_out?" */
    bool mFadeout; // 0xda
    /** "Is the Mat affected its Environment's color adjust?" */
    bool mColorAdjust; // 0xdb

    // Performance:
    /** "Performance options for this material" */
    MatPerfSettings mPerfSettings; // 0xdc

    /** "Use per-pixel lighting" */
    bool mPerPixelLit; // 0xdf

    // Emissive settings:
    /** "Settings for manipulating the emissive properties of the material" */

    /** "Multiplier to apply to emission" */
    float mEmissiveMultiplier; // 0xe0
    /** "Map for self illumination" */
    ObjPtr<RndTex> mEmissiveMap; // 0xe4

    // Normal settings:
    /** "Settings for manipulating the normals of the material.
        Requires per-pixel lighting." */
    /** "Texture map to define lighting normals.
        Requires per-pixel lighting." */
    ObjPtr<RndTex> mNormalMap; // 0xf8
    /** "Amount to diminish normal map bumpiness, 0 is neutral, 1 is no bumps, -1
     * exaggerates". Ranges from -3 to 1. */
    float mDeNormal; // 0x10c
    /** "Detail map texture" */
    ObjPtr<RndTex> mNormDetailMap; // 0x110
    /** "Texture tiling scale for the detail map" */
    float mNormDetailTiling; // 0x124
    /** "Strength of the detail map bumpiness" */
    float mNormDetailStrength; // 0x128

    // Specular settings:
    /** "Settings for manipulating the specular properties of the material" */
    /** "Specular color. If a specular texture is present, this color is multiplied by the
     * specular texture RGB color." */
    /** Specular power: "This is the maximum level of 'shininess' of the surface; higher
     * numbers represent a shinier surface. If a specular texture is present, this value
     * is multiplied by the specular texture alpha channel." */
    Hmx::Color mSpecularRGB; // 0x12c
    /** "Secondary specular color.  Only valid for certain shader variations." */
    /** "Secondary specular power.  Only valid for certain shader variations." */
    Hmx::Color mSpecular2RGB; // 0x13c
    /** "Texture map for specular color (RGB channels) and glossiness (Alpha channel).
     * Requires per-pixel lighting." */
    ObjPtr<RndTex> mSpecularMap; // 0x14c
    /** "Specular power in downward (strand) direction, 0 to disable". Ranges from 0 to
     * 100. */
    float mAnisotropy; // 0x160

    // Rim light settings:
    /** "Settings for manipulating the rim lighting properties of the material" */
    /** "Rim lighting color. If a rim texture is present, this color is multiplied by the
     * rim texture RGB color." */
    /** Rim power: "Rim lighting power. This is the sharpness of the wrap-around effect;
     * higher numbers result in a sharper rim lighting effect. If a rim texture is
     * present, this value is multiplied by the rim texture alpha channel."
     * Ranges from 0 to 64. */
    Hmx::Color mRimRGB; // 0x164
    /** "Texture map that defines the rim lighting color (in the RGB channels) and power
     * (in the Alpha channel)." */
    ObjPtr<RndTex> mRimMap; // 0x174
    /** "When enabled, this causes the rim effect to highlight the undersides of meshes"
     */
    bool mRimLightUnder; // 0x188

    // Environ map settings:
    /** "Settings for manipulating the environment map properties of the material" */
    /** "Cube texture for reflections" */
    ObjPtr<RndCubeTex> mEnvironMap; // 0x18c
    /** "Causes the relfection to increase at glancing angles and fade when viewed
     * directly" */
    bool mEnvironMapFalloff; // 0x1a0
    /** "Masks the reflection by the specular map alpha channel" */
    bool mEnvironMapSpecMask; // 0x1a1

    // Refraction settings:
    /** "Settings for applying refraction to the material" */
    /** "When enabled, this material will refract the screen under the material" */
    bool mRefractEnabled; // 0x1a2
    /** "The scale of the refraction of the screen under the material." Ranges from 0 to
     * 100. */
    float mRefractStrength; // 0x1a4
    /** "This is a normal map used to distort the screen under the material. If none is
     * specified, the regular normal map will be used." */
    ObjPtr<RndTex> mRefractNormalMap; // 0x1a8

    // Custom shader settings:
    /** "Settings for using a custom shader effect." */
    /** "Select a variation on the shader to enable a new range of rendering features." */
    ShaderVariation mShaderVariation; // 0x1bc
    /** "Use fur shader" */
    ObjPtr<RndFur> mFur; // 0x1c0

    // Misc settings:
    /** "Miscellaneous settings." */
    bool mNeverFitToSpline; // 0x1d4
    bool mAllowDistortionEffects; // 0x1d5
    float mShockwaveMult; // 0x1d8
    float mWorldProjectionTiling; // 0x1dc
    float mWorldProjectionStartBlend; // 0x1e0
    float mWorldProjectionEndBlend; // 0x1e4
    /** "Projected material from camera's POV" */
    bool mScreenAligned; // 0x1e8
    /** "Cull backface polygons" */
    Cull mCull; // 0x1ec
    /** "How to read and write the stencil buffer" */
    StencilMode mStencilMode; // 0x1f0
    /** "Multiplier to apply to bloom" */
    float mBloomMultiplier; // 0x1f4
};
