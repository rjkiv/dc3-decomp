#include "gesture/StreamRenderer.h"
#include "gesture/GestureMgr.h"
#include "hamobj/HamGameData.h"
#include "math/Color.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Rnd.h"
#include "rndobj/ShaderOptions.h"
#include "rndobj/Tex.h"
#include "rndobj/Utl.h"

RndCam *StreamRenderer::mCam;
RndTex *StreamRenderer::mBlurRT[2];

StreamRenderer::StreamRenderer()
    : mOutputTex(this), mForceMips(false), mDisplay(kStreamColor), mNumBlurs(4),
      mPlayer1DepthColor(1, 1, 1), mPlayer2DepthColor(0.48, 0.57, 0.8),
      mPlayer3DepthColor(0.05, 0.06, 0.54), mPlayer4DepthColor(0, 0, 0, 0),
      mPlayer5DepthColor(0, 0, 0, 0), mPlayer6DepthColor(0, 0, 0, 0),
      mPlayerDepthNobody(0, 0, 0, 0), mPlayer1DepthPalette(this),
      mPlayer2DepthPalette(this), mPlayerOtherDepthPalette(this),
      mBackgroundDepthPalette(this), mPlayer1DepthPaletteOffset(0),
      mPlayer2DepthPaletteOffset(0), mPlayerOtherDepthPaletteOffset(0),
      mBackgroundDepthPaletteOffset(0), mDrawPreClear(0), mForceDraw(0),
      mStaticColorIndices(0), mPCTestTex(this), mLagPrimaryTexture(0), unk154(0),
      unk190(0), unk194(0), unk198(0), unk19c(0), unk1a0(0), unk1a4(0), mPinkPlayer(0),
      mBluePlayer(0) {
    for (int i = 0; i < 6; i++) {
        mSmoothers[i].SetSmoothParameters(6, 0);
    }
    mLaggedPrimaryTexture[0] = nullptr;
    mLaggedPrimaryTexture[1] = nullptr;
}

StreamRenderer::~StreamRenderer() {
    delete mLaggedPrimaryTexture[0];
    delete mLaggedPrimaryTexture[1];
}

BEGIN_HANDLERS(StreamRenderer)
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(Hmx::Object)
    HANDLE(get_render_textures, OnGetRenderTextures)
    HANDLE_ACTION(
        set_crew_photo_horizontal_color, SetCrewPhotoHorizontalColor(_msg->Array(2))
    )
    HANDLE_ACTION(set_crew_photo_vertical_color, SetCrewPhotoVerticalColor(_msg->Array(2)))
    HANDLE_ACTION(
        set_crew_photo_player_detected,
        SetCrewPhotoPlayerDetected(_msg->Int(2), _msg->Int(3))
    )
    HANDLE_ACTION(set_crew_photo_player_centers, SetCrewPhotoPlayerCenters())
    HANDLE_ACTION(
        set_pink_player,
        SetPinkPlayer(
            TheGestureMgr->GetSkeletonIndexByTrackingID(
                TheGameData->Player(_msg->Size() > 2 ? _msg->Int(2) : 0)
                    ->GetSkeletonTrackingID()
            )
            + 1
        )
    )
    HANDLE_ACTION(
        set_blue_player,
        SetBluePlayer(
            TheGestureMgr->GetSkeletonIndexByTrackingID(
                TheGameData->Player(_msg->Size() > 2 ? _msg->Int(2) : 0)
                    ->GetSkeletonTrackingID()
            )
            + 1
        )
    )
END_HANDLERS

