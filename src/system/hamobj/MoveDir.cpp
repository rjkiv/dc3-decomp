#include "hamobj/MoveDir.h"
#include "FilterQueue.h"
#include "HamMaster.h"
#include "MoveDir.h"
#include "ScoreUtl.h"
#include "char/Character.h"
#include "flow/PropertyEventProvider.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonClip.h"
#include "gesture/SkeletonDir.h"
#include "gesture/SkeletonUpdate.h"
#include "gesture/SkeletonViz.h"
#include "gesture/StubCameraInput.h"
#include "hamobj/CharFeedback.h"
#include "hamobj/DancerSequence.h"
#include "hamobj/DancerSkeleton.h"
#include "hamobj/DetectFrame.h"
#include "hamobj/Difficulty.h"
#include "hamobj/ErrorNode.h"
#include "hamobj/FilterVersion.h"
#include "hamobj/HamAudio.h"
#include "hamobj/HamCharacter.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamMove.h"
#include "hamobj/HamPhraseMeter.h"
#include "hamobj/HamPlayerData.h"
#include "hamobj/MoveDetector.h"
#include "hamobj/MoveMgr.h"
#include "hamobj/PracticeSection.h"
#include "hamobj/ScoreUtl.h"
#include "hamobj/SongCollision.h"
#include "math/Color.h"
#include "math/Geo.h"
#include "math/Vec.h"
#include "meta/SongMetadata.h"
#include "meta/SongMgr.h"
#include "obj/Data.h"
#include "obj/DataFile.h"
#include "obj/DataFunc.h"
#include "obj/DataUtl.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Task.h"
#include "obj/Utl.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "rndobj/Dir.h"
#include "rndobj/Draw.h"
#include "rndobj/Font.h"
#include "rndobj/FontBase.h"
#include "rndobj/Overlay.h"
#include "rndobj/Rnd.h"
#include "rndobj/Utl.h"
#include "ui/PanelDir.h"
#include "ui/ResourceDirPtr.h"
#include "ui/UILabelDir.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/MakeString.h"
#include "utl/SongInfoCopy.h"
#include "utl/Std.h"
#include "utl/Symbol.h"
#include "utl/TimeConversion.h"
#include "world/Dir.h"
#include "xdk/XAPILIB.h"

float MoveDir::sLatencySeconds = 0;
float MoveDir::sPLFMinTimeError = 0;
bool MoveDir::sGameRecord = false;
bool MoveDir::sGameRecord2Player = false;

std::vector<FilterVersion *> MoveDir::sFilterVersions;
static float sFloat = 0.0f;

namespace {
    static Hmx::Rect sRect(0.05f, 0.95f, 0.25f, 0.25f);
    static Hmx::Color sGray(0.5, 0.5, 0.5, 1);
    static Hmx::Color sGreen(0, 0.6, 0, 0.5);
    static Hmx::Color sDarkerGray(0.3, 0.3, 0.3, 0.8);
    static Hmx::Color sLightGray(0.8, 0.8, 0.8, 1);
    static Hmx::Color sDarkGray(0.3, 0.3, 0.3, 0.6);

    float sOverlayWidth = sRect.y - sRect.x;

    float DrawOverlayBar(float f1, float f2, float f3, const Hmx::Color &c, float f4) {
        TheRnd.DrawRectScreen(
            Hmx::Rect(f2, f1, (f3 - f2), f4), c, nullptr, nullptr, nullptr
        );
        UtilDrawLine(Vector2(f2, f1), Vector2(f2, f1 + f4), sGray);
        UtilDrawLine(Vector2(f3, f1), Vector2(f3, f1 + f4), sGray);
        return f4;
    }

    float DrawDetectedBar(
        float f1, const char *c2, float f3, float f4, float f5, bool b6, bool b7
    ) {
        Hmx::Color gray = sDarkerGray;
        Hmx::Color green = sGreen;
        Hmx::Color lightGray = sLightGray;
        if (b6) {
            Multiply(sDarkerGray, 0.5f, gray);
            gray.alpha = sDarkerGray.alpha;
            Multiply(sGreen, 0.5f, green);
            green.alpha = sGreen.alpha;
            Multiply(sLightGray, 0.5f, lightGray);
            lightGray.alpha = sLightGray.alpha;
        }
        String str(c2);
        if (b7) {
            str += MakeString(": %.2f%%", f3 * 100);
        } else {
            str += MakeString(": %.2f", f3);
        }
        float fvar5 = (f5 - f4) * f3;
        DrawOverlayBar(f1, f4, (f5 - f4) + f4, gray, sFloat);
        DrawOverlayBar(f1, f4, fvar5 + f4, green, sFloat);
        TheRnd.DrawStringScreen(str.c_str(), Vector2(f4, f1), lightGray, true);
        return sFloat + f1;
    }

    void DrawBeatLine(float f1, float f2, float f3, const Hmx::Color &c) {
        Vector2 v20;
        v20.x = Interp(sRect.x, sRect.y, (sRect.w + f3) / (sRect.h + sRect.w + 4));
        v20.y = f1 + f2;
        Vector2 v18;
        v18.x = v20.x;
        v18.y = f1;
        UtilDrawLine(v18, v20, c);
    }

    float DrawPlayClip(float f, SkeletonClip *clip, int bar) {
        MILO_ASSERT(clip, 0x762);
        String clipName = clip->Name();
        if (bar < clip->NumMoveRatings()) {
            auto &rating = clip->GetMoveRating(bar);
            clipName += MakeString(
                " (bar %i: expected=%s)",
                bar,
                rating.mExpected.Null() ? "<none>" : rating.mExpected
            );
        } else {
            clipName += " (no rating overrides)";
        }
        Hmx::Rect rect(0.01f, f, 0.9f, sFloat);
        TheRnd.DrawRectScreen(rect, sDarkGray, nullptr, nullptr, nullptr);
        const Vector2 &stringScreen = TheRnd.DrawStringScreen(
            clipName.c_str(), Vector2(0.01f, f), sLightGray, true
        );
        return stringScreen.y;
    }

    namespace {
        struct DetectFrameSecondsCmp {
            bool operator()(const DetectFrame &frame, const float &f) const {
                return frame.Seconds() < f;
            }
        };
    }

}

String RecordClipName(const char *cc, int i2) {
    DateTime dt;
    GetDateAndTime(dt);
    Difficulty playerDiff = TheGameData->Player(0)->GetDifficulty();
    char diff;
    if (playerDiff == kDifficultyExpert) {
        diff = 'h';
    } else {
        diff = DifficultyToSym(playerDiff).Str()[0];
    }
    const char *prefix = "";
    switch (i2) {
    case 1:
        prefix = "b";
        break;
    case 2:
        prefix = "c";
        break;
    case 3:
        prefix = "d";
        break;
    default:
        break;
    }
    unsigned int dtCode = dt.ToCode();
    Symbol song = TheGameData->GetSong();
    const String &x = TheGameData->Player(0)->Unk2c();
    String ret(MakeString("%s%d~%s~%c~%s~%s", prefix, dtCode, song, diff, x, cc));
    if (ret.length() > 38) {
        ret.resize(38);
    }
    return ret;
}

MoveMode CurrentMoveMode() {
    MILO_ASSERT(TheHamDirector, 0x79);
    return TheHamDirector->InPracticeMode() ? (MoveMode)1 : (MoveMode)0;
}

void SetupRecordClip(
    ObjPtr<SkeletonClip> &clip, int i1, int i2, const char *cc, ObjectDir *dir
) {
    clip = Hmx::Object::New<SkeletonClip>();
    clip->EnableAlternateRecord(i1);
    clip->SetUnk11fc(i2);
    String clipName = RecordClipName(cc, i1);
    clipName += ".clp";
    clip->SetName(clipName.c_str(), dir);
    const char *path = MakeString("devkit:\\%s", clip->Name());
    MILO_LOG("Starting song recording: %s\n", path);
    clip->StartXboxRecording(path);
}

MoveDir::MoveDir()
    : mShowMoveOverlay(0), mErrorNodeInfo(0), mPlayClip(this), mRecordClip(this),
      unk2bc(this), unk2d0(this), unk2e4(0), mReportMove(this), mFiltersEnabled(0),
      mGamePanel(0), unk30c(0), mFilterQueue(0), mAsyncDetector(0), mUpdateLoader(0),
      mFinishingMoveMeasure(10000), mMoveOverlay(RndOverlay::Find("ham_move")),
      mDancerSeq(this), unk414(0), mDancerViz(Hmx::Object::New<SkeletonViz>()), unk41c(0),
      mDebugLatencyOffset(0), mDebugLoop(0), mLastPollMs(0), mDebugCollision(0),
      unkf84(-1) {
    for (int i = 0; i < 2; i++) {
        mMovePlayerData[i].Reset();
        mCurMoveSmoothers[i].SetCoeffs(1, 0);
        filler[i] = 0;
        mCurMoveNormalizedResult[i] = 0;
        unk3e8[i] = 0;
        mCurMove[i] = 0;
        mCurMoveRating[i] = kMoveRatingOk;
        unk3f0[i] = kMoveRatingOk;
        unkf04[i].Reset();
    }
    SetFilterVersion("ham2");
}

MoveDir::~MoveDir() {
    RELEASE(mFilterQueue);
    RELEASE(mAsyncDetector);
    mMoveOverlay = RndOverlay::Find("ham_move", false);
    if (mMoveOverlay && mMoveOverlay->GetCallback() == this) {
        mMoveOverlay->SetCallback(nullptr);
        if (TheLoadMgr.EditMode()) {
            mMoveOverlay->SetShowing(false);
        }
    }
    delete mDancerViz;
    if (SkeletonUpdate::HasInstance()) {
        SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
        if (handle.HasCallback(this)) {
            handle.RemoveCallback(this);
        }
    }
}

