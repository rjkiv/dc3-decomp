#include "gesture/SkeletonClip.h"
#include "SkeletonClip.h"
#include "gesture/ArchiveSkeleton.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonDir.h"
#include "gesture/SkeletonUpdate.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
#include "hamobj/HamMove.h"
#include "hamobj/HamPlayerData.h"
#include "hamobj/MoveDir.h"
#include "hamobj/ScoreUtl.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/Object.h"
#include "os/DateTime.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/System.h"
#include "rndobj/Anim.h"
#include "rndobj/Poll.h"
#include "utl/BinStream.h"
#include "utl/FileStream.h"
#include "utl/Loader.h"
#include "utl/MakeString.h"
#include "utl/Str.h"
#include "utl/Symbol.h"
#include <cstring>
#include <vector>

// do these correspond to kNumDifficulties?
std::vector<RecordedFrame> sFrames[4];
SkeletonFrame sCamFrame[4];
String sLoadedFile[4];

String SkeletonClip::sRemapClipSearch;
String SkeletonClip::sRemapClipReplace;

void ReserveFrames() {
    sFrames[0].reserve(18000);
    sFrames[1].reserve(18000);
    sFrames[2].reserve(800);
}

void RecordedFrame::MakeSkeletonFrame(SkeletonFrame &frame, int skel_idx) const {
    MILO_ASSERT_RANGE(skel_idx, 0, 6, 0x2e);
    memset(&frame, 0, sizeof(SkeletonFrame));
    frame.mFrameNumber = mFrameNumber;
    frame.mElapsedMs = mElapsedMs;
    frame.mUpVector = mUpVector;
    frame.mFloorPlane = mFloorPlane;
    SkeletonData &data = frame.mSkeletonDatas[skel_idx];
    data.mTracking = mTracked ? kSkeletonTracked : kSkeletonNotTracked;
    memcpy(data.mJointPositions, mJointPositions, 0x140);
    memcpy(data.mJointConfidences, mJointConfidences, 0x50);
    data.mQualityFlags = mQualityFlags;
    data.mTrackingID = mTrackingID;
    data.mCenter = data.mJointPositions[0];
}

SkeletonClip::SkeletonClip()
    : mFrames(sFrames), mCamFrame(sCamFrame), mLoadedFile(sLoadedFile),
      mRecordingPlayer(-1), mRecordDifficulty(kNumDifficulties),
      mWeighting(kClipNotWeighted), mSuspended(0), mRecording(0), mFileStream(nullptr),
      mInternalFrame(0), mAutoplay(false) {
    SetRate(k30_fps);
}

SkeletonClip::~SkeletonClip() {
    if (*mLoadedFile == mFile)
        *mLoadedFile = gNullStr;
    RELEASE(mFileStream);
}

BEGIN_HANDLERS(SkeletonClip)
    HANDLE_ACTION(xbox_load_frames, LoadClip(false))
    HANDLE_ACTION(xbox_start_record, StartXboxRecording(_msg->Str(2)))
    HANDLE_ACTION(start_recording, 0) // i guess this was stubbed out or something
    HANDLE_ACTION(stop_recording, StopRecording())
    HANDLE_EXPR(is_recording, IsRecording())
    HANDLE_ACTION(fill_move_ratings, FillMoveRatings())
    HANDLE_ACTION(apply_override_diff, 0) // ditto
    HANDLE_SUPERCLASS(RndAnimatable)
    HANDLE_SUPERCLASS(RndPollable)
    HANDLE_SUPERCLASS(CameraInput)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(SkeletonClip::MoveRating)
    SYNC_PROP(move_name, o.mMoveName)
    SYNC_PROP(expected, o.mExpectedRating)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(SkeletonClip)
    SYNC_PROP(file, mFile)
    SYNC_PROP_SET(autoplay, mAutoplay, SetAutoplay(_val.Int()))
    SYNC_PROP_SET(time_recorded, DateTimeStr(), )
    SYNC_PROP_SET(build, mRecordBuild, )
    SYNC_PROP_SET(song, SongStr(), )
    SYNC_PROP_SET(difficulty, DifficultyStr(), )
    SYNC_PROP(default_rating, mDefaultExpectedRating)
    SYNC_PROP_SET(weighted, mWeighting, mWeighting = (SkeletonClipWeighting)_val.Int())
    SYNC_PROP(move_ratings, mMoveRatings)
    SYNC_PROP_SET(song_start_seconds, SongStartSeconds(), )
    SYNC_PROP_SET(song_end_seconds, SongEndSeconds(), )
    SYNC_PROP_SET(
        override_diff, mOverrideDifficulty, mOverrideDifficulty = (Difficulty)_val.Int()
    )
    SYNC_SUPERCLASS(RndAnimatable)
    SYNC_SUPERCLASS(RndPollable)
    SYNC_SUPERCLASS(CameraInput)
