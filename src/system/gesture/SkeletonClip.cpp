#include "gesture/SkeletonClip.h"
#include "SkeletonClip.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/GestureMgr.h"
#include "gesture/Skeleton.h"
#include "gesture/SkeletonDir.h"
#include "gesture/SkeletonUpdate.h"
#include "hamobj/Difficulty.h"
#include "hamobj/HamDirector.h"
#include "hamobj/HamGameData.h"
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

SkeletonClip::SkeletonClip()
    : mRecordedFrames(sFrames), unk11f4(sCamFrame), unk11f8(sLoadedFile), unk11fc(-1),
      mDifficulty(kNumDifficulties), mWeighted(0), unk1230(0), unk1231(0),
      mFileStream(nullptr), unk1240(0), mAutoplay(false) {
    mRate = k30_fps;
}

SkeletonClip::~SkeletonClip() {
    if (*unk11f8 == mFile)
        *unk11f8 = gNullStr;
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
    SYNC_PROP(move_name, o.mName)
    SYNC_PROP(expected, o.mExpected)
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(SkeletonClip)
    SYNC_PROP(file, mFile)
    SYNC_PROP_SET(autoplay, mAutoplay, SetAutoplay(_val.Int()))
    SYNC_PROP_SET(time_recorded, DateTimeStr(), )
    SYNC_PROP_SET(build, mBuild, )
    SYNC_PROP_SET(song, Song(), )
    SYNC_PROP_SET(difficulty, DifficultyStr(), )
    SYNC_PROP(default_rating, mDefaultRating)
    SYNC_PROP_SET(weighted, mWeighted, mWeighted = _val.Int())
    SYNC_PROP(move_ratings, mMoveRatings)
    SYNC_PROP_SET(song_start_seconds, SongStartSeconds(), )
    SYNC_PROP_SET(song_end_seconds, SongEndSeconds(), )
    SYNC_PROP_SET(override_diff, mOverrideDiff, mOverrideDiff = _val.Int())
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
    bs << unk1240;
    bs << mAutoplay;
    bs << mDefaultRating;
    bs << mWeighted;
END_SAVES

BEGIN_COPYS(SkeletonClip)
    COPY_SUPERCLASS(RndAnimatable)
    COPY_SUPERCLASS(RndPollable)
    CREATE_COPY(SkeletonClip)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mFile)
        COPY_MEMBER(mMoveRatings)
        COPY_MEMBER(mAutoplay)
        COPY_MEMBER(mDefaultRating)
        COPY_MEMBER(mWeighted)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(SkeletonClip)
    const char *pathName = PathName(this);
    Symbol className = ClassName();
    int revs;
    bs >> revs;
    BinStreamRev d(bs, revs);
    if (d.rev == 10) {
        d.altRev = 1;
        d.rev = 9;
    }
    static const unsigned short gRevs[4] = { 9, 0, 1, 0 };
    if (d.rev > 9) {
        MILO_FAIL(
            "%s can't load new %s version %d > %d", pathName, className, d.rev, gRevs[0]
        );
    }
    if (d.altRev > 1) {
        MILO_FAIL(
            "%s can't load new %s alt version %d > %d",
            pathName,
            className,
            d.altRev,
            gRevs[2]
        );
    }
    RndAnimatable::Load(bs);
    Hmx::Object::Load(bs);
    bs >> mFile;
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
        d >> unk1240;
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
        d >> mDefaultRating;
    }
    if (d.altRev > 0) {
        bs >> mWeighted;
    } else {
        if (d.rev > 8) {
            static Symbol weighted("weighted");
            Symbol s;
            bs >> s;
            if (s != weighted) {
                mWeighted = 0;
            }
        } else {
            mWeighted = 1;
        }
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
    if ((int)mRecordedFrames->size() > (int)frame && frame >= 0) {
        RndAnimatable::SetFrame(frame, blend);
    }
}

float SkeletonClip::EndFrame() { return mRecordedFrames->size(); }

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
            unk1240++;
            float end = EndFrame() * 2;
            if (unk1240 >= end) {
                unk1240 = 0;
            }
            SetFrame(unk1240, 1);
        }
    }
}

const char *SkeletonClip::Song() const {
    if (mFile.empty()) {
        return gNullStr;
    } else if (!mSong.Null()) {
        return mSong.Str();
    } else {
        return "N/A";
    }
}

bool SkeletonClip::IsRecording() const { return unk1231 && !unk1230; }
const String &SkeletonClip::Path() const { return mFile; }

String SkeletonClip::DateTimeStr() const {
    if (mFile.empty())
        return gNullStr;
    else {
        String ret;
        mTimeRecorded.ToString(ret);
        return ret;
    }
}

const char *SkeletonClip::DifficultyStr() const {
    if (mFile.empty())
        return gNullStr;
    else if (mDifficulty == kNumDifficulties)
        return "N/A";
    else
        return DifficultyToSym(mDifficulty).Str();
}

bool SkeletonClip::IsFailClip() const {
    static Symbol move_bad("move_bad");
    return mDefaultRating == move_bad;
}

void SkeletonClip::SetPath(const char *path) { mFile = path; }

void SkeletonClip::EnableAlternateRecord(int recordingBuffer) {
    MILO_ASSERT(mFile.empty(), 0x7F);
    MILO_ASSERT_RANGE(recordingBuffer, 0, 4, 0x80);
    mRecordedFrames = &sFrames[recordingBuffer];
    unk11f4 = &sCamFrame[recordingBuffer];
    unk11f8 = &sLoadedFile[recordingBuffer];
}