BEGIN_PROPSYNCS(StreamRenderer)
    SYNC_PROP_MODIFY(output_texture, mOutputTex, SetOutputTex())
    SYNC_PROP_MODIFY(force_mips, mForceMips, SetOutputTex())
    SYNC_PROP(display, (int &)mDisplay)
    SYNC_PROP(num_blurs, mNumBlurs)
    SYNC_PROP(player_depth_nobody, mPlayerDepthNobody)
    SYNC_PROP(player_depth_nobody_alpha, mPlayerDepthNobody.alpha)
    SYNC_PROP(player1_depth_color, mPlayer1DepthColor)
    SYNC_PROP(player1_depth_color_alpha, mPlayer1DepthColor.alpha)
    SYNC_PROP(player2_depth_color, mPlayer2DepthColor)
    SYNC_PROP(player2_depth_color_alpha, mPlayer2DepthColor.alpha)
    SYNC_PROP(player3_depth_color, mPlayer3DepthColor)
    SYNC_PROP(player3_depth_color_alpha, mPlayer3DepthColor.alpha)
    SYNC_PROP(player4_depth_color, mPlayer4DepthColor)
    SYNC_PROP(player4_depth_color_alpha, mPlayer4DepthColor.alpha)
    SYNC_PROP(player5_depth_color, mPlayer5DepthColor)
    SYNC_PROP(player5_depth_color_alpha, mPlayer5DepthColor.alpha)
    SYNC_PROP(player6_depth_color, mPlayer6DepthColor)
    SYNC_PROP(player6_depth_color_alpha, mPlayer6DepthColor.alpha)
    SYNC_PROP(player1_depth_palette, mPlayer1DepthPalette)
    SYNC_PROP(player1_depth_palette_offset, mPlayer1DepthPaletteOffset)
    SYNC_PROP(player2_depth_palette, mPlayer2DepthPalette)
    SYNC_PROP(player2_depth_palette_offset, mPlayer2DepthPaletteOffset)
    SYNC_PROP(player_other_depth_palette, mPlayerOtherDepthPalette)
    SYNC_PROP(player_other_depth_palette_offset, mPlayerOtherDepthPaletteOffset)
    SYNC_PROP(background_depth_palette, mBackgroundDepthPalette)
    SYNC_PROP(background_depth_palette_offset, mBackgroundDepthPaletteOffset)
    SYNC_PROP(force_draw, mForceDraw)
    SYNC_PROP(static_color_indices, mStaticColorIndices)
    SYNC_PROP_MODIFY(draw_pre_clear, mDrawPreClear, UpdatePreClearState())
    SYNC_PROP(pc_test_texture, mPCTestTex)
    SYNC_PROP(lag_primary_texture, mLagPrimaryTexture)
    SYNC_PROP(crew_photo_edge_iterations, mCrewPhotoEdgeIterations)
    SYNC_PROP(crew_photo_edge_offset, mCrewPhotoEdgeOffset)
    SYNC_PROP(crew_photo_horizontal_color, mCrewPhotoHorizontalColor)
    SYNC_PROP(crew_photo_vertical_color, mCrewPhotoVerticalColor)
    SYNC_PROP(crew_photo_blur_start, mCrewPhotoBlurStart)
    SYNC_PROP(crew_photo_blur_width, mCrewPhotoBlurWidth)
    SYNC_PROP(crew_photo_blur_iterations, mCrewPhotoBlurIterations)
    SYNC_PROP(crew_photo_background_brightness, mCrewPhotoBackgroundBrightness)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(StreamRenderer)
    SAVE_REVS(12, 1)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mOutputTex;
    bs << mForceMips;
    bs << mDisplay;
    bs << mPlayer1DepthColor << mPlayer2DepthColor << mPlayer3DepthColor
       << mPlayerDepthNobody;
    bs << mPlayer1DepthPalette << mPlayer1DepthPaletteOffset;
    bs << mBackgroundDepthPalette << mBackgroundDepthPaletteOffset;
    bs << mNumBlurs;
    bs << mPCTestTex;
    bs << mPlayer2DepthPalette << mPlayer2DepthPaletteOffset;
    bs << mPlayerOtherDepthPalette << mPlayerOtherDepthPaletteOffset;
    bs << mDrawPreClear << mForceDraw;
    bs << mLagPrimaryTexture;
    bs << mPlayer1DepthColor << mPlayer2DepthColor << mPlayer3DepthColor;
    bs << mStaticColorIndices;
    bs << mCrewPhotoEdgeIterations;
    bs << mCrewPhotoEdgeOffset;
    bs << mCrewPhotoHorizontalColor;
    bs << mCrewPhotoVerticalColor;
    bs << mCrewPhotoBlurStart;
    bs << mCrewPhotoBlurWidth;
    bs << mCrewPhotoBlurIterations;
    bs << mCrewPhotoBackgroundBrightness;
END_SAVES