END_PROPSYNCS

BEGIN_SAVES(SkeletonClip)
    SAVE_REVS(9, 1)
    SAVE_SUPERCLASS(RndAnimatable)
    SAVE_SUPERCLASS(RndPollable)
    if (!bs.Cached()) {
        bs << mFile;
        bs << mMoveRatings;
    } else {
        bs << 0;
        std::vector<MoveRating> moves;
        bs << moves;
    }
    bs << mInternalFrame;
    bs << mAutoplay;
    bs << mDefaultExpectedRating;
    bs << mWeighting;
END_SAVES

BEGIN_COPYS(SkeletonClip)
    COPY_SUPERCLASS(RndAnimatable)
    COPY_SUPERCLASS(RndPollable)
    CREATE_COPY(SkeletonClip)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mFile)
        COPY_MEMBER(mMoveRatings)
        COPY_MEMBER(mAutoplay)
        COPY_MEMBER(mDefaultExpectedRating)
        COPY_MEMBER(mWeighting)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(9, 1)

BEGIN_LOADS(SkeletonClip)
    const char *pathName = PathName(this);
    Symbol className = ClassName();
    LOAD_REVS(bs);
    if (d.rev == 10) {
        d.altRev = 1;
        d.rev = 9;
    }
    if (d.rev > 9) {
        MILO_FAIL(
            "%s can't load new %s version %d > %d", pathName, className, d.rev, gRev
        );
    }
    if (d.altRev > 1) {
        MILO_FAIL(
            "%s can't load new %s alt version %d > %d",
            pathName,
            className,
            d.altRev,
            gAltRev
        );
    }
    LOAD_SUPERCLASS(RndAnimatable)
    LOAD_SUPERCLASS(Hmx::Object)
    d >> mFile;
    if (d.rev < 5) {
        mFile = MakeString(
            "%s/%s",
            FileRelativePath(FileRoot(), FileGetPath(Dir()->GetPathName())),
            mFile
        );
    } else {
        const char *relative =
            FileRelativePath(FileRoot(), FileGetPath(Dir()->GetPathName()));
        if (!strstr(mFile.c_str(), relative)) {
            mFile =
                MakeString("%s/%s", relative, mFile.substr(mFile.find_last_of("/") + 1));
        }
    }
    if (!sRemapClipSearch.empty()) {
        char buffer[1024];
        SearchReplace(
            mFile.c_str(), sRemapClipSearch.c_str(), sRemapClipReplace.c_str(), buffer
        );
        mFile = buffer;
    }
    if (d.rev > 7) {
        d >> mMoveRatings;
    }
    if (d.rev > 0) {
        d >> mInternalFrame;
        d >> mAutoplay;
        if (d.rev < 4) {
            bool b;
            d >> b;
        }
    }
    if (d.rev > 1 && d.rev < 7) {
        int x;
        d >> x;
        d >> x;
    }
    if (d.rev > 5) {
        d >> mDefaultExpectedRating;
    }
    if (d.altRev > 0) {
        int w;
        d >> w;
        mWeighting = (SkeletonClipWeighting)w;
    } else if (d.rev > 8) {
        static Symbol weighted("weighted");
        Symbol s;
        d >> s;
        if (s == weighted) {
            mWeighting = kClipWeighted;
        } else {
            mWeighting = kClipNotWeighted;
        }
    } else {
        mWeighting = kClipWeighted;
    }
END_LOADS

void SkeletonClip::StartAnim() {
    if (IsRecording()) {
        MILO_NOTIFY("Cannot start animating until finished recording.");
    } else {
        SkeletonDir *dir = dynamic_cast<SkeletonDir *>(Dir());
        if (dir) {
            dir->SetSkeletonClip(this);
        }
    }
}

void SkeletonClip::SetFrame(float frame, float blend) {
    if ((int)mFrames->size() > (int)frame && frame >= 0) {
        RndAnimatable::SetFrame(frame, blend);
    }
}

float SkeletonClip::EndFrame() { return mFrames->size(); }