BEGIN_HANDLERS(MoveDir)
    HANDLE_ACTION(start_song_record, 0)
    HANDLE_ACTION(stop_song_record, StopSongRecord())
    HANDLE_ACTION(
        simulate_song,
        SimulateSong(
            _msg->Size() > 2 ? _msg->Int(2) : 0, _msg->Size() > 3 ? _msg->Int(3) : 0
        )
    )
    HANDLE_ACTION(reload_scoring, ReloadScoring())
    HANDLE_ACTION(reset_detection, ResetDetection())
    HANDLE(stream_jump, OnStreamJump)
    HANDLE_EXPR(import_clip, ImportClip(_msg->Int(2)))
    HANDLE_ACTION(debug_rotate, mDancerViz->Rotate(_msg->Float(2)))
    // these don't appear to be inlined methods
    {
        static Symbol _s("disable_all_detectors");
        if (sym == _s) {
            MILO_ASSERT(mAsyncDetector, 0x136F);
            mAsyncDetector->DisableAllDetectors();
            return 0;
        }
    }
    {
        static Symbol _s("enable_detector");
        if (sym == _s) {
            MILO_ASSERT(mAsyncDetector, 0x1371);
            mAsyncDetector->EnableDetector(_msg->Obj<HamMove>(2));
            return 0;
        }
    }
    {
        static Symbol _s("disable_detector");
        if (sym == _s) {
            MILO_ASSERT(mAsyncDetector, 0x1373);
            mAsyncDetector->DisableDetector(_msg->Obj<HamMove>(2));
            return 0;
        }
    }
    HANDLE_EXPR(
        active_detector_result,
        mAsyncDetector->MoveRatingFrac(
            _msg->Int(2), (MoveAsyncDetector::RatingBar)0, _msg->Obj<HamMove>(3)
        )
    )
    HANDLE_EXPR(
        last_detector_result,
        mAsyncDetector->MoveRatingFrac(
            _msg->Int(2), (MoveAsyncDetector::RatingBar)1, _msg->Obj<HamMove>(3)
        )
    )
    HANDLE_EXPR(cur_move_normalized_result, mCurMoveNormalizedResult[_msg->Int(2)])
    HANDLE_EXPR(
        active_detector_looped_result,
        mAsyncDetector->MoveRatingFrac(
            _msg->Int(2), (MoveAsyncDetector::RatingBar)2, _msg->Obj<HamMove>(3)
        )
    )
    HANDLE_EXPR(
        cur_move_normalized_result_smoothed, mCurMoveSmoothers[_msg->Int(2)].Level()
    )
    HANDLE_ACTION(
        detector_clear_looped_result,
        mAsyncDetector->ClearLoopedRatingFrac(_msg->Obj<HamMove>(2))
    )
    HANDLE_EXPR(get_cur_move, mCurMove[_msg->Int(2)])
    HANDLE_EXPR(get_cur_measure, MoveIdx())
    HANDLE_EXPR(get_cur_beat, TheTaskMgr.TotalBeat())
    HANDLE_EXPR(get_finishing_move_measure, mFinishingMoveMeasure)
    HANDLE_ACTION(clear_limb_feedback, ClearLimbFeedback(_msg->Int(2)))
    HANDLE_ACTION(beat, OnBeat())
    HANDLE_SUPERCLASS(SkeletonDir)
END_HANDLERS

BEGIN_PROPSYNCS(MoveDir)
    SYNC_PROP_SET(current_move, mMovePlayerData[0].mCurMove.Ptr(), )
    SYNC_PROP_SET(filters_enabled, mFiltersEnabled, SetFiltersEnabled(_val.Int()))
    SYNC_PROP_SET(move_overlay, mShowMoveOverlay, SetMoveOverlay(_val.Int()))
    SYNC_PROP(debug_latency_offset, mDebugLatencyOffset)
    SYNC_PROP_SET(
        debug_skeleton_rotation,
        mDancerViz->PhysicalCamRotation(),
        mDancerViz->SetPhysicalCamRotation(_val.Float())
    )
    SYNC_PROP(debug_collision, mDebugCollision)
    SYNC_PROP(debug_node_types, mErrorNodeInfo)
    SYNC_PROP(debug_node_joints, mErrorNodeInfo)
    SYNC_PROP_SET(play_clip, mPlayClip.Ptr(), SetSongPlayClip(_val.Obj<SkeletonClip>()))
    SYNC_PROP(report_move, mReportMove)
    SYNC_PROP(record_clip, mRecordClip)
    SYNC_PROP(import_clip_path, mImportClipPath)
    SYNC_SUPERCLASS(SkeletonDir)
END_PROPSYNCS

BEGIN_SAVES(MoveDir)
    SAVE_REVS(35, 0)
    SAVE_SUPERCLASS(SkeletonDir)
    if (IsProxy()) {
        bs << mFiltersEnabled;
    }
    bs << mShowMoveOverlay;
    bs << mErrorNodeInfo;
    if (!bs.Cached()) {
        bs << mImportClipPath;
    } else {
        bs << 0;
    }
    MILO_ASSERT(mFilterVer, 0x922);
    bs << mFilterVer->mVersionSym;
END_SAVES

BEGIN_COPYS(MoveDir)
    COPY_SUPERCLASS(SkeletonDir)
    CREATE_COPY(MoveDir)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mShowMoveOverlay)
        COPY_MEMBER(mErrorNodeInfo)
        COPY_MEMBER(mImportClipPath)
        COPY_MEMBER(mFiltersEnabled)
        COPY_MEMBER(mPlayClip)
        COPY_MEMBER(mRecordClip)
        COPY_MEMBER(unk2bc)
        COPY_MEMBER(unk2d0)
        COPY_MEMBER(mReportMove)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(MoveDir)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

INIT_REVS(0x23, 0)

void MoveDir::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(0x23, 0)
    if (d.rev < 9) {
        RndDir::PreLoad(d.stream);
    } else {
        SkeletonDir::PreLoad(d.stream);
    }
    Symbol song = TheGameData->GetSong();
    if (!IsProxy() && gLoadingProxyFromDisk && !song.Null()) {
        SongMgr *songMgr = ObjectDir::Main()->Find<SongMgr>("song_mgr", false);
        if (songMgr) {
            const SongMetadata *songData =
                songMgr->Data(songMgr->GetSongIDFromShortName(song, true));
            if (songData->Version() < 11) {
                mUpdateLoader = dynamic_cast<DirLoader *>(TheLoadMgr.AddLoader(
                    FilePath(FileRoot(), songMgr->SongFilePath(song, "_update.milo", 11)),
                    kLoadFront
                ));
            }
        }
    }
    d.PushRev(this);
}

void MoveDir::PostLoad(BinStream &bs) {
    BinStreamRev d(bs, bs.PopRev(this));
    if (d.rev < 9) {
        RndDir::PostLoad(d.stream);
    } else {
        SkeletonDir::PostLoad(d.stream);
    }
    if (d.rev < 5) {
        bool b;
        d >> b;
    }
    if (d.rev > 0 && d.rev < 2) {
        String str;
        d >> str;
    }
    if (!IsProxy() || d.rev < 8) {
        if (d.rev > 3 && d.rev < 9) {
            String str;
            d >> str;
        }
        if (d.rev > 5 && d.rev < 32) {
            if (d.rev > 0x1A) {
                ObjPtrVec<HamMove> moves(this, (EraseMode)0, kObjListAllowNull);
                d >> moves;
            } else {
                ObjPtr<HamMove> move(this);
                d >> move;
            }
        }
    }
    if (IsProxy() && d.rev > 10 && d.rev < 0xD) {
        ObjPtr<Character> character(this);
        WorldDir *wDir = TheHamDirector ? TheHamDirector->GetVenueWorld() : nullptr;
        character.Load(d.stream, true, wDir);
    }
    if (IsProxy() && d.rev > 0xC) {
        d >> mFiltersEnabled;
    }
    char buf[0x80];
    if (d.rev < 0x23) {
        if (IsProxy() && d.rev > 0xE) {
            d.stream.ReadString(buf, 0x80);
        }
        if (IsProxy() && d.rev > 0xD) {
            d.stream.ReadString(buf, 0x80);
        }
        if (IsProxy() && d.rev > 0x17) {
            d.stream.ReadString(buf, 0x80);
        }
    }
    if (d.rev > 6) {
        if (d.rev > 9 && d.rev < 18) {
            int x;
            d >> x;
            mShowMoveOverlay = x;
        } else {
            d >> mShowMoveOverlay;
        }
    }
    if (d.rev > 0xF && d.rev < 0x1F) {
        bool b;
        d >> b;
    }
    if (d.rev > 0x16 && d.rev < 0x22) {
        bool b;
        d >> b;
    }
    if (d.rev > 0x14) {
        if (d.rev > 0x1B) {
            d >> mErrorNodeInfo;
        } else {
            Symbol s;
            d >> s;
        }
    } else if (d.rev > 0x11) {
        int x;
        d >> x;
    }
    if (d.rev > 0x15 && d.rev < 0x1D) {
        bool b;
        d >> b;
    }
    if (d.rev > 0x19 && d.rev < 0x21) {
        int x;
        d >> x;
        bool b;
        d >> b;
        int y, z;
        d >> y >> z;
        for (int i = 0; i < 3; i++) {
            d >> b >> b;
        }
    }
    if (d.rev > 0x13) {
        d >> mImportClipPath;
    }
    if (d.rev < 0x19) {
        if (d.rev > 0x14) {
            int x;
            d >> x;
            Symbol s;
            for (int i = 0; i < x; i++) {
                int n;
                d >> s >> n;
            }
        } else if (d.rev > 0xB) {
            int max = 5;
            if (d.rev < 0x11) {
                max = 4;
            }
            for (int i = 0; i < max; i++) {
                int x;
                d >> x;
            }
        }
    }
    Symbol filterVersion;
    static Symbol ham1("ham1");
    static Symbol ham2("ham2");
    if (d.rev < 0x1A) {
        filterVersion = ham1;
    } else if (d.rev < 0x1E) {
        filterVersion = ham2;
    } else {
        d >> filterVersion;
    }
    SetFilterVersion(filterVersion);
    if (mUpdateLoader) {
        ObjectDir *loaderDir = mUpdateLoader->GetDir();
        RELEASE(mUpdateLoader);
        if (loaderDir) {
            for (ObjDirItr<Hmx::Object> it(loaderDir, true); it != nullptr; ++it) {
                Hmx::Object *cur = it;
                if (cur != loaderDir) {
                    const char *curName = cur->Name();
                    HamMove *move = dynamic_cast<HamMove *>(cur);
                    if (move) {
                        HamMove *find = Find<HamMove>(curName, false);
                        if (find) {
                            find->Update(move);
                        }
                    } else {
                        ObjectDir *dir = dynamic_cast<ObjectDir *>(cur);
                        if (dir && !*dir->GetPathName()) {
                            continue;
                        } else {
                            Hmx::Object *find = Find<Hmx::Object>(curName, false);
                            if (find) {
                                delete find;
                            }
                            it->SetName(curName, this);
                        }
                    }
                }
            }
            delete loaderDir;
        } else {
            MILO_NOTIFY("%s has no associated update file for song", PathName(this));
        }
    }
    static Symbol DLC_UPDATE_FONTS("DLC_UPDATE_FONTS");
    DataArray *updateArray = DataGetMacro(DLC_UPDATE_FONTS);
    for (int i = 0; i < updateArray->Size(); i++) {
        char buffer[256];
        String curStr(updateArray->Str(i));
        strcpy(buffer, MakeString("%s_%s", curStr, SystemLanguage()));
        AddClassExt(buffer, RndFont::StaticClassName());
        RndFont *updateFont = Find<RndFont>(buffer, false);
        if (updateFont) {
            FilePath path;
            if (ResourceDirBase::MakeResourcePath(
                    path, "HamLabel", "UILabelDir", curStr.c_str()
                )) {
                ObjDirPtr<UILabelDir> labelDirPtr;
                labelDirPtr.LoadFile(path, false, true, kLoadFront, false);
                if (labelDirPtr.IsLoaded()) {
                    RndFontBase *font = labelDirPtr->FontObj(gNullStr);
                    MILO_ASSERT(font, 0xA52);
                    ReplaceObject(font, updateFont, false, false, true);
                    mUpdateFonts.push_back(labelDirPtr);
                }
            }
        }
    }
    if (d.rev < 3 && !IsProxy()) {
        MILO_NOTIFY(
            "%s MoveDir older than version 3, need to resave this file", PathName(this)
        );
    }
    if (TheLoadMgr.EditMode()) {
        if (mFiltersEnabled) {
            MiloInit();
        }
    } else {
        mRecordClip = nullptr;
    }
}