BEGIN_COPYS(StreamRenderer)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(StreamRenderer)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mOutputTex)
        COPY_MEMBER(mForceMips)
        COPY_MEMBER(mDisplay)
        COPY_MEMBER(mPlayer1DepthColor)
        COPY_MEMBER(mPlayer2DepthColor)
        COPY_MEMBER(mPlayer3DepthColor)
        COPY_MEMBER(mPlayer4DepthColor)
        COPY_MEMBER(mPlayer5DepthColor)
        COPY_MEMBER(mPlayer6DepthColor)
        COPY_MEMBER(mPlayerDepthNobody)
        COPY_MEMBER(mPlayer1DepthPalette)
        COPY_MEMBER(mPlayer1DepthPaletteOffset)
        COPY_MEMBER(mPlayer2DepthPalette)
        COPY_MEMBER(mPlayer2DepthPaletteOffset)
        COPY_MEMBER(mPlayerOtherDepthPalette)
        COPY_MEMBER(mPlayerOtherDepthPaletteOffset)
        COPY_MEMBER(mBackgroundDepthPalette)
        COPY_MEMBER(mBackgroundDepthPaletteOffset)
        COPY_MEMBER(mNumBlurs)
        COPY_MEMBER(mPCTestTex)
        COPY_MEMBER(mDrawPreClear)
        COPY_MEMBER(mForceDraw)
        COPY_MEMBER(mLagPrimaryTexture)
        COPY_MEMBER(mCrewPhotoEdgeIterations)
        COPY_MEMBER(mCrewPhotoEdgeOffset)
        COPY_MEMBER(mCrewPhotoHorizontalColor)
        COPY_MEMBER(mCrewPhotoVerticalColor)
        COPY_MEMBER(mCrewPhotoBlurStart)
        COPY_MEMBER(mCrewPhotoBlurWidth)
        COPY_MEMBER(mCrewPhotoBlurIterations)
        COPY_MEMBER(mCrewPhotoBackgroundBrightness)
        SetOutputTex();
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(StreamRenderer)
    LOAD_REVS(bs)
    ASSERT_REVS(12, 1)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndDrawable)
    d >> mOutputTex;
    SetOutputTex();
    d >> mForceMips;
    d >> (int &)mDisplay;
    if (d.rev > 1) {
        d >> mPlayer1DepthColor >> mPlayer2DepthColor >> mPlayer3DepthColor
            >> mPlayerDepthNobody >> mPlayer1DepthPalette >> mPlayer1DepthPaletteOffset;
    }
    if (d.rev > 2) {
        d >> mBackgroundDepthPalette >> mBackgroundDepthPaletteOffset;
    }
    if (d.rev > 3) {
        d >> mNumBlurs;
    }
    if (d.rev > 4) {
        d >> mPCTestTex;
    }
    if (d.rev > 5) {
        d >> mPlayer2DepthPalette;
        d >> mPlayer2DepthPaletteOffset;
    } else {
        mPlayer2DepthPalette = mPlayer1DepthPalette;
        mPlayer2DepthPaletteOffset = mPlayer1DepthPaletteOffset;
    }
    if (d.rev > 6) {
        d >> mPlayerOtherDepthPalette;
        d >> mPlayerOtherDepthPaletteOffset;
    } else {
        mPlayerOtherDepthPalette = mPlayer1DepthPalette;
        mPlayerOtherDepthPaletteOffset = mPlayer1DepthPaletteOffset;
    }
    if (d.rev > 7) {
        d >> mDrawPreClear;
        if (d.altRev > 0) {
            d >> mForceDraw;
        }
    }
    if (d.rev > 8) {
        d >> mLagPrimaryTexture;
    }
    if (d.rev > 9) {
        d >> mPlayer4DepthColor >> mPlayer5DepthColor >> mPlayer6DepthColor;
    } else {
        mPlayer4DepthColor = mPlayer3DepthColor;
        mPlayer5DepthColor = mPlayer3DepthColor;
        mPlayer6DepthColor = mPlayer3DepthColor;
    }
    if (d.rev > 10) {
        d >> mStaticColorIndices;
    } else {
        mStaticColorIndices = false;
    }
    if (d.rev > 11) {
        d >> mCrewPhotoEdgeIterations;
        d >> mCrewPhotoEdgeOffset;
        d >> mCrewPhotoHorizontalColor;
        d >> mCrewPhotoVerticalColor;
        d >> mCrewPhotoBlurStart;
        d >> mCrewPhotoBlurWidth;
        d >> mCrewPhotoBlurIterations;
        d >> mCrewPhotoBackgroundBrightness;
    } else {
        mCrewPhotoEdgeIterations = 0;
        mCrewPhotoEdgeOffset = 0;
        mCrewPhotoBlurStart = 0;
        mCrewPhotoBlurWidth = 0;
        mCrewPhotoBlurIterations = 0;
        mCrewPhotoBackgroundBrightness = 0;
        mCrewPhotoHorizontalColor = 0;
        mCrewPhotoVerticalColor = 0;
    }