void SkeletonClip::Poll() {
    if (IsRecording()) {
        SkeletonUpdateHandle handle = SkeletonUpdate::InstanceHandle();
        const SkeletonFrame *frame = handle.GetCameraInput()->NewFrame();
        if (frame) {
            PollRecording(*frame);
        }
    } else {
        SkeletonDir *dir = dynamic_cast<SkeletonDir *>(Dir());
        if (mAutoplay && TheLoadMgr.EditMode() && dir && dir->TestClip() == this) {
            if (++mInternalFrame >= EndFrame() * 2) {
                mInternalFrame = 0;
            }
            SetFrame(mInternalFrame / 2, 1);
        }
    }
}

bool SkeletonClip::PrevSkeleton(
    const Skeleton &current, int previous_ms, ArchiveSkeleton &previous, int &elapsed_ms
) const {
    if (current.SkeletonIndex() == 0) {
        int index;
        int loop_count;
        if (CurRecordedFrame(index, loop_count)) {
            auto &frames = *mFrames;
            float f6 =
                frames.back().mSeconds * (float)loop_count + frames[index].mSeconds;
            float f5 = -((float)previous_ms / 1000 - f6);
            const RecordedFrame *rf = RecordedFrameAt(frames, f5, index, loop_count);
            if (rf) {
                elapsed_ms = (f6 - f5) * 1000;
                SkeletonFrame skeletonFrame;
                rf->MakeSkeletonFrame(skeletonFrame, current.SkeletonIndex());
                Skeleton skeleton;
                skeleton.Poll(current.SkeletonIndex(), skeletonFrame);
                previous.Set(skeleton);
                return true;
            }
        }
    }
    return false;
}

const char *SkeletonClip::SongStr() const {
    if (mFile.empty()) {
        return gNullStr;
    } else if (!mRecordSong.Null()) {
        return mRecordSong.Str();
    } else {
        return "N/A";
    }
}

bool SkeletonClip::IsRecording() const { return mRecording && !mSuspended; }
const String &SkeletonClip::Path() const { return mFile; }

String SkeletonClip::DateTimeStr() const {
    if (mFile.empty())
        return gNullStr;
    else {
        String ret;
        mRecordDateTime.ToString(ret);
        return ret;
    }
}

const char *SkeletonClip::DifficultyStr() const {
    if (mFile.empty())
        return gNullStr;
    else if (mRecordDifficulty == kNumDifficulties)
        return "N/A";
    else
        return DifficultyToSym(mRecordDifficulty).Str();
}

bool SkeletonClip::IsFailClip() const {
    static Symbol move_bad("move_bad");
    return mDefaultExpectedRating == move_bad;
}

void SkeletonClip::SetPath(const char *path) { mFile = path; }

void SkeletonClip::EnableAlternateRecord(int recordingBuffer) {
    MILO_ASSERT(mFile.empty(), 0x7F);
    MILO_ASSERT_RANGE(recordingBuffer, 0, 4, 0x80);
    mFrames = &sFrames[recordingBuffer];
    mCamFrame = &sCamFrame[recordingBuffer];
    mLoadedFile = &sLoadedFile[recordingBuffer];
}

BinStream &operator<<(BinStream &bs, const SkeletonClip::MoveRating &mr) {
    bs << mr.mMoveName << mr.mExpectedRating << mr.mWeighting;
    return bs;
}

BinStream &operator>>(BinStreamRev &d, SkeletonClip::MoveRating &mr) {
    d >> mr.mMoveName;
    d >> mr.mExpectedRating;
    if (d.altRev > 0) {
        int x;
        d >> x;
        mr.mWeighting = (SkeletonClipWeighting)x;
    } else if (d.rev > 8) {
        Symbol s;
        d >> s;
        if (s == "weighted") {
            mr.mWeighting = kClipWeighted;
        } else if (s == "unweighted") {
            mr.mWeighting = kClipNotWeighted;
        } else {
            mr.mWeighting = kClipDefaultWeighting;
        }
    } else {
        mr.mWeighting = kClipDefaultWeighting;
    }
    return d;
}

int SkeletonClip::NumMoveRatings() const { return mMoveRatings.size(); }

void SkeletonClip::WriteClipHeader(FileStream &stream) {
    stream << 8;
    GetDateAndTime(mRecordDateTime);
    stream << mRecordDateTime;
    stream << mRecordSong;
    stream << mRecordDifficulty;
    DataArray *arr = SystemConfig()->FindArray("version", false);
    if (arr) {
        stream << arr->Str(1);
    } else {
        stream << "milo";
    }
    stream << mFrames->size();
}