void MoveDir::DrawShowing() {
    if (IsProxy()) {
        if (mDebugCollision) {
            SongCollision *col = Find<SongCollision>("SongCollision", false);
            if (col) {
                int beat = TheTaskMgr.Beat();
                if (unkf84 != beat) {
                    unkf84 = beat;
                    MILO_ASSERT(TheHamDirector, 0xAB1);
                    for (int i = 0; i < 2; i++) {
                        HamCharacter *hChar = TheHamDirector->GetCharacter(i);
                        if (hChar) {
                            unkf04[i] = hChar->WorldXfm();
                        }
                    }
                }
                Difficulty diffs[2];
                for (int i = 0; i < 2; i++) {
                    diffs[i] = TheGameData->Player(i)->GetDifficulty();
                }
                std::vector<SongCollisionOutput> songCollisionOutputs;
                col->IsCollision(beat, beat + 1, diffs, unkf04, &songCollisionOutputs);
                for (int i = 0; i < songCollisionOutputs.size(); i++) {
                    SongCollisionOutput &curOutput = songCollisionOutputs[i];
                    Hmx::Color color(0.8f, 0.8f, 0.8f);
                    Hmx::Color color20;
                    if (curOutput.unke0) {
                        color20.Set(1, 0, 0);
                    } else {
                        color20.Set(0, 1, 0);
                    }
                    for (int j = 0; j < 2; j++) {
                        const Vector3 &v = curOutput.unk60[j].v;
                        UtilDrawSphere(v, 1, color, nullptr);
                        UtilDrawString(MakeString("%i:%i", j, beat + i), v, color);
                        TheRnd.DrawLine(v, curOutput.unk0[j], color, false);
                        UtilDrawSphere(curOutput.unk0[j], 1, color, nullptr);
                        TheRnd.DrawLine(v, curOutput.unk20[j], color, false);
                        UtilDrawSphere(curOutput.unk20[j], 1, color, nullptr);
                    }
                    for (int j = 0; j < 2; j++) {
                        const Vector3 &v = curOutput.unk60[j].v;
                        Vector3 vsub;
                        Add(curOutput.unk40[j], v, vsub);
                        TheRnd.DrawLine(v, vsub, color20, false);
                        UtilDrawSphere(vsub, 2, color20, nullptr);
                        UtilDrawString(MakeString("%i", j), vsub, color20);
                    }
                }
            }
        }
    } else if (TheLoadMgr.EditMode()) {
        if (mDancerSeq) {
            ObjDirItr<SkeletonViz> it(this, true);
            if (it) {
                StubCameraInput input;
                input.PollTracking();
                const DancerSkeleton *skel = mDancerSeq->CurSkeleton();
                if (skel) {
                    it->Visualize(input, *skel, nullptr, false);
                }
            }
        } else {
            SkeletonDir::DrawShowing();
        }
    }
}

void MoveDir::Poll() {
    SkeletonDir::Poll();
    mDancerViz->Poll();
    if (TheHamDirector) {
        int curMeasure = TheTaskMgr.CurrentMeasure();
        for (int i = 0; i < 2; i++) {
            HamMove *oldMove = mCurMove[i];
            mCurMove[i] = nullptr;
            filler[i] = oldMove;
            MovePlayerData &curPlayerData = mMovePlayerData[i];
            auto &keys = curPlayerData.mMoveKeys;
            if (curMeasure >= 0 && curMeasure < keys.size()) {
                mCurMove[i] = keys[curMeasure].move;
            }
            MoveRating oldRating = mCurMoveRating[i];
            mCurMoveRating[i] = kMoveRatingOk;
            unk3f0[i] = oldRating;
            float oldRes = mCurMoveNormalizedResult[i];
            mCurMoveNormalizedResult[i] = 0;
            unk3e8[i] = oldRes;

            if (mCurMove[i]) {
                std::pair<DetectFrame *, DetectFrame *> frames;
                DetectRange(curPlayerData.mDetectFrames, frames, curMeasure, curMeasure);
                float frac = DetectFrac(i, mCurMove[i], frames);
                mCurMoveRating[i] =
                    DetectFracToMoveRating(frac, mCurMove[i]->RatingOverride());
                mCurMoveNormalizedResult[i] =
                    DetectFracToRatingFrac(frac, mCurMove[i]->RatingOverride());
            }
            mCurMoveSmoothers[i].Smooth(
                mCurMoveNormalizedResult[i],
                TheMaster && TheMaster->Unk70() == 3 ? TheTaskMgr.DeltaUISeconds() * 4.0f
                                                     : TheTaskMgr.DeltaUISeconds()
            );
            if (mCurMoveRating[i] <= kMoveRatingPerfect && unk3f0[i] > 1) {
                static Symbol passed_move_p1("passed_move_p1");
                static Symbol passed_move_p2("passed_move_p2");
                TheHamProvider->Export(
                    Message(i == 0 ? passed_move_p1 : passed_move_p2, mCurMove[i]->Name()),
                    true
                );
            }
        }
        if ((mCurMoveRating[0] <= 1 || mCurMoveRating[1] <= 1) && unk3f0[0] > 1
            && unk3f0[1] > 1) {
            static Message msg_passed_move("passed_move");
            TheHamProvider->Export(msg_passed_move, true);
        }
    }
}

void MoveDir::Enter() {
    PanelDir::Enter();
    int i13 = 0;
    if (TheHamDirector) {
        std::vector<HamMoveKey> hamMoveKeys;
        for (int i = 0; i < kNumDifficultiesDC2; i++) {
            TheHamDirector->MoveKeys((Difficulty)i, this, hamMoveKeys);
            int numKeys = hamMoveKeys.size();
            MaxEq(i13, numKeys);
            if (i == kDifficultyEasy) {
                while (--numKeys > 0) {
                    HamMoveKey &curKey = hamMoveKeys[numKeys];
                    if (curKey.move && curKey.move->IsFinalPose()) {
                        int tmp = curKey.beat / -4.0f;
                        mFinishingMoveMeasure = 1 - tmp;
                    }
                }
            }
        }
    }

    for (int i = 0; i < 2; i++) {
        MovePlayerData &cur = mMovePlayerData[i];
        cur.mCurMove = nullptr;
        cur.mMoveKeys.reserve(i13);
        cur.mDetectFrames.reserve(i13 * 16);
    }

    if (!TheLoadMgr.EditMode()) {
        mGamePanel = ObjectDir::Main()->Find<Hmx::Object>("game_panel", false);
        mErrorNodeInfo = 0;
        SetFiltersEnabled(true);
        unk310 = -1;
    } else {
        mGamePanel = nullptr;
    }

    if (TheHamDirector) {
        if (TheLoadMgr.EditMode()) {
            ResetDetection();
        }
        WorldDir *wDir = TheHamDirector->GetVenueWorld();
        if (wDir) {
            for (int i = 0; i < 2; i++) {
                MovePlayerData &cur = mMovePlayerData[i];
                ObjectDir *playerDir =
                    wDir->Find<ObjectDir>(MakeString("player%i", i), false);
                if (playerDir) {
                    cur.mFeedback =
                        playerDir->Find<CharFeedback>("char_feedback.cf", false);
                }
                if (cur.mFeedback) {
                    cur.mFeedback->ResetErrors();
                }
                cur.mPhraseMeter =
                    wDir->Find<HamPhraseMeter>(MakeString("phrase_meter%i", i), false);
                cur.mTextFeedback =
                    wDir->Find<RndDrawable>(MakeString("text_feedback%i", i), false);
            }
        }
        delete mFilterQueue;
        mFilterQueue = new FilterQueue();
        RELEASE(mAsyncDetector);
        mAsyncDetector = new MoveAsyncDetector(this);
        mDancerViz->Init();
        if (TheMaster) {
            static Symbol stream_jump("stream_jump");
            static Symbol beat("beat");
            TheMaster->AddSink(this, stream_jump);
            TheMaster->AddSink(this, beat);
        }
    }
}