END_LOADS

void StreamRenderer::DrawShowing() {
    if (!mDrawPreClear) {
        DrawToTexture();
    }
}

void StreamRenderer::UpdatePreClearState() {
    TheRnd.PreClearDrawAddOrRemove(this, mDrawPreClear, false);
}

void StreamRenderer::Init() {
    REGISTER_OBJ_FACTORY(StreamRenderer)
    MILO_ASSERT(!mCam, 0xC9);
    mCam = ObjectDir::Main()->New<RndCam>("[stream renderer cam]");
    for (int i = 0; i < DIM(mBlurRT); i++) {
        MILO_ASSERT(mBlurRT[i] == NULL, 0xCF);
        mBlurRT[i] = Hmx::Object::New<RndTex>();
        mBlurRT[i]->SetBitmap(
            320, 240, TheRnd.Bpp(), RndTex::kRenderedNoZ, false, nullptr
        );
    }
}

void StreamRenderer::Terminate() {
    for (int i = 0; i < DIM(mBlurRT); i++) {
        RELEASE(mBlurRT[i]);
    }
    RELEASE(mCam);
}

void StreamRenderer::SetPinkPlayer(int player) { mPinkPlayer = player; }
void StreamRenderer::SetBluePlayer(int player) { mBluePlayer = player; }

DataNode StreamRenderer::OnGetRenderTextures(DataArray *) {
    return GetRenderTextures(Dir());
}

void StreamRenderer::SetCrewPhotoHorizontalColor(DataArray *cfg) {
    if (cfg->Size() == 3) {
        mCrewPhotoHorizontalColor.red = cfg->Float(0);
        mCrewPhotoHorizontalColor.green = cfg->Float(1);
        mCrewPhotoHorizontalColor.blue = cfg->Float(2);
    }
}

void StreamRenderer::SetCrewPhotoVerticalColor(DataArray *cfg) {
    if (cfg->Size() == 3) {
        mCrewPhotoVerticalColor.red = cfg->Float(0);
        mCrewPhotoVerticalColor.green = cfg->Float(1);
        mCrewPhotoVerticalColor.blue = cfg->Float(2);
    }
}

ShaderType StreamRenderer::GetShaderType() const {
    ShaderType t = kDrawRectShader;
    switch (mDisplay) {
    case 0:
        t = kYUVtoRGBShader;
        break;
    case 1:
        t = kYUVtoBlackAndWhiteShader;
        break;
    case 2:
        t = kDrawRectShader;
        break;
    case 3:
        t = kPlayerDepthVisShader;
        break;
    case 4:
        t = kPlayerDepthShellShader;
        break;
    case 5:
        t = kPlayerDepthShell2Shader;
        break;
    case 6:
        t = kPlayerGreenScreenShader;
        break;
    case 7:
        t = kPlayerDepthGreenScreenShader;
        break;
    case 8:
        t = kCrewPhotoShader;
        break;
    default:
        MILO_FAIL("Unknown StreamDisplay in StreamRenderer");
        break;
    }
    return t;
}

void StreamRenderer::SetCrewPhotoPlayerDetected(int player, bool b2) {
    MILO_ASSERT_RANGE(player, 0, 6, 0x212);
    float set = b2 ? 1.0f : 0.0f;
    switch (player) {
    case 0:
        unk190 = set;
        break;
    case 1:
        unk194 = set;
        break;
    case 2:
        unk198 = set;
        break;
    case 3:
        unk19c = set;
        break;
    case 4:
        unk1a0 = set;
        break;
    case 5:
        unk1a4 = set;
        break;
    default:
        break;
    }
}