void SkeletonClip::WriteClipFrame(FileStream &stream, const RecordedFrame &recordedFrame) {
    stream << recordedFrame.mFrameNumber;
    stream << recordedFrame.mElapsedMs;
    stream << recordedFrame.mUpVector;
    stream << recordedFrame.mFloorPlane;
    stream << recordedFrame.mTracked;
    if (recordedFrame.mTracked) {
        for (int i = 0; i < kNumJoints; i++) {
            stream << recordedFrame.mJointPositions[i];
            stream << recordedFrame.mJointConfidences[i];
        }
        stream << recordedFrame.mQualityFlags;
        stream << recordedFrame.mTrackingID;
    }
    stream << recordedFrame.mSeconds;
}

void SkeletonClip::WriteClip(FileStream &stream) {
    WriteClipHeader(stream);
    for (int i = 0; i < mFrames->size(); i++) {
        WriteClipFrame(stream, (*mFrames)[i]);
    }
}

const SkeletonClip::MoveRating &SkeletonClip::GetMoveRating(int bar) const {
    MILO_ASSERT_RANGE(bar, 0, mMoveRatings.size(), 0x371);
    return mMoveRatings[bar];
}

void SkeletonClip::StopRecordingNoClear() {
    mRecording = false;
    mFileStream = new FileStream(mFile.c_str(), FileStream::kWrite, true);
    if (mFileStream->Fail()) {
        MILO_FAIL("Recording failed; could not open output file (%s).", mFile.c_str());
    } else {
        MILO_ASSERT(mFileStream, 0x339);
        WriteClip(*mFileStream);
    }
    RELEASE(mFileStream);
}

void SkeletonClip::FlushMoveRecord(const char *name) {
    if (!mFrames->empty()) {
        StopRecordingNoClear();
        String clipStr(name);
        const char *clipPath = MakeString("devkit:\\%s.clp", clipStr);
        MILO_LOG("Starting song recording: %s\n", clipPath);
        StartXboxRecording(clipPath);
    } else
        MILO_LOG("can't save empty recording\n");
}

bool SkeletonClip::SkeletonFrameAt(float f1, SkeletonFrame &frame) const {
    int idk1, idk2;
    const RecordedFrame *recordedFrame = RecordedFrameAt(*mFrames, f1, idk1, idk2);
    if (recordedFrame) {
        recordedFrame->MakeSkeletonFrame(frame, 0);
        return true;
    } else
        return false;
}

void SkeletonClip::StopRecording() {
    if (!IsRecording()) {
        MILO_NOTIFY("You must start recording first");
    } else {
        StopRecordingNoClear();
        mFrames->clear();
    }
}

const SkeletonFrame *SkeletonClip::PollNewFrame() {
    int idk1, idk2;
    const RecordedFrame *recordedFrame = CurRecordedFrame(idk1, idk2);
    if (recordedFrame && recordedFrame->mTracked) {
        recordedFrame->MakeSkeletonFrame(*mCamFrame, 0);
        return mCamFrame;
    } else
        return nullptr;
}

DataNode OnSkeletonClipRemapPaths(DataArray *a) {
    SkeletonClip::sRemapClipSearch = a->Str(1);
    SkeletonClip::sRemapClipReplace = a->Str(2);
    return 1;
}

void SkeletonClip::Init() {
    REGISTER_OBJ_FACTORY(SkeletonClip);
    DataRegisterFunc("skeleton_clip_remap_paths", OnSkeletonClipRemapPaths);
}