void MoveDir::Exit() {
    PanelDir::Exit();
    if (TheMaster) {
        TheMaster->RemoveSink(this);
    }
}

void MoveDir::Update(const SkeletonUpdateData &data) {
    if (mFilterQueue) {
        mFilterQueue->Poll(data);
    }
}

void MoveDir::PostUpdate(const SkeletonUpdateData *data) {
    if (data) {
        if (mRecordClip) {
            mRecordClip->PollRecording(*data->unk8);
        }
        if (unk2bc) {
            unk2bc->PollRecording(*data->unk8);
        }
        if (unk2d0) {
            unk2d0->PollRecording(*data->unk8);
        }
        if (TheLoadMgr.EditMode()) {
            MILO_ASSERT(TheGameData, 0x387);
            TheGameData->AutoAssignSkeletons(data);
        }
        if (mMoveOverlay->Showing()) {
            if (mPlayClip && mDebugLatencyOffset) {
                SkeletonFrame skeletonFrame;
                if (mPlayClip->SkeletonFrameAt(
                        sLatencySeconds + SongSeconds(), skeletonFrame
                    )) {
                    unk424.Poll(0, skeletonFrame);
                }
            } else {
                const Skeleton *playerSkeleton = TheGameData->Player(0)->GetSkeleton(
                    reinterpret_cast<const Skeleton *const(&)[6]>(*data->unk4)
                );
                if (playerSkeleton) {
                    unk424 = *playerSkeleton;
                }
            }
        }
    }
    PostUpdateFilters();
    for (int i = 0; i < 2; i++) {
        HamMove *curMove = mMovePlayerData[i].mCurMove;
        if (!mFiltersEnabled || !curMove || curMove->IsRest()) {
            if (mMovePlayerData[i].mFeedback) {
                mMovePlayerData[i].mFeedback->ResetErrors();
            }
        }
    }
    FinalPoseStateMachine();
}

void MoveDir::Draw(const BaseSkeleton &baseSkeleton, SkeletonViz &skeletonViz) {
    if (unk414) {
        int disp_ms = unk414->ElapsedMs();
        if (disp_ms != -1) {
            for (int i = 0; i < kNumJoints; i++) {
                Vector3 vdisp;
                int actual_ms;
                unk414->Displacement(
                    nullptr, kCoordCamera, (SkeletonJoint)i, disp_ms, vdisp, actual_ms
                );
                MILO_ASSERT(disp_ms == actual_ms, 0x50F);
                Vector3 camJointPos = unk414->CamJointPos((SkeletonJoint)i);
                Vector3 vdiff;
                Subtract(camJointPos, vdisp, vdiff);
                Hmx::Color color(0.3f, 0.6f, 0.3f);
                mDancerViz->DrawLine3D(vdiff, camJointPos, 0.01f, color, nullptr);
            }
        }
    } else if (mFiltersEnabled && unk41c) {
        MILO_ASSERT(TheGestureMgr, 0x51A);
        MILO_ASSERT(TheHamDirector, 0x51B);
        MoveMode moveMode = CurrentMoveMode();
        const Skeleton *player_skel = dynamic_cast<const Skeleton *>(&baseSkeleton);
        MILO_ASSERT(player_skel, 0x51F);
        SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
        float songSpeed = SongSpeed();
        ErrorFrameInput input(
            handle.History(), unk41c->GetDancerFrame()->mSkeleton, *player_skel, songSpeed
        );
        ErrorNode **errorNodes = mFilterVer->mErrorNodes;
        for (int i = 0; i < mFilterVer->NumNodes(); i++) {
            ErrorNode *node = errorNodes[i];
            if (node->IsTypeJointMatch(mErrorNodeInfo)) {
                ErrorNodeInput nodeInput;
                mFilterVer->NodeInput(i, unk41c, moveMode, nodeInput);
                node->VizError(skeletonViz, input, nodeInput);
            }
        }
    }
}

