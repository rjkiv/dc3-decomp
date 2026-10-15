#include "gesture/StreamRenderer.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "gesture/LiveCameraInput.h"
#include "gesture/Skeleton.h"
#include "hamobj/Ham.h"
#include "hamobj/HamGameData.h"
#include "math/Color.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "os/Debug.h"
#include "rnddx9/Rnd.h"
#include "rndobj/Cam.h"
#include "rndobj/Draw.h"
#include "rndobj/RenderState.h"
#include "rndobj/Rnd.h"
#include "rndobj/ShaderMgr.h"
#include "rndobj/ShaderOptions.h"
#include "rndobj/Tex.h"
#include "rndobj/Utl.h"

RndCam *StreamRenderer::mCam;
RndTex *StreamRenderer::mBlurRT[2];

namespace {
    static int DisplayStreams[9];

    void GetPlayerIndexes(int *const indices) {
        bool indexAvailable[NUM_SKELETONS] = { true, true, true, true, true, true };
        for (int i = 0; i < NUM_SKELETONS; i++) {
            indices[i] = -1;
        }
        Skeleton *skel0 = TheGestureMgr->GetSkeletonByTrackingID(
            TheGameData->Player(0)->GetSkeletonTrackingID()
        );
        if (skel0) {
            int idx = skel0->SkeletonIndex() + 1;
            indices[0] = idx;
            indexAvailable[idx - 1] = false;
        } else {
            indices[0] = 0;
            indexAvailable[0] = false;
        }
        Skeleton *skel1 = TheGestureMgr->GetSkeletonByTrackingID(
            TheGameData->Player(1)->GetSkeletonTrackingID()
        );
        if (skel1) {
            int idx = skel1->SkeletonIndex() + 1;
            indices[1] = idx;
            indexAvailable[idx - 1] = false;
        } else {
            indices[1] = 0;
            indexAvailable[1] = false;
        }
        for (int i = 0; i < NUM_SKELETONS; i++) {
            if (indices[i] == -1) {
                for (int j = 0; j < NUM_SKELETONS; j++) {
                    if (indexAvailable[j]) {
                        indexAvailable[j] = false;
                        indices[i] = j + 1;
                        break;
                    }
                }
            }
        }
    }

    bool CheckTexType(RndTex *tex) {
        MILO_ASSERT(tex, 0x45);

        if ((tex->GetType() & RndTex::kRendered) == 0) {
            MILO_NOTIFY_ONCE("%s not renderable", tex->Name());
            return false;
        }
        return true;
    }

    RndTex *SetPaletteTexture(RndTex *tex, enum StreamDisplay display) {
        if (tex) {
            return tex;
        }
        if ((((display != kStreamPlayerDepthShell)
              && (display != kStreamPlayerDepthShell2))
             && (display != kStreamPlayerGreenscreen))
            && (display != kStreamPlayerDepthGreenscreen)) {
            return TheRnd.GetDefaultTex(1);
        }
        return TheRnd.GetDefaultTex(2);
    }
};

