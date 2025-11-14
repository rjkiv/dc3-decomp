#include "char/CharClipSet.h"
#include "char/CharClipGroup.h"
#include "char/Character.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"

CharClipSet::CharClipSet()
    : mCharFilePath(), mPreviewChar(this), mPreviewClip(this), mStillClip(this) {
    ResetPreviewState();
    SetRate(k1_fpb);
}

CharClipSet::~CharClipSet() {}

BEGIN_PROPSYNCS(CharClipSet)
    SYNC_PROP(char_file_path, mCharFilePath)
    SYNC_PROP(preview_clip, mPreviewClip)
    SYNC_PROP(still_clip, mStillClip)
    SYNC_PROP(filter_flags, mFilterFlags)
    SYNC_PROP_SET(bpm, mBpm, SetBpm(_val.Int()))
    SYNC_PROP(preview_walk, mPreviewWalk)
    SYNC_SUPERCLASS(ObjectDir)
END_PROPSYNCS

BEGIN_SAVES(CharClipSet)
    SAVE_REVS(24, 0)
    SAVE_SUPERCLASS(ObjectDir)
    bs << mCharFilePath;
    bs << mPreviewClip;
    bs << mFilterFlags;
    bs << mBpm;
    bs << mPreviewWalk;
    bs << mStillClip;
END_SAVES

BEGIN_COPYS(CharClipSet)
    COPY_SUPERCLASS(ObjectDir)
    CREATE_COPY(CharClipSet)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mCharFilePath)
        COPY_MEMBER(mPreviewClip)
        COPY_MEMBER(mFilterFlags)
        COPY_MEMBER(mBpm)
        COPY_MEMBER(mPreviewWalk)
        COPY_MEMBER(mStillClip)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(CharClipSet)
    ObjectDir::Load(bs);
END_LOADS

void CharClipSet::PreSave(BinStream &bs) {
    if (mPreviewChar)
        mPreviewChar->SetName("", 0);
    if (bs.Cached()) {
        ResetPreviewState();
        ResetEditorState();
    }
}

void CharClipSet::PostSave(BinStream &bs) {
    ObjectDir::PostSave(bs);
    if (mPreviewChar) {
        mPreviewChar->SetName("preview_character", this);
        mPreviewChar->Enter();
    }
}

void CharClipSet::PreLoad(BinStream &) {}

void CharClipSet::PostLoad(BinStream &) {}

void CharClipSet::SetFrame(float frame, float blend) {}

float CharClipSet::StartFrame() {
    if (mPreviewClip)
        return mPreviewClip->StartBeat();
    else
        return 0;
}

float CharClipSet::EndFrame() {
    if (mPreviewClip)
        return mPreviewClip->EndBeat();
    else
        return 0;
}

void CharClipSet::ListDrawChildren(std::list<class RndDrawable *> &) {}

void CharClipSet::ResetEditorState() {
    ResetPreviewState();
    ObjectDir::ResetEditorState();
}

void CharClipSet::SetBpm(int bpm) {
    static Symbol sBpm("bpm");
    mBpm = bpm;
}

void CharClipSet::ResetPreviewState() {
    delete mPreviewChar;
    mPreviewClip = 0;
    mStillClip = 0;
    mCharFilePath.SetRoot("");
    mFilterFlags = 0;
    mBpm = 90;
    mPreviewWalk = false;
}

void CharClipSet::SortGroups() {
    for (ObjDirItr<CharClipGroup> it(this, false); it != 0; ++it) {
        it->Sort();
    }
}

void CharClipSet::LoadCharacter() {
    MILO_ASSERT(TheLoadMgr.EditMode(), 0x14b);
    delete mPreviewChar;
    ObjectDir *loadedobj = DirLoader::LoadObjects(mCharFilePath, 0, 0);
    ObjectDir *dummy = dynamic_cast<RndDir *>(loadedobj);
    mPreviewChar = dynamic_cast<RndDir *>(dummy);
    Character *theChar = dynamic_cast<Character *>(dummy);
    if (mPreviewChar && !theChar) {
        for (ObjDirItr<Character> it(mPreviewChar, true); it != nullptr; ++it) {
            mPreviewChar = it;
            break;
        }
    }
    if (mPreviewChar) {
        mPreviewChar->Enter();
        mPreviewChar->SetName("preview_character", this);
    } else
        MILO_NOTIFY(
            "Preview character can only be loaded if the CharClipSet is the top-level directory."
        );
}

void CharClipSet::RecenterAll() { MILO_NOTIFY("You can only recenter clips from PC"); }

DataNode CharClipSet::OnListClips(DataArray *) {
    std::list<CharClip *> clips;
    for (ObjDirItr<CharClip> it(this, true); it != nullptr; ++it) {
        if ((mFilterFlags & it->Flags()) == mFilterFlags) {
            clips.push_back(it);
        }
    }
    clips.sort(ObjNameSort());
    DataArray *arr = new DataArray(clips.size() + 1);
    arr->Node(0) = NULL_OBJ;
    int idx = 1;
    for (std::list<CharClip *>::iterator it = clips.begin(); it != clips.end(); ++it) {
        arr->Node(idx++) = *it;
    }
    DataNode ret(arr, kDataArray);
    arr->Release();
    return ret;
}

BEGIN_HANDLERS(CharClipSet)
    HANDLE_ACTION(sort_groups, SortGroups())
    HANDLE_ACTION(recenter_all, RecenterAll())
    HANDLE_ACTION(load_character, LoadCharacter())
    HANDLE(list_clips, OnListClips)
    HANDLE_SUPERCLASS(ObjectDir)
END_HANDLERS