float MoveDir::UpdateOverlay(RndOverlay *overlay, float f2) {
    if (mFiltersEnabled) {
        HamMove *curMove = mMovePlayerData[0].mCurMove;
        if (curMove) {
            const FilterVersion *fv = curMove->FilterVer();
            SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
            int numNodes = fv->NumNodes();
            MILO_ASSERT(TheGestureMgr, 0x795);
            MILO_ASSERT(mDancerViz, 0x796);
            MILO_ASSERT(TheHamDirector, 0x797);
            MoveMode moveMode = CurrentMoveMode();
            MoveMirrored mirrored =
                curMove->Mirrored() != false ? kMirroredYes : kMirroredNo;
            if (sFloat == 0) {
                Vector2 v =
                    TheRnd.DrawStringScreen("W", Vector2(sRect.x, f2), sLightGray, false);
                sFloat = (v.y - f2) * 0.8f;
            }
            if (mPlayClip) {
                f2 = DrawPlayClip(f2, mPlayClip, TheTaskMgr.CurrentMeasure());
            }

            TheRnd.DrawStringScreen(
                MakeString("%s", fv->mVersionSym),
                Vector2(sRect.x - 0.05f, f2),
                sLightGray,
                true
            );

            for (int i = 0; i < 4; i++) {
                Symbol ratingState;
                float threshold;
                RatingStateThreshold(i, ratingState, threshold, curMove->RatingOverride());
                Vector2 v88, v98;
                v88.y = f2;
                v98.y = sFloat + f2;
                float f33 = (sRect.y - sRect.x) / (sRect.h + sRect.w + 4) + sRect.x;
                v98.x = (0.99f - f33) * (threshold + f33);
                v88.x = v98.x;
                UtilDrawLine(v88, v98, sGray);
                const char *str = strstr(ratingState.Str(), "_");
                if (str) {
                    TheRnd.DrawStringScreen(str + 1, Vector2(v98.x, f2), sLightGray, true);
                }
            }

            float f44 = sFloat + f2;
            float f49 = (sRect.y - sRect.x) / (sRect.h + sRect.w + 4) + sRect.x;
            float f33 = 0.99f - f49;
            float f34 = curMove->IsRest() ? 0 : DetectFrac(0, -1);
            const char *mirrorText = mirrored == kMirroredYes ? "(mirror)" : gNullStr;
            f44 = DrawDetectedBar(
                f44,
                MakeString(
                    "%i %s %s", TheTaskMgr.CurrentMeasure(), curMove->Name(), mirrorText
                ),
                f34,
                f49,
                0.99f,
                false,
                true
            );
            DrawOverlayBar(f44, f49, 0.99f, sDarkGray, sFloat);
            DrawOverlayBar(f44, f49, mLastPollMs * 0.0625f * f33 + f49, sGreen, sFloat);
            TheRnd.DrawStringScreen(
                MakeString("timer: %.3fms\n", mLastPollMs),
                Vector2(f49, f44),
                sLightGray,
                true
            );

            f34 = sFloat;
            if (fv->mType == kFilterVersionHam1) {
                f34 *= (float)numNodes + 1;
            } else if (fv->mType == kFilterVersionHam2) {
                f34 *= 2;
            }
            Hmx::Rect r4a20(
                0,
                sFloat * 2 + f44,
                (sRect.y - sRect.x) * (0 / (sRect.h + sRect.w + 4)) + sRect.x,
                f34
            );
            f33 = r4a20.y;
            TheRnd.DrawRectScreen(
                r4a20, Hmx::Color(0.3f, 0.3f, 0.3f, 0.9f), nullptr, nullptr, nullptr
            );
            Hmx::Rect r49c0;
            r49c0.x = r4a20.w;
            r49c0.y = r4a20.y;
            r49c0.h = r4a20.h;
            float f50 = sRect.h + sRect.w + 4;
            r49c0.w = (sRect.y - sRect.x) * (f50 / f50) + sRect.x - f44;
            TheRnd.DrawRectScreen(r49c0, sDarkGray, nullptr, nullptr, nullptr);

            if (fv->mType == kFilterVersionHam1) {
                Vector2 v2(sRect.x, sFloat + f33);
                for (int n = 0; n < numNodes; n++) {
                    ErrorNode *errorNode = fv->mErrorNodes[n];
                    Vector2 v1 = TheRnd.DrawStringScreen(
                        errorNode->Name().Str(), v2, sLightGray, false
                    );
                    TheRnd.DrawStringScreen(
                        errorNode->Name().Str(),
                        Vector2(v2.x - (v1.x - v2.x), v2.y),
                        sLightGray,
                        true
                    );
                    v2.y += sFloat;
                };
            }

            for (int i = 0; i < 5; i++) {
                float fI = i;
                float x = (sRect.y - sRect.x) * (fI + sRect.w) / (sRect.h + sRect.w + 4)
                    + sRect.x;
                TheRnd.DrawStringScreen(MakeString("%i", i), Vector2(x, f33), sGray, true);
                DrawBeatLine(f33, f34, fI, sGray);
            }

            float beat = TheTaskMgr.TotalBeat();
            int measure = TheTaskMgr.CurrentMeasure() * 4;
            float fMeasure = measure;
            float f38 = beat - fMeasure;
            float yRatio = TheRnd.YRatio() * sFloat * 0.5f;
            MoveFrame *closestMoveFrame = ClosestMoveFrame();
            auto &moveFrames = curMove->GetMoveFrames();
            for (int i = 0; i < moveFrames.size(); i++) {
                MoveFrame &cur = moveFrames[i];
                const Ham2FrameWeight &wt = cur.FrameWeight(mirrored);
                if (wt.unk0 != 0) {
                    float x = (sRect.y - sRect.x) * (wt.unk0 + sRect.w)
                            / (sRect.h + sRect.w + 4)
                        + sRect.x;
                    bool isClosest = &cur == closestMoveFrame;
                    Hmx::Color c49b0 = isClosest ? Hmx::Color(0.8f, 0.8f, 0.8f)
                                                 : Hmx::Color(0.8f, 0.8f, 0);
                    DrawBeatLine(f33, f34, wt.unk0, c49b0);
                    TheRnd.DrawStringScreen(
                        MakeString("%.2f", wt.unk0), Vector2(x, f33 - sFloat), c49b0, true
                    );
                    if (fv->mType == kFilterVersionHam1) {
                        Vector2 v4b00(x - yRatio, sFloat + f33);
                        for (int n = 0; n < numNodes; n++) {
                            Vector2 v4ad0(x, v4b00.y + sFloat);
                            UtilDrawRect2D(v4b00, v4ad0, c49b0);
                            v4b00.y += sFloat;
                        }
                    }
                }
            }
            std::pair<DetectFrame *, DetectFrame *> range;
            DetectRange(
                mMovePlayerData[0].mDetectFrames,
                range,
                TheTaskMgr.CurrentMeasure(),
                TheTaskMgr.CurrentMeasure()
            );
            if (fv->mType == kFilterVersionHam1) {
                for (DetectFrame *it = range.first; it != range.second; ++it) {
                    const MoveFrame *mf = it->GetMoveFrame();
                    float f35 = sFloat + f33;
                    float f36 = (sRect.y - sRect.x) * (mf->Beat() + sRect.w)
                            / (sRect.h + sRect.w + 4)
                        + sRect.x;
                    float f45 = f36 - f44;
                    for (int n = 0; n < numNodes; n++) {
                        Hmx::Rect r49e0(f45, f35, f36, f35 + sFloat - f35);
                        const Ham1NodeWeight &wt =
                            mf->NodeWeightHam1(n, moveMode, mirrored);
                        if (wt.unk0) {
                            float green = 1 - it->BestNodeError(n).x;
                            Hmx::Color c4a30(green * -1 + 1, green, green * 0, 1);
                            TheRnd.DrawRectScreen(r49e0, c4a30, nullptr, nullptr, nullptr);
                        }
                    }
                    f35 += sFloat;
                }
            }
            static Symbol merge_moves("merge_moves");
            int mergeMovesProp = TheHamProvider->Property(merge_moves)->Int();
            const DancerSkeleton *skelPtr = nullptr;
            unk41c = nullptr;
            if (mergeMovesProp) {
                if (curMove->GetDancerSequence()) {
                    float f41 = f38 / 4;
                    auto &dancerFrames = curMove->GetDancerSequence()->GetDancerFrames();
                    int skelIdx = Round((float)dancerFrames.size() * f41);
                    skelPtr = &dancerFrames[skelIdx].mSkeleton;
                }
            } else {
                float secs = BeatToSeconds(fMeasure - sRect.w);
                float nextSecs = BeatToSeconds((float)(measure + 4) + sRect.h);
                auto it = std::lower_bound(
                    mMovePlayerData[0].mDetectFrames.begin(),
                    mMovePlayerData[0].mDetectFrames.end(),
                    secs,
                    DetectFrameSecondsCmp()
                );
                float f48 = f34 + f33;
                for (; it != mMovePlayerData[0].mDetectFrames.end(); ++it) {
                    // stuff and things
                }
            }
            Hmx::Color yellow(1, 1, 0);
            DrawBeatLine(f33, f34, f38, sLightGray);
            TheRnd.DrawStringScreen(
                MakeString("%.2f", f38),
                Vector2(
                    (sRect.y - sRect.x) * (f33 + sRect.w) / (sRect.h + sRect.w + 4)
                        + sRect.x,
                    f33
                ),
                yellow,
                true
            );
            float latencyBeat = SecondsToBeat(BeatToSeconds(beat) - sLatencySeconds);
            DrawBeatLine(f33, f34, latencyBeat - fMeasure, Hmx::Color(0, 0.5f, 0));

            f34 = f34 + sFloat + f33;
            if (fv->mType == kFilterVersionHam2) {
                f34 = f34 + sFloat;
            }
            float f39 = Max(0.2f, 1.0f - f34);
            Hmx::Rect r4b50(sRect.x, f34, TheRnd.YRatio() * f39, f39);
            TheRnd.DrawRectScreen(r4b50, sDarkGray, nullptr, nullptr, nullptr);

            Vector2 mfVec(sRect.x, f34);

            if (closestMoveFrame) {
                TheRnd.DrawStringScreen(
                    MakeString("%.2f", closestMoveFrame->Beat()), mfVec, sLightGray, true
                );
                mDancerViz->SetUsePhysicalCam(true);
                mDancerViz->SetPhysicalCamScreenRect(r4b50);
                if (skelPtr) {
                    StubCameraInput input;
                    input.PollTracking();
                    std::vector<SkeletonCallback *> callbacks;
                    callbacks.push_back(this);
                    unk414 = skelPtr;
                    mDancerViz->Visualize(input, *skelPtr, &callbacks, false);
                    unk414 = nullptr;
                }
            } else {
                int numDFs = unkf88.size();
                TheRnd.DrawStringScreen(
                    MakeString("asyc: %d", numDFs), mfVec, sLightGray, true
                );
                if (numDFs != 0) {
                    mDancerViz->SetUsePhysicalCam(true);
                    int ceiled = ceilf(sqrtf((float)numDFs));
                    // more stuff and more things
                    FOREACH (it, unkf88) {
                    }
                }
            }

            std::vector<SkeletonCallback *> callbacks;
            callbacks.push_back(this);
            Hmx::Rect r2 = r4b50;
            r2.x += r4b50.w + 0.01f;
            TheRnd.DrawRectScreen(r2, sDarkGray, nullptr, nullptr, nullptr);
            mDancerViz->SetUsePhysicalCam(true);
            mDancerViz->SetPhysicalCamScreenRect(r2);
            mDancerViz->Visualize(*handle.GetCameraInput(), *unk414, &callbacks, false);
            const Vector2 &vret = TheRnd.DrawStringScreen(
                MakeString("latency offset: %s", mDebugLatencyOffset ? "ON" : "OFF"),
                Vector2(r2.x, r2.y),
                sLightGray,
                true
            );
            TheRnd.DrawStringScreen(
                MakeString("rotation: %.2f", mDancerViz->PhysicalCamRotation()),
                Vector2(r2.x, vret.y),
                sLightGray,
                true
            );
            return f34 + f39;
        }
    }
    return f2;
}

DataNode OnDetectFracToRating(DataArray *a) {
    HamMove *move = a->Size() > 2 ? a->Obj<HamMove>(2) : nullptr;
    const std::vector<float> *ratings = nullptr;
    if (move) {
        ratings = move->RatingOverride();
    }
    return DetectFracToRating(a->Float(1), ratings, nullptr);
}

DataNode OnDetectFracToRatingFrac(DataArray *a) {
    HamMove *move = a->Obj<HamMove>(2);
    return DetectFracToRatingFrac(a->Float(1), move->RatingOverride());
}

DataNode OnRatingStateToIndex(DataArray *a) { return RatingStateToIndex(a->Sym(1)); }

DataNode OnGetScoreBonus(DataArray *a) {
    HamMove *move = a->Size() > 2 ? a->Obj<HamMove>(2) : nullptr;
    const std::vector<float> *ratings = nullptr;
    if (move) {
        ratings = move->RatingOverride();
    }
    return GetScoreBonus(a->Float(1), ratings);
}

void MoveDir::Init() {
    REGISTER_OBJ_FACTORY(MoveDir);
    DataArray *cfg = SystemConfig()->FindArray("scoring", false);
    if (cfg) {
        LoadScoring(cfg);
    }
    DataRegisterFunc("detect_frac_to_rating", OnDetectFracToRating);
    DataRegisterFunc("detect_frac_to_rating_frac", OnDetectFracToRatingFrac);
    DataRegisterFunc("rating_state_to_index", OnRatingStateToIndex);
    DataRegisterFunc("get_score_bonus", OnGetScoreBonus);
}

void MoveDir::ClearLimbFeedback(int player) {
    MILO_LOG("MoveDir::ClearLimbFeedback(int player = %d)\n", player);
    CharFeedback *feedback = mMovePlayerData[player].mFeedback;
    HamPlayerData *hpd = TheGameData->Player(player);
    if (feedback && hpd) {
        feedback->ResetErrors();
        for (int i = 0; i < 4; i++) {
            feedback->UpdateLimb(i, false);
        }
    }
}

void MoveDir::SetFiltersEnabled(bool enabled) {
    mFiltersEnabled = enabled;
    if (mFiltersEnabled && TheLoadMgr.EditMode()) {
        MiloInit();
    }
}

void MoveDir::SetFilterVersion(Symbol version) {
    for (int i = 0; i < sFilterVersions.size(); i++) {
        if (sFilterVersions[i]->mVersionSym == version) {
            mFilterVer = sFilterVersions[i];
            return;
        }
    }
    MILO_FAIL("Could not find filter version %s", version);
}

const FilterVersion *MoveDir::FindFilterVersion(FilterVersionType t) {
    for (std::vector<FilterVersion *>::iterator it = sFilterVersions.begin();
         it != sFilterVersions.end();
         ++it) {
        if ((*it)->mType == t)
            return *it;
    }
    return nullptr;
}

HamMove *MoveDir::CurrentMove(int player) const {
    MILO_ASSERT((0) <= (player) && (player) < (2), 0x164);
    return mMovePlayerData[player].mCurMove;
}