StreamRenderer::StreamRenderer()
    : mOutputTexture(this), mForceMips(false), mDisplay(kStreamColor), mNumBlurs(4),
      mPlayer1Col(1, 1, 1), mPlayer2Col(0.48, 0.57, 0.8), mPlayer3Col(0.05, 0.06, 0.54),
      mPlayer4Col(0, 0, 0, 0), mPlayer5Col(0, 0, 0, 0), mPlayer6Col(0, 0, 0, 0),
      mNobodyCol(0, 0, 0, 0), mPlayer1Palette(this), mPlayer2Palette(this),
      mPlayerOtherPalette(this), mBackgroundPalette(this), mPlayer1PaletteOffset(0),
      mPlayer2PaletteOffset(0), mPlayerOtherPaletteOffset(0), mBackgroundPaletteOffset(0),
      mDrawPreClear(0), mForceDraw(0), mStaticColorIndexing(0), mTestTex(this),
      mLagPrimaryTexture(0), mLaggedPingPong(0),
      gCrewPhotoPlayersDetected0123(0, 0, 0, 0), gCrewPhotoPlayersDetected45(0, 0, 0, 0) {
    for (int i = 0; i < NUM_SKELETONS; i++) {
        mCenters[i].SetSmoothParameters(6, 0);
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
    SYNC_PROP_MODIFY(output_texture, mOutputTexture, SetOutputTex())
    SYNC_PROP_MODIFY(force_mips, mForceMips, SetOutputTex())
    SYNC_PROP(display, (int &)mDisplay)
    SYNC_PROP(num_blurs, mNumBlurs)
    SYNC_PROP(player_depth_nobody, mNobodyCol)
    SYNC_PROP(player_depth_nobody_alpha, mNobodyCol.alpha)
    SYNC_PROP(player1_depth_color, mPlayer1Col)
    SYNC_PROP(player1_depth_color_alpha, mPlayer1Col.alpha)
    SYNC_PROP(player2_depth_color, mPlayer2Col)
    SYNC_PROP(player2_depth_color_alpha, mPlayer2Col.alpha)
    SYNC_PROP(player3_depth_color, mPlayer3Col)
    SYNC_PROP(player3_depth_color_alpha, mPlayer3Col.alpha)
    SYNC_PROP(player4_depth_color, mPlayer4Col)
    SYNC_PROP(player4_depth_color_alpha, mPlayer4Col.alpha)
    SYNC_PROP(player5_depth_color, mPlayer5Col)
    SYNC_PROP(player5_depth_color_alpha, mPlayer5Col.alpha)
    SYNC_PROP(player6_depth_color, mPlayer6Col)
    SYNC_PROP(player6_depth_color_alpha, mPlayer6Col.alpha)
    SYNC_PROP(player1_depth_palette, mPlayer1Palette)
    SYNC_PROP(player1_depth_palette_offset, mPlayer1PaletteOffset)
    SYNC_PROP(player2_depth_palette, mPlayer2Palette)
    SYNC_PROP(player2_depth_palette_offset, mPlayer2PaletteOffset)
    SYNC_PROP(player_other_depth_palette, mPlayerOtherPalette)
    SYNC_PROP(player_other_depth_palette_offset, mPlayerOtherPaletteOffset)
    SYNC_PROP(background_depth_palette, mBackgroundPalette)
    SYNC_PROP(background_depth_palette_offset, mBackgroundPaletteOffset)
    SYNC_PROP(force_draw, mForceDraw)
    SYNC_PROP(static_color_indices, mStaticColorIndexing)
    SYNC_PROP_MODIFY(draw_pre_clear, mDrawPreClear, UpdatePreClearState())
    SYNC_PROP(pc_test_texture, mTestTex)
    SYNC_PROP(lag_primary_texture, mLagPrimaryTexture)
    SYNC_PROP(crew_photo_edge_iterations, gCrewPhotoEdgeIterations)
    SYNC_PROP(crew_photo_edge_offset, gCrewPhotoEdgeOffset)
    SYNC_PROP(crew_photo_horizontal_color, gCrewPhotoHorizontalColor)
    SYNC_PROP(crew_photo_vertical_color, gCrewPhotoVerticalColor)
    SYNC_PROP(crew_photo_blur_start, gCrewPhotoBlurStart)
    SYNC_PROP(crew_photo_blur_width, gCrewPhotoBlurWidth)
    SYNC_PROP(crew_photo_blur_iterations, gCrewPhotoBlurIterations)
    SYNC_PROP(crew_photo_background_brightness, gCrewPhotoBackgroundBrightness)
    SYNC_SUPERCLASS(RndDrawable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(StreamRenderer)
    SAVE_REVS(12, 1)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(RndDrawable)
    bs << mOutputTexture;
    bs << mForceMips;
    bs << mDisplay;
    bs << mPlayer1Col << mPlayer2Col << mPlayer3Col << mNobodyCol;
    bs << mPlayer1Palette << mPlayer1PaletteOffset;
    bs << mBackgroundPalette << mBackgroundPaletteOffset;
    bs << mNumBlurs;
    bs << mTestTex;
    bs << mPlayer2Palette << mPlayer2PaletteOffset;
    bs << mPlayerOtherPalette << mPlayerOtherPaletteOffset;
    bs << mDrawPreClear << mForceDraw;
    bs << mLagPrimaryTexture;
    bs << mPlayer4Col << mPlayer5Col << mPlayer6Col;
    bs << mStaticColorIndexing;
    bs << gCrewPhotoEdgeIterations;
    bs << gCrewPhotoEdgeOffset;
    bs << gCrewPhotoHorizontalColor;
    bs << gCrewPhotoVerticalColor;
    bs << gCrewPhotoBlurStart;
    bs << gCrewPhotoBlurWidth;
    bs << gCrewPhotoBlurIterations;
    bs << gCrewPhotoBackgroundBrightness;
END_SAVES

BEGIN_COPYS(StreamRenderer)
    COPY_SUPERCLASS(Hmx::Object)
    COPY_SUPERCLASS(RndDrawable)
    CREATE_COPY(StreamRenderer)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mOutputTexture)
        COPY_MEMBER(mForceMips)
        COPY_MEMBER(mDisplay)
        COPY_MEMBER(mPlayer1Col)
        COPY_MEMBER(mPlayer2Col)
        COPY_MEMBER(mPlayer3Col)
        COPY_MEMBER(mPlayer4Col)
        COPY_MEMBER(mPlayer5Col)
        COPY_MEMBER(mPlayer6Col)
        COPY_MEMBER(mNobodyCol)
        COPY_MEMBER(mPlayer1Palette)
        COPY_MEMBER(mPlayer1PaletteOffset)
        COPY_MEMBER(mPlayer2Palette)
        COPY_MEMBER(mPlayer2PaletteOffset)
        COPY_MEMBER(mPlayerOtherPalette)
        COPY_MEMBER(mPlayerOtherPaletteOffset)
        COPY_MEMBER(mBackgroundPalette)
        COPY_MEMBER(mBackgroundPaletteOffset)
        COPY_MEMBER(mNumBlurs)
        COPY_MEMBER(mTestTex)
        COPY_MEMBER(mDrawPreClear)
        COPY_MEMBER(mForceDraw)
        COPY_MEMBER(mLagPrimaryTexture)
        COPY_MEMBER(gCrewPhotoEdgeIterations)
        COPY_MEMBER(gCrewPhotoEdgeOffset)
        COPY_MEMBER(gCrewPhotoHorizontalColor)
        COPY_MEMBER(gCrewPhotoVerticalColor)
        COPY_MEMBER(gCrewPhotoBlurStart)
        COPY_MEMBER(gCrewPhotoBlurWidth)
        COPY_MEMBER(gCrewPhotoBlurIterations)
        COPY_MEMBER(gCrewPhotoBackgroundBrightness)
        SetOutputTex();
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(12, 1)

BEGIN_LOADS(StreamRenderer)
    LOAD_REVS(bs)
    ASSERT_REVS(12, 1)
    LOAD_SUPERCLASS(Hmx::Object)
    LOAD_SUPERCLASS(RndDrawable)
    d >> mOutputTexture;
    SetOutputTex();
    d >> mForceMips;
    d >> (int &)mDisplay;
    if (d.rev > 1) {
        d >> mPlayer1Col >> mPlayer2Col >> mPlayer3Col >> mNobodyCol;
        d >> mPlayer1Palette >> mPlayer1PaletteOffset;
    }
    if (d.rev > 2) {
        d >> mBackgroundPalette >> mBackgroundPaletteOffset;
    }
    if (d.rev > 3) {
        d >> mNumBlurs;
    }
    if (d.rev > 4) {
        d >> mTestTex;
    }
    if (d.rev > 5) {
        d >> mPlayer2Palette;
        d >> mPlayer2PaletteOffset;
    } else {
        mPlayer2Palette = mPlayer1Palette;
        mPlayer2PaletteOffset = mPlayer1PaletteOffset;
    }
    if (d.rev > 6) {
        d >> mPlayerOtherPalette;
        d >> mPlayerOtherPaletteOffset;
    } else {
        mPlayerOtherPalette = mPlayer2Palette;
        mPlayerOtherPaletteOffset = mPlayer2PaletteOffset;
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
        d >> mPlayer4Col >> mPlayer5Col >> mPlayer6Col;
    } else {
        mPlayer4Col = mPlayer3Col;
        mPlayer5Col = mPlayer3Col;
        mPlayer6Col = mPlayer3Col;
    }
    if (d.rev > 10) {
        d >> mStaticColorIndexing;
    } else {
        mStaticColorIndexing = false;
    }
    if (d.rev > 11) {
        d >> gCrewPhotoEdgeIterations;
        d >> gCrewPhotoEdgeOffset;
        d >> gCrewPhotoHorizontalColor;
        d >> gCrewPhotoVerticalColor;
        d >> gCrewPhotoBlurStart;
        d >> gCrewPhotoBlurWidth;
        d >> gCrewPhotoBlurIterations;
        d >> gCrewPhotoBackgroundBrightness;
    } else {
        gCrewPhotoEdgeIterations = 0;
        gCrewPhotoEdgeOffset = 0;
        gCrewPhotoBlurStart = 0;
        gCrewPhotoBlurWidth = 0;
        gCrewPhotoBlurIterations = 0;
        gCrewPhotoBackgroundBrightness = 0;
        gCrewPhotoHorizontalColor = 0;
        gCrewPhotoVerticalColor = 0;
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

void StreamRenderer::SetPinkPlayer(int player) { gCrewPhotoPlayersDetected45.z = player; }
void StreamRenderer::SetBluePlayer(int player) { gCrewPhotoPlayersDetected45.w = player; }

DataNode StreamRenderer::OnGetRenderTextures(DataArray *) {
    return GetRenderTextures(Dir());
}

void StreamRenderer::SetCrewPhotoHorizontalColor(DataArray *cfg) {
    if (cfg->Size() == 3) {
        gCrewPhotoHorizontalColor.red = cfg->Float(0);
        gCrewPhotoHorizontalColor.green = cfg->Float(1);
        gCrewPhotoHorizontalColor.blue = cfg->Float(2);
    }
}

void StreamRenderer::SetCrewPhotoVerticalColor(DataArray *cfg) {
    if (cfg->Size() == 3) {
        gCrewPhotoVerticalColor.red = cfg->Float(0);
        gCrewPhotoVerticalColor.green = cfg->Float(1);
        gCrewPhotoVerticalColor.blue = cfg->Float(2);
    }
}

ShaderType StreamRenderer::GetShaderType() const {
    ShaderType t = kDrawRectShader;
    switch (mDisplay) {
    case kStreamColor:
        t = kYUVtoRGBShader;
        break;
    case kStreamBlackAndWhite:
        t = kYUVtoBlackAndWhiteShader;
        break;
    case kStreamBasicDepth:
        t = kDrawRectShader;
        break;
    case kStreamPlayerDepthVis:
        t = kPlayerDepthVisShader;
        break;
    case kStreamPlayerDepthShell:
        t = kPlayerDepthShellShader;
        break;
    case kStreamPlayerDepthShell2:
        t = kPlayerDepthShell2Shader;
        break;
    case kStreamPlayerGreenscreen:
        t = kPlayerGreenScreenShader;
        break;
    case kStreamPlayerDepthGreenscreen:
        t = kPlayerDepthGreenScreenShader;
        break;
    case kStreamCrewPhoto:
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
        gCrewPhotoPlayersDetected0123.x = set;
        break;
    case 1:
        gCrewPhotoPlayersDetected0123.y = set;
        break;
    case 2:
        gCrewPhotoPlayersDetected0123.z = set;
        break;
    case 3:
        gCrewPhotoPlayersDetected0123.w = set;
        break;
    case 4:
        gCrewPhotoPlayersDetected45.x = set;
        break;
    case 5:
        gCrewPhotoPlayersDetected45.y = set;
        break;
    default:
        break;
    }
}

void StreamRenderer::SetCrewPhotoPlayerCenters() {
    for (int i = 0; i < NUM_SKELETONS; i++) {
        Skeleton *skel = TheGestureMgr->GetSkeletonByTrackingID(
            TheGestureMgr->GetSkeleton(i).TrackingID()
        );
        float min_x = 1;
        float min_y = 1;
        float min_z = 1;
        float max_x = -1;
        float max_y = -1;
        float max_z = -1;
        if (skel) {
            for (int j = 0; j < kNumJoints; j++) {
                Vector2 pos;
                skel->ScreenPos((SkeletonJoint)j, pos);
                float camZ = skel->TrackedJoints()[j].mRawCameraPos.z;
                MinEq(min_x, pos.x);
                MaxEq(max_x, pos.x);
                MinEq(min_y, pos.y);
                MaxEq(max_y, pos.y);
                MinEq(min_z, camZ);
                MaxEq(max_z, camZ);
            }
            Vector3 vres((min_x + max_x) / 2, (min_y + max_y) / 2, (min_z + max_z) / 2);
            mCenters[i].Smooth(vres, TheTaskMgr.DeltaUISeconds(), false);
            gCrewPhotoPlayerCenter[i] = mCenters[i].Value();
        } else {
            gCrewPhotoPlayerCenter[i] = Vector3(0, 0, 0);
        }
    }
}

void StreamRenderer::DrawToTexture() {
    if (TheRnd.DrawMode() != Rnd::kDrawNormal)
        return;
    if (!Showing())
        return;
    if (mLagPrimaryTexture && mLaggedPrimaryTexture[0] == NULL) {
        MILO_ASSERT(mLaggedPrimaryTexture[1] == NULL, 245);
        mLaggedPrimaryTexture[0] = New<RndTex>();
        mLaggedPrimaryTexture[0]->SetBitmap(
            320, 240, 16, RndTex::kRegularLinear, false, nullptr
        );
        mLaggedPrimaryTexture[1] = New<RndTex>();
        mLaggedPrimaryTexture[1]->SetBitmap(
            320, 240, 16, RndTex::kRegularLinear, false, nullptr
        );
    }
    if (mOutputTexture == nullptr)
        return;
    TheDxRnd.SetShaderRegisterAlloc(static_cast<DxRnd::RegisterAlloc>(3));
    LiveCameraInput *cam = TheGestureMgr->GetLiveCameraInput();
    int stream = DisplayStreams[mDisplay];
    int stream_wat = (stream & 2) >> 1;
    if (stream & 1 && !cam->Unk11EA()) {
        cam->PollNewStream(LiveCameraInput::kBufferDepth);
    } else if ((stream_wat & 0xff) != 0 && !cam->Unk11E9()) {
        cam->PollNewStream(LiveCameraInput::kBufferColor);
    }

    // 0x254
    const RndTex *cam_tex = RndCam::Current()->TargetTex();
    if (cam_tex != nullptr) {
        MILO_NOTIFY_ONCE(
            "%s: Cannot render to texture (%s) while already rendering to texture (%s).",
            PathName(cam_tex),
            PathName(this),
            PathName(cam_tex)
        );
    }
    mCam->SetTargetTex(nullptr);

    // 0x390
    TheRenderState.SetTextureFilter(0, RndRenderState::kFilterModePoint, false);
    TheRenderState.SetTextureClamp(0, RndRenderState::kClampModeClamp);
    RndTex *pal = SetPaletteTexture(mPlayer1Palette, mDisplay);
    TheShaderMgr.SetPConstant(static_cast<PShaderConstant>(10), pal);

    TheRenderState.SetTextureFilter(10, RndRenderState::kFilterModeLinear, false);
    TheRenderState.SetTextureClamp(10, RndRenderState::kClampModeWrap);
    pal = SetPaletteTexture(mPlayer2Palette, mDisplay);
    TheShaderMgr.SetPConstant(static_cast<PShaderConstant>(12), pal);

    TheRenderState.SetTextureFilter(12, RndRenderState::kFilterModeLinear, false);
    TheRenderState.SetTextureClamp(12, RndRenderState::kClampModeWrap);
    pal = SetPaletteTexture(mPlayerOtherPalette, mDisplay);
    TheShaderMgr.SetPConstant(static_cast<PShaderConstant>(14), pal);

    TheRenderState.SetTextureFilter(14, RndRenderState::kFilterModeLinear, false);
    TheRenderState.SetTextureClamp(14, RndRenderState::kClampModeWrap);
    pal = SetPaletteTexture(mPlayerOtherPalette, mDisplay);
    TheShaderMgr.SetPConstant(static_cast<PShaderConstant>(11), pal);

    TheRenderState.SetTextureFilter(11, RndRenderState::kFilterModeLinear, false);
    TheRenderState.SetTextureClamp(11, RndRenderState::kClampModeWrap);
    Vector4 v4;
    v4.x = mNobodyCol.red;
    v4.y = mNobodyCol.green;
    v4.z = mNobodyCol.blue;
    v4.w = mNobodyCol.alpha;
    TheShaderMgr.SetPConstant(static_cast<PShaderConstant>(64), v4);
}
