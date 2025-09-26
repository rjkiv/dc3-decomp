#pragma once
#include "math/Color.h"
#include "math/DoubleExponentialSmoother.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Tex.h"
#include "utl/MemMgr.h"

enum StreamDisplay {
    /** "Color output of camera" */
    kStreamColor = 0,
    /** "Depth buffer output" */
    kStreamBasicDepth = 1,
    /** "DC1 visualizer" */
    kStreamPlayerDepthVis = 2,
    /** "DC1 player helper frame" */
    kStreamPlayerDepthShell = 3,
    /** "DC2 player helper frame" */
    kStreamPlayerDepthShell2 = 4,
    /** "Convert color output to black and white" */
    kStreamBlackAndWhite = 5,
    /** "RGB player without background" */
    kStreamPlayerGreenscreen = 6,
    /** "RGB player with depth buffer" */
    kStreamPlayerDepthGreenscreen = 7,
    /** "Color output with edge detection on background and radial blur on players" */
    kStreamCrewPhoto = 8
};

// size 0x3b4
/** "Renders Natal stream textures into a texture." */
class StreamRenderer : public RndDrawable {
public:
    // Hmx::Object
    virtual ~StreamRenderer();
    OBJ_CLASSNAME(StreamRenderer);
    OBJ_SET_TYPE(StreamRenderer);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndDrawable
    virtual void DrawShowing();
    virtual void DrawPreClear() { DrawToTexture(); }
    virtual void UpdatePreClearState();

    OBJ_MEM_OVERLOAD(0x25);
    NEW_OBJ(StreamRenderer);

    void SetPinkPlayer(int);
    void SetBluePlayer(int);
    void SetCrewPhotoPlayerDetected(int, bool);

    void SetOutputTex() {
        if (mForceMips && mOutputTex) {
            mOutputTex->SetBitmap(
                mOutputTex->Width(),
                mOutputTex->Height(),
                mOutputTex->Bpp(),
                mOutputTex->GetType(),
                true,
                nullptr
            );
        }
    }

    static void Init();
    static void Terminate();

private:
    void SetCrewPhotoPlayerCenters();

protected:
    StreamRenderer();

    void DrawToTexture();
    void SetCrewPhotoHorizontalColor(DataArray *);
    void SetCrewPhotoVerticalColor(DataArray *);
    DataNode OnGetRenderTextures(DataArray *);

    static RndCam *mCam;
    static RndTex *mBlurRT[2];

    /** "Texture to write to" */
    ObjPtr<RndTex> mOutputTex; // 0x40
    /** "Generate mip maps for the texture." */
    bool mForceMips; // 0x54
    /** "Natal buffer to display" */
    StreamDisplay mDisplay; // 0x58
    /** "Number of times to blur the player silhouette texture".
        Ranges from 0 to 64. */
    int mNumBlurs; // 0x5c
    /** "Player 1 color" */
    Hmx::Color mPlayer1DepthColor; // 0x60
    /** "Player 2 color" */
    Hmx::Color mPlayer2DepthColor; // 0x70
    /** "Player 3 color" */
    Hmx::Color mPlayer3DepthColor; // 0x80
    /** "Player 4 color" */
    Hmx::Color mPlayer4DepthColor; // 0x90
    /** "Player 5 color" */
    Hmx::Color mPlayer5DepthColor; // 0xa0
    /** "Player 6 color" */
    Hmx::Color mPlayer6DepthColor; // 0xb0
    /** "Color for non-player pixels (i.e. the background)" */
    Hmx::Color mPlayerDepthNobody; // 0xc0
    /** "1D palette for player 1 depth" */
    ObjPtr<RndTex> mPlayer1DepthPalette; // 0xd0
    /** "1D palette for player 2 depth" */
    ObjPtr<RndTex> mPlayer2DepthPalette; // 0xe4
    /** "1D palette for players 3-6" */
    ObjPtr<RndTex> mPlayerOtherDepthPalette; // 0xf8
    /** "1D palette for background depth " */
    ObjPtr<RndTex> mBackgroundDepthPalette; // 0x10c
    /** "Starting point for p1 palette". Ranges from -1 to 1. */
    float mPlayer1DepthPaletteOffset; // 0x120
    /** "Starting point for p2 palette". Ranges from -1 to 1. */
    float mPlayer2DepthPaletteOffset; // 0x124
    /** "Starting point for p3-p6 palettes". Ranges from -1 to 1. */
    float mPlayerOtherDepthPaletteOffset; // 0x128
    /** "Starting point for palette". Ranges from -1 to 1. */
    float mBackgroundDepthPaletteOffset; // 0x12c
    bool mDrawPreClear; // 0x130
    /** "Always render the image, even there is no new depth/color buffer this frame." */
    bool mForceDraw; // 0x131
    /** "Assign colors by index instead of giving preference to Player 1 and Player 2." */
    bool mStaticColorIndices; // 0x132
    /** "StreamRenderer will output this test texture.
        Only works on the PC." */
    ObjPtr<RndTex> mPCTestTex; // 0x134
    /** "Used to lag the depth image by one frame
        to better line up with the color image for greenscreening" */
    bool mLagPrimaryTexture; // 0x148
    RndTex *mLaggedPrimaryTexture[2]; // 0x14c
    int unk154; // 0x154
    float mCrewPhotoEdgeIterations; // 0x158
    float mCrewPhotoEdgeOffset; // 0x15c
    Hmx::Color mCrewPhotoHorizontalColor; // 0x160
    Hmx::Color mCrewPhotoVerticalColor; // 0x170
    float mCrewPhotoBlurStart; // 0x180
    float mCrewPhotoBlurWidth; // 0x184
    float mCrewPhotoBlurIterations; // 0x188
    float mCrewPhotoBackgroundBrightness; // 0x18c
    float unk190; // 0x190
    float unk194; // 0x194
    float unk198; // 0x198
    float unk19c; // 0x19c
    float unk1a0; // 0x1a0
    float unk1a4; // 0x1a4
    float mPinkPlayer; // 0x1a8
    float mBluePlayer; // 0x1ac
    char filler2[0x60];
    Vector3DESmoother mSmoothers[6]; // 0x210
};