int MoveDir::MoveIdx() const { return TheTaskMgr.CurrentMeasure(); }
int MoveDir::MoveBeat() const { return TheTaskMgr.CurrentBeat(); }

void MoveDir::SetMoveOverlay(bool overlay) {
    if (!mFiltersEnabled && TheLoadMgr.EditMode()) {
        SetFiltersEnabled(true);
    }
    mShowMoveOverlay = overlay;
    mMoveOverlay->SetShowing(overlay);
}

SkeletonClip *MoveDir::ImportClip(bool b1) {
    if (mImportClipPath.empty()) {
        MILO_NOTIFY("Set import_clip_path first");
        return nullptr;
    } else {
        const char *filename = FileGetName(mImportClipPath.c_str());
        SkeletonClip *clip = Find<SkeletonClip>(filename, false);
        if (clip) {
            MILO_LOG("%s already exists, not importing\n", filename);
        } else {
            clip = Hmx::Object::New<SkeletonClip>();
            clip->SetName(filename, this);
            clip->SetPath(mImportClipPath.c_str());
        }
        return clip;
    }
}

void MoveDir::StopSongRecord() {
    if (mRecordClip && mRecordClip->IsRecording()) {
        mRecordClip->StopRecording();
        if (unk2bc)
            unk2bc->StopRecording();
    } else {
        MILO_NOTIFY("Start recording first");
    }
}

void MoveDir::FlushMoveRecord() {
    SkeletonClip *clip = unk2d0;
    if (clip) {
        String clipName = RecordClipName("ktb", -1);
        clip->FlushMoveRecord(clipName.c_str());
    } else {
        MILO_NOTIFY("skeleton recording not yet active");
    }
}

void MoveDir::SwapMoveRecord() {
    SkeletonClip *clip = unk2d0;
    if (clip) {
        clip->SwapMoveRecord();
    } else {
        MILO_NOTIFY("skeleton recording not yet active");
    }
}

HamMove *MoveDir::GetMoveAtMeasure(int player, int i2) {
    static Symbol move("move");
    HamPlayerData *hpd = TheGameData->Player(player);
    Keys<Symbol, Symbol> *keys =
        TheHamDirector->GetPropKeys(hpd->GetDifficulty(), move)->AsSymbolKeys();
    return Find<HamMove>((*keys)[i2].value.Str(), false);
    return nullptr;
}

DancerSequence *MoveDir::PerformanceSequence(Difficulty diff) {
    MILO_ASSERT((0) <= (diff) && (diff) < (kNumDifficulties), 0x207);
    Symbol diffSym = DifficultyToSym(diff);
    const char *seqName = MakeString("performance_%s.seq", diffSym);
    return Find<DancerSequence>(seqName, false);
}

void MoveDir::FinishGameRecord() {
    MILO_ASSERT(!TheLoadMgr.EditMode(), 0x604);
    if (mRecordClip) {
        MILO_LOG("Finishing song recording: %s\n", mRecordClip->Path());
        mRecordClip->StopRecording();
        RELEASE(mRecordClip);
    }
    if (unk2bc) {
        MILO_LOG("Finishing song recording: %s\n", unk2bc->Path());
        unk2bc->StopRecording();
        RELEASE(unk2bc);
    }
    RELEASE(unk2d0);
}

void MoveDir::SetupSongRecordClip() {
    static Symbol rhythm_battle("rhythm_battle");
    bool b1 = mGamePanel && mGamePanel->Type() == rhythm_battle;
    bool b7 = false;
    if (mGamePanel) {
        static Message msg("is_game_over");
        b7 = mGamePanel->Handle(msg, true).Int();
    }
    if (!b7) {
        const char *modeStr;
        if (b1) {
            modeStr = "ktb";
        } else if (TheHamDirector->InPracticeMode()) {
            modeStr = "bid";
        } else
            modeStr = "pi";
        if (sGameRecord && !mRecordClip) {
            unsigned int x = sGameRecord2Player;
            if (x) {
                SetupRecordClip(mRecordClip, 0, 0, modeStr, this);
                SetupRecordClip(unk2bc, 1, 1, modeStr, this);
            } else {
                SetupRecordClip(mRecordClip, x, -1, modeStr, this);
            }
        }
        if (b1 && !unk2d0) {
            SetupRecordClip(unk2d0, 2, 0, modeStr, this);
        }
    }
}

void MoveDir::SetDancerSequence(DancerSequence *seq) { mDancerSeq = seq; }

void MoveDir::LoadScoring(const DataArray *cfg) {
    static Symbol min_frame_dist_beats("min_frame_dist_beats");
    cfg->FindData(min_frame_dist_beats, HamMove::sMinFrameDistBeats);
    static Symbol latency_offset("latency_offset");
    cfg->FindData(latency_offset, sLatencySeconds);
    sLatencySeconds /= 1000;
    static Symbol plf_min_time_error("plf_min_time_error");
    sPLFMinTimeError = cfg->FindFloat(plf_min_time_error);
    ScoreUtlInit(cfg);
    DeleteAll(sFilterVersions);
    DataArray *versionsArr = cfg->FindArray("versions");
    for (int i = 1; i < versionsArr->Size(); i++) {
        sFilterVersions.push_back(FilterVersion::Create(versionsArr->Array(i)));
    }
    MILO_ASSERT(!sFilterVersions.empty(), 0x2E2);
}

void MoveDir::ReloadScoring() {
    MILO_ASSERT(TheLoadMgr.EditMode(), 0x1268);
    DataArray *cfg = SystemConfig("scoring");
    DataArray *file = DataReadFile(cfg->Array(1)->File(), true);
    LoadScoring(file);
    ScoreUtlInit(file);
    Enter();
    file->Release();
}

void MoveDir::ResetDetection() {
    if (TheHamDirector) {
        if (SkeletonUpdate::HasInstance()) {
            SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
            if (!handle.HasCallback(this)) {
                handle.AddCallback(this);
            }
        }
        MILO_ASSERT(TheGameData, 0x642);
        for (int i = 0; i < 2; i++) {
            HamPlayerData *player_data = TheGameData->Player(i);
            MILO_ASSERT(player_data, 0x646);
            if (player_data->IsPlaying()) {
                ResetDetectFrames(i, player_data->GetDifficulty());
            }
        }
        SetupSongRecordClip();
    }
}

void MoveDir::SetSongPlayClip(SkeletonClip *clip) {
    if (!mFiltersEnabled && clip) {
        SetFiltersEnabled(true);
    }
    if (mRecordClip && mRecordClip->IsRecording()) {
        MILO_NOTIFY("Can't set play clip while recording");
    } else {
        mPlayClip = clip;
        SetSkeletonClip(clip);
        ResetDetection();
        TheGameData->UnassignSkeletons();
    }
}

void MoveDir::MiloUpdate() {
    SkeletonDir::MiloUpdate();
    MILO_ASSERT(TheGestureMgr, 0xB14);
    SetCurrentMove(0, mMovePlayerData[0].mCurMove);
    SetMoveOverlay(mShowMoveOverlay);
    SetSongPlayClip(mPlayClip);
}

DataNode MoveDir::OnStreamJump(const DataArray *) {
    if (mDebugLoop) {
        ResetDetection();
        unk310 = -1;
    }
    return 0;
}

void MoveDir::OnBeat() {
    if (TheMaster && (int)TheMaster->TotalBeat2() % 4 == 3
        && (int)TheMaster->TotalBeat1() % 4 == 0) {
        for (int i = 0; i < 2; i++) {
            mCurMoveSmoothers[i].Reset();
        }
    }
}

void MoveDir::SetDebugLoop(bool loop) { mDebugLoop = loop; }

PracticeSection *MoveDir::GetPracticeSection(Difficulty d) {
    for (ObjDirItr<PracticeSection> it(this, true); it != nullptr; ++it) {
        if (it->GetDifficulty() == d) {
            return it;
        }
    }
    return nullptr;
}

DancerSequence *MoveDir::SkillsSequence(Difficulty d, Symbol s1, Symbol s2) {
    PracticeSection *section = GetPracticeSection(d);
    if (section) {
        return section->SequenceForDetection(s1, s2);
    } else {
        return nullptr;
    }
}

void MoveDir::SetCurrentMove(int player, HamMove *move) {
    MILO_ASSERT_RANGE(player, 0, 2, 0x563);
    MovePlayerData &mpd = mMovePlayerData[player];
    HamPhraseMeter *hpm = mpd.mPhraseMeter;
    if (hpm) {
        hpm->SetRatingFrac(0, -1);
        hpm->SetShowing(
            move && move->Scored() && TheGameData->Player(player)->IsPlaying()
            && !InGracePeriod(player)
        );
    }
    if (mpd.mTextFeedback) {
        mpd.mTextFeedback->SetShowing(mpd.mState == 0);
    }
    mpd.mCurMove = move;
    if (move) {
        float f8 = TheTaskMgr.TotalBeat() - TheTaskMgr.CurrentMeasure() * 4;
        float f9 = BeatToSeconds(f8);
        f9 = (BeatToSeconds(f8 + 4.0f) - f9) * 1000.0f;
        if (TheMaster && TheMaster->GetAudio()
            && TheMaster->GetAudio()->GetSongStream()) {
            f9 = f9 / TheMaster->GetAudio()->GetSongStream()->GetSpeed();
        }
        if (move->SuppressGuideGesture()) {
            XNuiDelayUI((int)f9);
        }
        if (move->SuppressPracticeOptions()) {
            static Message suppressMsg("begin_suppress_practice_options", 0);
            suppressMsg[0] = f9 / 1000.0f;
            TheHamProvider->Handle(suppressMsg, false);
        }
    }
    mMoveOverlay->SetCallback(this);
}