BinStream &operator<<(BinStream &bs, const SkeletonClip::MoveRating &mr) {
    bs << mr.mName << mr.mExpected << mr.unkc;
    return bs;
}

BinStream &operator>>(BinStreamRev &d, SkeletonClip::MoveRating &mr) {
    d >> mr.mName;
    d >> mr.mExpected;
    if (d.altRev > 0) {
        int x;
        d >> x;
        mr.unkc = x;
    } else if (d.rev > 8) {
        Symbol s;
        d >> s;
        if (s == "weighted") {
            mr.unkc = 1;
        } else if (s == "unweighted") {
            mr.unkc = 0;
        } else {
            mr.unkc = 2;
        }
    } else {
        mr.unkc = 2;
    }
    return d.stream;
}

int SkeletonClip::NumMoveRatings() const { return mMoveRatings.size(); }

void SkeletonClip::WriteClipHeader(FileStream &stream) {
    stream << 8;
    GetDateAndTime(mTimeRecorded);
    stream << mTimeRecorded;
    stream << mSong;
    stream << mDifficulty;
    DataArray *arr = SystemConfig()->FindArray("version", false);
    const char *str = arr ? arr->Str(1) : "milo";
    stream << str;
    stream << (int)mRecordedFrames->size();
}

void SkeletonClip::WriteClipFrame(FileStream &stream, const RecordedFrame &recordedFrame) {
    stream << recordedFrame.unk0;
    stream << recordedFrame.unk4;
    stream << recordedFrame.unk8;
    stream << recordedFrame.unk18;
    stream << recordedFrame.unk28;
    if (recordedFrame.unk28) {
        for (int i = 0; i < kNumJoints; i++) {
            stream << recordedFrame.unk2c[i];
            stream << recordedFrame.unk16c[i];
        }
        stream << recordedFrame.unk1bc;
        stream << recordedFrame.unk1c0;
    }
    stream << recordedFrame.unk1c4;
}

void SkeletonClip::WriteClip(FileStream &stream) {
    WriteClipHeader(stream);
    for (int i = 0; i < mRecordedFrames->size(); i++) {
        WriteClipFrame(stream, (*mRecordedFrames)[i]);
    }
}

const SkeletonClip::MoveRating &SkeletonClip::GetMoveRating(int bar) const {
    MILO_ASSERT_RANGE(bar, 0, mMoveRatings.size(), 0x371);
    return mMoveRatings[bar];
}

void SkeletonClip::StopRecordingNoClear() {
    unk1231 = false;
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
    if (!mRecordedFrames->empty()) {
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
    const RecordedFrame *recordedFrame =
        RecordedFrameAt(*mRecordedFrames, f1, idk1, idk2);
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
        mRecordedFrames->clear();
    }
}

const SkeletonFrame *SkeletonClip::PollNewFrame() {
    int idk1, idk2;
    const RecordedFrame *recordedFrame = CurRecordedFrame(idk1, idk2);
    if (recordedFrame && recordedFrame->unk28) {
        recordedFrame->MakeSkeletonFrame(*unk11f4, 0);
        return unk11f4;
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
                if (clip)
                    clip->SetRecordedTime(dt);
                Symbol sym;
                fs >> sym;
                if (clip)
                    clip->SetSong(sym);
                Difficulty d;
                fs >> (int &)d;
                if (clip)
                    clip->SetDifficulty(d);
            }
            String ext;
            if (version > 5) {
                fs >> ext;
            } else
                ext = "milo";
            if (clip)
                clip->SetBuild(ext);
            if (version > 4) {
                int numFrames;
                fs >> numFrames;
                if (numFrames > mRecordedFrames->capacity()) {
                    MILO_NOTIFY(
                        "%i recorded frames greater than capacity %i",
                        numFrames,
                        mRecordedFrames->capacity()
                    );
                }
                mRecordedFrames->resize(numFrames);
                for (int i = 0; i < numFrames; i++) {
                    LoadFrame(fs, (*mRecordedFrames)[i], version);
                    if (fs.Fail()) {
                        MILO_NOTIFY(
                            "Bad clip data, truncating from %d frames to %d", numFrames, i
                        );
                        numFrames = i;
                        mRecordedFrames->resize(numFrames);
                        break;
                    }
                }
            } else {
                while (fs.Eof() == NotEof) {
                    RecordedFrame frame;
                    LoadFrame(fs, frame, version);
                    mRecordedFrames->push_back(frame);
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
        mRecordedFrames->clear();
        if (Dir()) {
            LoadClipFromFile(mFile, this);
            *unk11f8 = mFile;
        }
    }
}

void SkeletonClip::SetAutoplay(bool b1) {
    if (mFile.empty()) {
        MILO_NOTIFY("Recording hasn't been made yet, can't autoplay");
    } else if (!TheHamDirector) {
        MILO_NOTIFY("Autoplay not supported in song playback mode");
    } else {
        mAutoplay = b1;
    }
}

void SkeletonClip::StartXboxRecording(const char *cc) {
    MILO_ASSERT(!IsRecording(), 0x2F2);
    mFile = cc;
    unk1231 = true;
    if (TheLoadMgr.EditMode()) {
        SkeletonDir *dir = dynamic_cast<SkeletonDir *>(Dir());
        if (dir) {
            dir->SetSkeletonClip(nullptr);
        }
    }
    if (TheGameData && TheHamDirector) {
        mSong = TheGameData->GetSong();
        mDifficulty = TheGameData->Player(0)->GetDifficulty();
    } else {
        mSong = gNullStr;
        mDifficulty = kNumDifficulties;
    }
}
