#include "gesture/StreamRenderer.h"
#include "math/Color.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/Rnd.h"
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
    mLaggedPrimaryTexture[0] = 0;
    mLaggedPrimaryTexture[1] = 0;
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
    SAVE_REVS(0xC, 1)
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

void StreamRenderer::Init() {
    REGISTER_OBJ_FACTORY(StreamRenderer)
    MILO_ASSERT(!mCam, 0xC9);
    mCam = ObjectDir::Main()->New<RndCam>("[stream renderer cam]");
    for (int i = 0; i < 2; i++) {
        MILO_ASSERT(mBlurRT[i] == NULL, 0xCF);
        mBlurRT[i] = Hmx::Object::New<RndTex>();
        mBlurRT[i]->SetBitmap(
            0x140, 0xf0, TheRnd.Bpp(), RndTex::kTexRenderedNoZ, false, nullptr
        );
    }
}

void StreamRenderer::Terminate() {
    for (int i = 0; i < 2; i++) {
        RELEASE(mBlurRT[i]);
    }
    RELEASE(mCam);
}

void StreamRenderer::SetPinkPlayer(int player) { mPinkPlayer = player; }
void StreamRenderer::SetBluePlayer(int player) { mBluePlayer = player; }

DataNode StreamRenderer::OnGetRenderTextures(DataArray *) {
    return GetRenderTextures(Dir());
}

void StreamRenderer::UpdatePreClearState() {
    TheRnd.PreClearDrawAddOrRemove(this, mDrawPreClear, false);
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