float MoveDir::SongSeconds() {
    float secs = TheTaskMgr.Seconds(TaskMgr::kRealTime);
    if (TheMaster && TheMaster->GetAudio() && TheMaster->GetAudio()->GetSongStream()) {
        float time =
            TheMaster->GetAudio()->GetSongStream()->GetJumpBackTotalTime(secs * 1000.f);
        secs += time / 1000.0f;
    }
    return secs;
}

float MoveDir::SongSpeed() const {
    if (TheMaster) {
        return TheMaster->GetAudio()->GetSongStream()->GetSpeed();
    } else {
        return 1;
    }
}

float MoveDir::DetectRangePSNR(
    const std::pair<const DetectFrame *, const DetectFrame *> &detectFrames,
    const FilterVersion *fv
) const {
    MILO_ASSERT(fv->mType == kFilterVersionHam2, 0x1E8);
    float ret = 0;
    MoveMode moveMode = CurrentMoveMode();
    for (const DetectFrame *it = detectFrames.first; it != detectFrames.second; ++it) {
        const Ham2FrameWeight &wt = it->GetMoveFrame()->FrameWeight(it->Mirror());
        float cmp = wt.unk0;
        if (cmp > 0 && it->HasScore()) {
            ret += it->Score(fv, moveMode) * cmp;
        }
    }
    return ret;
}

float MoveDir::DetectRangeFrac(
    const std::pair<DetectFrame *, DetectFrame *> &detectFrames, const FilterVersion *fv
) const {
    MILO_ASSERT(fv->mType == kFilterVersionHam1, 0x1D5);
    int idx = 0;
    float ret = 0;
    MoveMode moveMode = CurrentMoveMode();
    for (DetectFrame *it = detectFrames.first; it != detectFrames.second; ++it, ++idx) {
        ret += it->Score(fv, moveMode);
    }
    if (idx > 0) {
        return Clamp(0.0f, 1.0f, ret / (float)idx);
    } else {
        return 0;
    }
}

bool MoveDir::InGracePeriod(int player) {
    Hmx::Object *provider = TheGameData->Player(player)->Provider();
    if (!provider) {
        return false;
    }
    static Symbol start_score_move_index("start_score_move_index");
    const DataNode *prop = provider->Property(start_score_move_index, false);
    if (!prop) {
        return false;
    } else {
        return TheTaskMgr.CurrentMeasure() < prop->Int();
    }
}

MoveFrame *MoveDir::ClosestMoveFrame() {
    struct FilterFrameDist {
        FilterFrameDist(float dist) : mDist(dist) {}
        bool operator()(const MoveFrame &frame1, const MoveFrame &frame2) const {
            return fabsf(frame1.Beat() - mDist) < fabsf(frame2.Beat() - mDist);
        }

        float mDist; // 0x0
    };
    HamMove *move = mMovePlayerData[0].mCurMove;
    if (!move)
        return nullptr;

    int measure = TheTaskMgr.CurrentMeasure();
    float beat = TheTaskMgr.TotalBeat();
    int measureBeats = measure * 4;
    std::vector<MoveFrame> &frames = move->GetMoveFrames();
    MoveFrame *ret = std::min_element(
        frames.begin(), frames.end(), FilterFrameDist(beat - (float)measureBeats)
    );
    return ret != frames.end() ? ret : nullptr;
}

float MoveDir::DetectFrac(
    int player,
    const HamMove *move,
    const std::pair<DetectFrame *, DetectFrame *> &detectFrames
) {
    MILO_ASSERT_RANGE(player, 0, 2, 0x187);
    MILO_ASSERT(TheGameData, 0x188);
    MILO_ASSERT(move, 0x189);
    const FilterVersion *fv = move->FilterVer();
    float frac;
    if (fv->mType == kFilterVersionHam2) {
        frac = move->PSNRToDetectFrac(DetectRangePSNR(detectFrames, fv));
    } else {
        frac = DetectRangeFrac(detectFrames, fv);
    }
    Symbol autoplay = TheGameData->Player(player)->Autoplay();
    if (!autoplay.Null()) {
        static Symbol maximum("maximum");
        if (autoplay == maximum) {
            frac = 1;
        } else {
            frac = RatingToDetectFrac(autoplay, move->RatingOverride());
        }
        if (detectFrames.first != detectFrames.second) {
            int i8 = 0;
            int i7 = 0;
            for (DetectFrame *it = detectFrames.first; it != detectFrames.second; ++it) {
                const Ham2FrameWeight &wt = it->GetMoveFrame()->FrameWeight(it->Mirror());
                if (wt.unk0 != 0) {
                    i8++;
                    if (it->HasScore()) {
                        i7++;
                    }
                }
            }
            if (i8 != 0) {
                frac *= (float)i7 / (float)i8;
            }
        }
    }
    return frac;
}

void MoveDir::DetectRange(
    std::vector<DetectFrame> &frames,
    std::pair<DetectFrame *, DetectFrame *> &range,
    int low,
    int high
) {
    range.first =
        std::lower_bound(frames.begin(), frames.end(), low, DetectFrameMoveIdxCmp());
    range.second =
        std::upper_bound(frames.begin(), frames.end(), high, DetectFrameMoveIdxCmp());
}

float MoveDir::DetectFrac(int player, int i2) {
    MILO_ASSERT_RANGE(player, 0, 2, 0x16A);
    int u2 = TheTaskMgr.CurrentMeasure();
    if (i2 == -1) {
        i2 = u2;
    }

    MovePlayerData &mpd = mMovePlayerData[player];
    auto &keys = mpd.mMoveKeys;
    HamMove *move = nullptr;
    if (i2 >= 0 && i2 < keys.size()) {
        move = keys[i2].move;
    }

    if (!move) {
        return 0;
    } else {
        std::pair<DetectFrame *, DetectFrame *> range;
        DetectRange(mpd.mDetectFrames, range, i2, i2);
        if (range.first == range.second) {
            return mAsyncDetector->MoveRatingFrac(
                player, (MoveAsyncDetector::RatingBar)(u2 != i2), move
            );
        } else {
            return DetectFrac(player, move, range);
        }
    }
}

void MoveDir::EnqueueDetectFrames(
    float f1, int player, std::vector<DetectFrame> &frames, const FilterVersion *fv
) {
    MILO_ASSERT(mFilterQueue, 0x536);
    MILO_ASSERT_RANGE(player, 0, 2, 0x537);
    MILO_ASSERT(TheHamDirector, 0x538);

    std::pair<DetectFrame *, DetectFrame *> range;
    int measure = TheTaskMgr.CurrentMeasure();
    DetectRange(frames, range, measure - 1, measure + 1);
    CurrentMoveMode();
    DetectFrame *toInsert = nullptr;
    for (DetectFrame *it = range.first; it != range.second; ++it) {
        float f12 = ScaleDistToError(fv->mScaleOp, fabsf(it->Seconds() - f1));
        if (f12 < 1) {
            mFilterQueue->EnqueueFrame(player, f12, f1 - it->Seconds(), it, fv);
            if (f12 <= 1000) {
                toInsert = it;
            }
        }
    }
    if (toInsert) {
        unkf88.insert(toInsert);
    }
}

void MoveDir::ResetDetectFrames(int player, Difficulty diff) {
    MILO_ASSERT_RANGE(player, 0, 2, 0x678);
    MILO_ASSERT_RANGE(diff, 0, kNumDifficulties, 0x679);
    MILO_ASSERT(TheHamDirector, 0x67A);
    SetupSongRecordClip();
    if (mFilterQueue) {
        mFilterQueue->CancelJob();
    }
    auto &mpd = mMovePlayerData[player];
    unk310 = -1;
    mpd.mState = 0;
    mpd.mDetectFrames.clear();
    if (diff != kDifficultyBeginner) {
        DancerSequence *seq;
        if (TheHamDirector->InPracticeMode()) {
            Symbol start = TheHamDirector->PracticeStart();
            Symbol end = TheHamDirector->PracticeEnd();
            seq = SkillsSequence(diff, start, end);
        } else {
            seq = PerformanceSequence(diff);
        }
        if (!seq) {
            MILO_NOTIFY(
                "%s: could not find %s DancerSequence (%s)",
                PathName(this),
                DifficultyToSym(diff),
                TheHamDirector->InPracticeMode() ? "skills" : "perform"
            );
        } else {
            auto &dancerFrames = seq->GetDancerFrames();
            if (dancerFrames.empty()) {
                MILO_LOG(
                    "%s %s: could not reset detect frames, no DancerFrames\n",
                    PathName(this),
                    DifficultyToSym(diff)
                );
            } else {
                auto dancerFrameIt = dancerFrames.begin();
                auto &moveKeys = mpd.mMoveKeys;
                int moveKeysCap = moveKeys.capacity();
                TheHamDirector->MoveKeys(diff, this, moveKeys);
                if (moveKeys.size() > moveKeysCap) {
                    unsigned int moveKeysSize = moveKeys.size();
                    MILO_NOTIFY(
                        "%s move keys size (%i) above capacity (%i)",
                        PathName(this),
                        moveKeysSize,
                        moveKeysCap
                    );
                }
                int detectFramesCap = mpd.mDetectFrames.capacity();
                for (int i = 0; i < moveKeys.size(); i++) {
                    if (dancerFrameIt->mMoveIdx == i) {
                        auto &curMoveKey = moveKeys[i];
                        HamMove *move = curMoveKey.move;
                        auto &moveFrames = move->GetMoveFrames();
                        MoveMirrored mirrored =
                            move->Mirrored() ? kMirroredYes : kMirroredNo;
                        for (int j = 0; j < moveFrames.size(); j++) {
                            if (dancerFrameIt->mMoveFrameIdx == j) {
                                DetectFrame detectFrame;
                                auto &curMoveFrame = moveFrames[j];
                                float secs =
                                    curMoveFrame.QuantizedSeconds(curMoveKey.beat);
                                detectFrame.Reset(
                                    mFilterVer,
                                    secs,
                                    &curMoveFrame,
                                    dancerFrameIt,
                                    mirrored
                                );
                                mpd.mDetectFrames.push_back(detectFrame);
                                ++dancerFrameIt;
                                if (dancerFrameIt == dancerFrames.end()) {
                                    if (mpd.mDetectFrames.size() > detectFramesCap) {
                                        unsigned int detectFramesSize =
                                            mpd.mDetectFrames.size();
                                        MILO_NOTIFY(
                                            "%s detect frames size (%i) above capacity (%i)",
                                            PathName(this),
                                            detectFramesSize,
                                            detectFramesCap
                                        );
                                    }
                                    return;
                                }
                            } else {
                                MILO_LOG(
                                    "%s %s: invalid DancerFrame at move %i frame %i\n",
                                    PathName(this),
                                    DifficultyToSym(diff),
                                    i,
                                    dancerFrameIt->mMoveFrameIdx
                                );
                            }
                        }
                    }
                }
            }
        }
    }
}