void SkeletonClip::LoadClipFromFile(String str, SkeletonClip *clip) {
    FileStream fs(str.c_str(), FileStream::kRead, true);
    if (fs.Fail()) {
        MILO_NOTIFY(
            "%s: loading failed; could not open file (%s).",
            clip ? PathName(clip) : gNullStr,
            str.c_str()
        );
        return;
    } else if (fs.Size() == 0) {
        MILO_NOTIFY("File %s is empty", str.c_str());
        return;
    } else {
        int version;
        fs >> version;
        if (version == 0) {
            MILO_NOTIFY(
                "Version 0 clips no longer supported, you should delete %s", str.c_str()
            );
            return;
        } else {
            if (version > 1 && version < 8) {
                bool bbb;
                fs >> bbb;
            }
            if (version > 3) {
                DateTime dt;
                fs >> dt;
                if (clip) {
                    clip->mRecordDateTime = dt;
                }
                Symbol sym;
                fs >> sym;
                if (clip) {
                    clip->mRecordSong = sym;
                }
                Difficulty d;
                fs >> (int &)d;
                if (clip) {
                    clip->mRecordDifficulty = d;
                }
            }
            String ext;
            if (version > 5) {
                fs >> ext;
            } else
                ext = "milo";
            if (clip) {
                clip->mRecordBuild = ext;
            }
            if (version > 4) {
                int numFrames;
                fs >> numFrames;
                if (numFrames > mFrames->capacity()) {
                    MILO_NOTIFY(
                        "%i recorded frames greater than capacity %i",
                        numFrames,
                        mFrames->capacity()
                    );
                }
                mFrames->resize(numFrames);
                for (int i = 0; i < numFrames; i++) {
                    LoadFrame(fs, (*mFrames)[i], version);
                    if (fs.Fail()) {
                        MILO_NOTIFY(
                            "Bad clip data, truncating from %d frames to %d", numFrames, i
                        );
                        numFrames = i;
                        mFrames->resize(numFrames);
                        break;
                    }
                }
            } else {
                while (fs.Eof() == NotEof) {
                    RecordedFrame frame;
                    LoadFrame(fs, frame, version);
                    mFrames->push_back(frame);
                }
            }
        }
    }
}

void SkeletonClip::LoadClip(bool tool_sync) {
    MILO_ASSERT(!tool_sync || TheLoadMgr.EditMode(), 0x3EC);
    if (IsRecording()) {
        MILO_NOTIFY("Cannot open file while still recording");
    } else {
        mFrames->clear();
        if (Dir()) {
            LoadClipFromFile(mFile, this);
            *mLoadedFile = mFile;
        }
    }
}

void SkeletonClip::SetAutoplay(bool b1) {
    if (mFile.empty()) {
        MILO_NOTIFY("Recording hasn't been made yet, can't autoplay");
        return;
    }
    if (TheHamDirector) {
        MILO_NOTIFY("Autoplay not supported in song playback mode");
        return;
    }
    mAutoplay = b1;
}

void SkeletonClip::StartXboxRecording(const char *cc) {
    MILO_ASSERT(!IsRecording(), 0x2F2);
    mFile = cc;
    mRecording = true;
    if (TheLoadMgr.EditMode()) {
        SkeletonDir *dir = dynamic_cast<SkeletonDir *>(Dir());
        if (dir) {
            dir->SetSkeletonClip(nullptr);
        }
    }
    if (TheGameData && TheHamDirector) {
        mRecordSong = TheGameData->GetSong();
        mRecordDifficulty = TheGameData->Player(0)->GetDifficulty();
    } else {
        mRecordSong = gNullStr;
        mRecordDifficulty = kNumDifficulties;
    }
}

float SkeletonClip::SongStartSeconds() const {
    if (!mFile.empty() && !mFrames->empty() && !mRecordSong.Null()) {
        return mFrames->front().mSeconds;
    }
    return 0;
}

float SkeletonClip::SongEndSeconds() const {
    if (!mFile.empty() && !mFrames->empty() && !mRecordSong.Null()) {
        float f = mFrames->back().mSeconds;
        if (IsFailClip()) {
            f *= 1000.0f;
        }
        return f;
    }
    return 0;
}

void SkeletonClip::FillMoveRatings() {
    if (mRecordDifficulty == kNumDifficulties) {
        MILO_NOTIFY("Can\'t fill move ratings, this is not a song recording");
        return;
    }

    if (TheHamDirector) {
        MoveDir *moveDir = dynamic_cast<MoveDir *>(Dir());
        if (moveDir) {
            std::vector<HamMoveKey> keys;
            TheHamDirector->MoveKeys(mRecordDifficulty, moveDir, keys);
            static Symbol Default("default");
            mMoveRatings.resize(keys.size());
            for (int i = 0; i < keys.size(); i++) {
                mMoveRatings[i].mMoveName = keys[i].move->Name();
                mMoveRatings[i].mExpectedRating = Default;
                mMoveRatings[i].mWeighting = kClipDefaultWeighting;
            }
        }
    }
}