void MoveDir::FinalPoseStateMachine() {
    int other_player = 1;
    float f13 = TheTaskMgr.TotalBeat() - (float)(TheTaskMgr.CurrentMeasure() * 4);
    for (int player = 0; other_player > -1; player++, other_player--) {
        HamMove *curMove = mMovePlayerData[player].mCurMove;
        if (TheGameData->Player(player)->IsPlaying() && !InGracePeriod(player) && curMove
            && curMove->IsFinalPose()) {
            const FilterVersion *fv = curMove->FilterVer();
            if (curMove->IsFinalPose() && mMovePlayerData[player].mState != 2) {
                float frac;
                if (TheMoveMgr->HasRoutine()) {
                    frac = mAsyncDetector->MoveRatingFrac(
                        player, (MoveAsyncDetector::RatingBar)0, curMove
                    );
                } else {
                    frac = DetectFrac(player, -1);
                }
                auto &moveFrames = curMove->GetMoveFrames();
                if (!moveFrames.empty()) {
                    float backBeat = moveFrames.back().Beat();
                    if (mMovePlayerData[player].mState == 0 && backBeat <= f13) {
                        MILO_ASSERT_RANGE(other_player, 0, 2, 0x4CE);
                        if (mMovePlayerData[other_player].mState == 0) {
                            static Message msg("final_pose_photo");
                            TheHamProvider->Export(msg, true);
                        }
                        mMovePlayerData[player].mState = 1;
                    }
                    if (mMovePlayerData[player].mState == 1) {
                        float measure = TheTaskMgr.CurrentMeasure() * 4;
                        float secs = BeatToSeconds(measure + backBeat);
                        float dist =
                            ScaleFullErrorDist(fv->mScaleOp) + sLatencySeconds + secs;
                        float beat = SecondsToBeat(dist);
                        if (4 <= beat - measure) {
                            MILO_NOTIFY_ONCE(
                                "%s last frame is too late, end pose won't be scored correctly",
                                curMove->Name()
                            );
                        }
                        if (dist <= unk30c || f13 >= 4 - HamMove::sMinFrameDistBeats) {
                            static Symbol final_pose_rating("final_pose_rating");
                            TheGameData->Player(player)->Provider()->SetProperty(
                                final_pose_rating,
                                DetectFracToRating(
                                    frac, curMove->RatingOverride(), nullptr
                                )
                            );
                            mMovePlayerData[player].mState = 2;
                        }
                    }
                }
            }
        }
    }
}

void MoveDir::PostUpdateFilters() {
    if (mFilterQueue) {
        if (!mFilterQueue->HasJob()) {
            unkf88.clear();
            bool b15 = false;
            for (int i = 0; i < 2; i++) {
                if (mMovePlayerData[i].mCurMove) {
                    b15 = true;
                    break;
                }
            }
            if (mFiltersEnabled && b15 && TheMaster && TheMaster->GetAudio()
                && TheMaster->GetAudio()->IsReady()) {
                MILO_ASSERT(TheGameData, 0x3D9);
                float f32 = SongSeconds() - sLatencySeconds;
                bool cmp = f32 > unk310;
                if (cmp || TheLoadMgr.EditMode()) {
                    if (cmp) {
                        unk310 = f32;
                    }
                    mFilterQueue->EnqueueNewJob(f32, SongSpeed(), CurrentMoveMode());
                    for (int i = 0; i < 2; i++) {
                        if (TheGameData->Player(i)->IsPlaying() && !InGracePeriod(i)) {
                            MovePlayerData &mpd = mMovePlayerData[i];
                            if (mpd.mCurMove && mpd.mCurMove->Scored()) {
                                EnqueueDetectFrames(
                                    f32, i, mpd.mDetectFrames, mpd.mCurMove->FilterVer()
                                );
                            }
                        }
                        if (mAsyncDetector) {
                            mAsyncDetector->EnqueueDetectFrames(
                                TheTaskMgr.CurrentMeasure(),
                                TheTaskMgr.CurrentBeat(),
                                f32,
                                i
                            );
                        }
                    }
                    mFilterQueue->StartJob();
                }
            }
        } else if (mFilterQueue->IsJobFinished()) {
            mLastPollMs = mFilterQueue->LastPollMs();
            MoveMode moveMode = CurrentMoveMode();
            float fracs[2];
            for (int i = 0; i < 2; i++) {
                if (TheMoveMgr->HasRoutine()) {
                    fracs[i] = mAsyncDetector->MoveRatingFrac(
                        i, (MoveAsyncDetector::RatingBar)0, mMovePlayerData[i].mCurMove
                    );
                } else {
                    fracs[i] = DetectFrac(i, -1);
                }
            }

            DetectFrame *frames[2];
            if (mFilterQueue->GetResults(unk30c, frames, sPLFMinTimeError)) {
                static Symbol flip_camshot_targets("flip_camshot_targets");
                const DataNode *prop = TheHamProvider->Property(flip_camshot_targets);
                bool b15 = prop ? prop->Int() != 0 : false;
                for (int i = 0; i < 2; i++) {
                    CharFeedback *charFeedback = b15 ? mMovePlayerData[i == 0].mFeedback
                                                     : mMovePlayerData[i].mFeedback;
                    HamPlayerData *hpd = TheGameData->Player(i);
                    bool cmp = charFeedback && hpd && hpd->IsPlaying();
                    if (!cmp) {
                        charFeedback->ResetErrors();
                    } else {
                        HamMove *curMove = mMovePlayerData[i].mCurMove;
                        DetectFrame *curFrame = frames[i];
                        if (curMove && curFrame) {
                            const FilterVersion *fv = curMove->FilterVer();
                            auto *errorNodes = fv->mErrorNodes;
                            if (curMove->Version() == kFilterVersionHam1) {
                                float float_arr[kNumLimbFeedbacks];
                                memset(float_arr, 0, sizeof(float_arr));
                                for (int n = 0; n < MoveFrame::kNumHam1Nodes; n++) {
                                    const Ham1NodeWeight &wt =
                                        curFrame->GetMoveFrame()->NodeWeightHam1(
                                            n, moveMode, curFrame->Mirror()
                                        );
                                    if (wt.unk0) {
                                        ErrorNode *node = errorNodes[n];
                                        float f34 = curFrame->BestNodeError(n).x;
                                        int limbs = node->GetFeedbackLimbs();
                                        for (int k = 0; k < kNumLimbFeedbacks; k++) {
                                            if ((1 << k) & limbs) {
                                                float_arr[k] += f34;
                                            }
                                        }
                                    }
                                }
                                for (int k = 0; k < kNumLimbFeedbacks; k++) {
                                    float frac = Clamp(0.0f, 1.0f, 1.0f - float_arr[k]);
                                    int move_rating;
                                    DetectFracToRating(
                                        frac, curMove->RatingOverride(), &move_rating
                                    );
                                    if (move_rating == kMoveRatingOk) {
                                        charFeedback->UpdateLimb(k, true);
                                    } else if (move_rating <= kMoveRatingPerfect) {
                                        charFeedback->UpdateLimb(k, false);
                                    }
                                }
                            } else {
                                MILO_ASSERT(fv->mType == kFilterVersionHam2, 0x460);
                                const MoveFrame *mf = curFrame->GetMoveFrame();
                                const Ham2FrameWeight &wt =
                                    mf->FrameWeight(curFrame->Mirror());
                                if (wt.unk0 > 0.5f) {
                                    for (int k = 0; k < kNumLimbFeedbacks; k++) {
                                        float f35 = curFrame->LimbPSNR(fv, 1 << k);
                                        if (wt.unk14[k] > wt.unk4[k]) {
                                            if (f35 > wt.unk14[k]) {
                                                charFeedback->UpdateLimb(k, false);
                                            } else if (f35 < wt.unk4[k]) {
                                                charFeedback->UpdateLimb(k, true);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                float f32 =
                    TheTaskMgr.TotalBeat() - (float)(TheTaskMgr.CurrentMeasure() * 4);
                for (int i = 0; i < 2; i++) {
                    auto &mpd = mMovePlayerData[i];
                    HamMove *curMove = mpd.mCurMove;
                    if (TheGameData->Player(i)->IsPlaying() && !InGracePeriod(i)
                        && curMove && curMove->Scored()) {
                        float frac;
                        if (TheMoveMgr->HasRoutine()) {
                            frac = mAsyncDetector->MoveRatingFrac(
                                i, (MoveAsyncDetector::RatingBar)0, curMove
                            );
                        } else {
                            frac = DetectFrac(i, -1);
                        }
                        Symbol autoplay = TheGameData->Player(i)->Autoplay();
                        if (frac > fracs[i] || !autoplay.Null()) {
                            frac =
                                DetectFracToRatingFrac(frac, curMove->RatingOverride());
                            MILO_ASSERT(mpd.mPhraseMeter, 0x49A);
                            mpd.mPhraseMeter->SetRatingFrac(frac, 4 - f32);
                            static Symbol rating_frac("rating_frac");
                            TheGameData->Player(i)->Provider()->SetProperty(
                                rating_frac, frac
                            );
                        }
                    }
                }
            }
        }
    }
}