void SkeletonClip::LoadFrame(BinStream &bs, RecordedFrame &frame, int i) {
    if (i > 6) {
        bs >> frame.mFrameNumber;
    } else {
        frame.mFrameNumber = 0;
    }

    if (i > 1) {
        bs >> frame.mElapsedMs;
        bs >> frame.mUpVector;
        bs >> frame.mFloorPlane;
    } else {
        frame.mElapsedMs = 0x21;
        frame.mUpVector.Set(0, 1.0f, 0);
        frame.mFloorPlane.Set(0, 0, 0, 0);
    }

    bs >> frame.mTracked;
    if (frame.mTracked || i < 2) {
        for (int i = 0; i < kNumJoints; i++) {
            bs >> frame.mJointPositions[i];
            bs >> (int &)frame.mJointConfidences[i];
        }
        bs >> frame.mQualityFlags;
        bs >> frame.mTrackingID;
    }

    if (i == 1) {
        String s = "";
        bs >> s;
    } else if (i > 2) {
        bs >> frame.mSeconds;
    }
}

void SkeletonClip::SwapMoveRecord() {
    int size = mFrames->size() - 400;
    if (size > 0) {
        mFrames->erase(mFrames->begin(), mFrames->begin() + size);
    }
}

void SkeletonClip::PollRecording(const SkeletonFrame &frame) {
    if (IsRecording()) {
        float songSeconds = 0;
        if (TheHamDirector) {
            songSeconds = MoveDir::SongSeconds();
        }
        if (TheHamDirector && !mFrames->empty()
            && mFrames->back().mSeconds >= songSeconds) {
            return;
        }

        RecordedFrame recordedFrame;
        recordedFrame.mFrameNumber = frame.mFrameNumber;
        recordedFrame.mElapsedMs = frame.mElapsedMs;
        recordedFrame.mUpVector = frame.mUpVector;
        recordedFrame.mFloorPlane = frame.mFloorPlane;

        int active_skel_idx = -1;
        if (mRecordingPlayer == -1) {
            active_skel_idx = TheGestureMgr->GetActiveSkeletonIndex();
        } else {
            HamPlayerData *player = TheGameData->Player(mRecordingPlayer);
            active_skel_idx = TheGestureMgr->GetSkeletonIndexByTrackingID(
                player->GetSkeletonTrackingID()
            );
        }

        if (active_skel_idx != -1) {
            MILO_ASSERT_RANGE(active_skel_idx, 0, 6, 0x2a9);
            const SkeletonData &data = frame.mSkeletonDatas[active_skel_idx];
            recordedFrame.mTracked = data.mTracking == kSkeletonTracked;
            memcpy(
                recordedFrame.mJointPositions,
                data.mJointPositions,
                sizeof(Vector3) * kNumJoints
            );
            memcpy(
                recordedFrame.mJointConfidences,
                data.mJointConfidences,
                sizeof(JointConfidence) * kNumJoints
            );
            recordedFrame.mQualityFlags = data.mQualityFlags;
            recordedFrame.mTrackingID = data.mTrackingID;
            recordedFrame.mSeconds = songSeconds;

            int size = mFrames->size();
            int cap = mFrames->capacity();
            if (cap == size) {
                MILO_LOG(
                    "Can't record any more frames, reached capacity (%i)\n",
                    mFrames->capacity()
                );
            } else {
                mFrames->push_back(recordedFrame);
            }
        }
    }
}

const RecordedFrame *SkeletonClip::CurRecordedFrame(int &index, int &loop_count) const {
    bool active = mRecording && !mSuspended;
    if (!active) {
        if (TheHamDirector) {
            float secs = MoveDir::SongSeconds();
            return RecordedFrameAt(*mFrames, secs, index, loop_count);
        }
        if (mFrames->size() != 0 && mFrames->size() > GetFrame()) {
            loop_count = 0;
            index = GetFrame();
            return &(*mFrames)[index];
        }
    }
    return nullptr;
}

struct RecordedFrameCmp { /* Size=0x1 */
    bool operator()(float secs, const RecordedFrame &frame) const {
        return secs < frame.mSeconds;
    }
    bool operator()(const RecordedFrame &frame, float secs) const {
        return frame.mSeconds < secs;
    }
};

const RecordedFrame *SkeletonClip::RecordedFrameAt(
    const std::vector<RecordedFrame> &frames,
    float song_seconds,
    int &index,
    int &loop_count
) {
    loop_count = 0;
    if (!frames.empty()) {
        int numFrames = frames.size();
        while (song_seconds > frames.back().mSeconds) {
            if (loop_count >= numFrames) {
                return nullptr;
            }
            song_seconds -= frames.back().mSeconds;
            loop_count++;
        }
        auto frameBegin = frames.begin();
        auto frameEnd = frames.end();
        auto it =
            std::lower_bound(frameBegin, frameEnd, song_seconds, RecordedFrameCmp());
        if (it != frameEnd) {
            index = it - frameBegin;
            return it;
        }
    }

    return nullptr;
}
