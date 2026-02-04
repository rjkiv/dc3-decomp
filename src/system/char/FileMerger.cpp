#include "char/FileMerger.h"
#include "CharClipGroup.h"
#include "char/CharPollGroup.h"
#include "obj/Dir.h"
#include "obj/DirLoader.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "rndobj/Group.h"
#include "rndobj/Mat.h"
#include "rndobj/Rnd.h"
#include "rndobj/Tex.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "utl/PoolAlloc.h"

FileMerger *FileMerger::sFmDeleting;

class NullLoader : public Loader {
public:
    NullLoader(const FilePath &fp, LoaderPos pos, Loader::Callback *cb)
        : Loader(fp, pos), mCallback(cb) {}
    virtual ~NullLoader() {
        if (mCallback)
            mCallback->FailedLoading(this);
    }
    virtual const char *DebugText() {
        return MakeString("NullLoader: %s", mFile.c_str());
    }
    virtual bool IsLoaded() const { return false; }
    virtual const char *StateName() const { return "NullLoader"; }

    POOL_OVERLOAD(NullLoader, 0x1F);

protected:
    virtual void PollLoading() {
        mCallback->FinishLoading(this);
        mCallback = nullptr;
        delete this;
    }

    Loader::Callback *mCallback; // 0x18
};

void FileMerger::Merger::Clear(bool b1) {
    mLoaded.Set(FilePath::Root().c_str(), "");
    Hmx::Object *owner = mLoadedObjects.Owner();
    if (owner != sFmDeleting) {
        static Message msg("on_pre_clear", 0);
        msg[0] = mName;
        owner->HandleType(msg);
    }
    while (!mLoadedObjects.empty()) {
        Hmx::Object *front = mLoadedObjects.front();
        delete front;
    }
    ObjectDir *mergerDir = MergerDir();
    if (mergerDir) {
        while (!mLoadedSubdirs.empty()) {
            ObjectDir *curSubdir = mLoadedSubdirs.front();
            mLoadedSubdirs.pop_front();
            mergerDir->RemoveSubDir(curSubdir);
        }
    } else {
        mLoadedSubdirs.clear();
    }
    if (b1 && !TheRnd.GetUnk1b4()) {
        TheRnd.BeginDrawing();
        TheRnd.EndDrawing();
    }
}

FileMerger::FileMerger()
    : mMergers(this), mAsyncLoad(0), mLoadingLoad(0), mCurLoader(0), mFilter(0),
      mHeap(GetCurrentHeapNum()), mOrganizer(this) {
    MILO_ASSERT(MemNumHeaps() == 0 || (mHeap != kNoHeap && mHeap != kSystemHeap), 0x86);
}

FileMerger::~FileMerger() {
    FileMerger *old = sFmDeleting;
    sFmDeleting = this;
    Clear();
    sFmDeleting = old;
}

BEGIN_HANDLERS(FileMerger)
    HANDLE_EXPR(loaded, FindMerger(_msg->Sym(2), true)->mLoaded)
    HANDLE(select, OnSelect)
    HANDLE(start_load, OnStartLoad)
    HANDLE_ACTION(clear, Clear())
    HANDLE_ACTION(clear_selections, ClearSelections())
    HANDLE_EXPR(merger_index, FindMergerIndex(_msg->Sym(2), _msg->Int(3)))
    HANDLE_EXPR(is_loading, 0)
    HANDLE_ACTION(clear_filter, mFilter = nullptr)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_CUSTOM_PROPSYNC(FileMerger::Merger)
    SYNC_PROP(name, o.mName)
    SYNC_PROP(selected, o.mSelected)
    SYNC_PROP(loaded, o.mLoaded)
    SYNC_PROP(dir, o.mDir)
    SYNC_PROP(proxy, o.mProxy)
    SYNC_PROP(subdirs, (int &)o.mSubdirs)
    SYNC_PROP(preclear, o.mPreClear)
    SYNC_PROP(loaded_objects, o.mLoadedObjects) {
        static Symbol _s("loaded_subdirs");
        if (sym == _s && (_op & (kPropGet | kPropSize)))
            return PropSync(o.mLoadedSubdirs, _val, _prop, _i + 1, _op);
    }
END_CUSTOM_PROPSYNC

BEGIN_PROPSYNCS(FileMerger)
    SYNC_PROP(mergers, mMergers)
    SYNC_PROP(disable_all, sDisableAll)
    SYNC_PROP_SET(loading_load, mLoadingLoad, )
    SYNC_PROP_SET(async_load, mAsyncLoad, )
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BinStream &operator<<(BinStream &bs, const FileMerger::Merger &fm) {
    bs << fm.mName;
    bs << fm.mSelected;
    bs << fm.mLoaded;
    bs << fm.mDir;
    bs << fm.mProxy;
    bs << fm.mSubdirs;
    bs << fm.mPreClear;
    return bs;
}

BEGIN_SAVES(FileMerger)
    SAVE_REVS(5, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    bs << mMergers;
END_SAVES

BEGIN_COPYS(FileMerger)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(FileMerger)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mMergers)
    END_COPYING_MEMBERS
END_COPYS

BEGIN_LOADS(FileMerger)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

void FileMerger::PreSave(BinStream &) { Clear(); }
void FileMerger::PostSave(BinStream &) { StartLoadInternal(false, false); }

BinStreamRev &operator>>(BinStreamRev &d, FileMerger::Merger &fm) {
    d >> fm.mName;
    d >> fm.mSelected;
    d >> fm.mLoaded;
    d >> fm.mDir;
    if (d.rev > 0) {
        if (d.rev != 4) {
            d >> fm.mProxy;
        }
        d >> (int &)fm.mSubdirs;
        if (d.rev > 2) {
            d >> fm.mPreClear;
        }
    }
    return d;
}

INIT_REVS(5, 0)

void FileMerger::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(5, 0)
    Hmx::Object::Load(bs);
    if (d.rev < 2) {
        String str;
        d >> str;
    }
    d >> mMergers;
    StartLoadInternal(true, true);
}

void FileMerger::FinishLoading(Loader *ldr) {
    DirLoader *dl = dynamic_cast<DirLoader *>(ldr);
    Merger *merger = NotifyFileLoaded(ldr, dl);
    if (dl && !sDisableAll) {
        if (merger->mProxy) {
            MILO_ASSERT(dl->GetDir(), 0x236);
            ObjectDir *dir = Dir()->Find<ObjectDir>(dl->GetDir()->Name(), false);
            if (dir) {
                ReserveToFit(dl->GetDir(), dir, 0);
                MergeDirs(dl->GetDir(), dir, *this);
                dir->SyncObjects();
            } else {
                ObjectDir *dlDir = dl->GetDir();
                ReserveToFit(nullptr, Dir(), 2);
                dlDir->SetName(dlDir->Name(), Dir());
                merger->mLoadedObjects.push_back(dlDir);
            }
        } else {
            ObjectDir *mergerDir = merger->MergerDir();
            ReserveToFit(dl->GetDir(), mergerDir, 0);
            MergeDirs(dl->GetDir(), mergerDir, *this);
        }
    }
    PostMerge(merger, dl, true);
}

void FileMerger::FailedLoading(Loader *l) {
    MILO_ASSERT(l == mCurLoader, 0x204);
    MILO_ASSERT(l->LoaderFile() == mFilesPending.front()->loading, 0x205);
    static Message msg("on_load_failed", 0);
    msg[0] = mFilesPending.front()->mName;
    HandleType(msg);
    PostMerge(mFilesPending.front(), dynamic_cast<DirLoader *>(l), false);
}

MergeFilter::Action FileMerger::Filter(Hmx::Object *o1, Hmx::Object *o2, ObjectDir *dir) {
    Action a;
    if (mFilter) {
        a = mFilter->Filter(o1, o2, dir);
    } else {
        a = MergeAction(o1, o2, dir);
    }
    if (a == 1 && !o2) {
        mFilesPending.front()->mLoadedObjects.push_back(o1);
    }
    return a;
}

MergeFilter::SubdirAction FileMerger::FilterSubdir(ObjectDir *o1, ObjectDir *o2) {
    SubdirAction a;
    if (mFilter) {
        a = mFilter->FilterSubdir(o1, o2);
    } else {
        a = MergeFilter::DefaultSubdirAction(o1, mFilesPending.front()->mSubdirs);
    }
    if (a == 1 && !o2->HasSubDir(o1)) {
        mFilesPending.front()->mLoadedSubdirs.push_back(o1);
    }
    return a;
}

bool FileMerger::OriginalPath(Hmx::Object *obj, String &str) {
    Merger *merger = InMerger(obj);
    if (merger) {
        str = merger->mLoaded;
        return true;
    } else {
        return false;
    }
}

void FileMerger::Clear() {
    for (int i = 0; i < mMergers.size(); i++) {
        mMergers[i].Clear(false);
    }
    if (mCurLoader) {
        Merger *merger = mFilesPending.front();
        mFilesPending.clear();
        mFilesPending.push_front(merger);
        DeleteCurLoader();
    }
}

bool FileMerger::StartLoad(bool b) { return StartLoadInternal(b, false); }

FileMerger::Merger *FileMerger::FindMerger(Symbol name, bool warn) {
    int idx = FindMergerIndex(name, warn);
    if (idx != -1) {
        return &mMergers[idx];
    } else {
        return nullptr;
    }
}

void FileMerger::ClearSelections() {
    for (int i = 0; i < mMergers.size(); i++) {
        mMergers[i].mSelected.Set(FilePath::Root().c_str(), "");
    }
    if (mCurLoader) {
        Merger *front = mFilesPending.front();
        mFilesPending.clear();
        mFilesPending.push_front(front);
        DeleteCurLoader();
    }
}

int FileMerger::FindMergerIndex(Symbol name, bool warn) {
    for (int i = 0; i < mMergers.size(); i++) {
        if (mMergers[i].mName == name) {
            return i;
        }
    }
    if (warn) {
        MILO_NOTIFY("%s could not find Merger %s", PathName(this), name);
    }
    return -1;
}

FileMerger::Merger *FileMerger::InMerger(Hmx::Object *o) {
    for (int i = 0; i < mMergers.size(); i++) {
        Merger &cur = mMergers[i];
        if (cur.IsObjectLoaded(o)) {
            return &cur;
        }
    }
    return nullptr;
}

void FileMerger::DeleteCurLoader() {
    if (mCurLoader) {
        DirLoader *d = dynamic_cast<DirLoader *>(mCurLoader);
        // if (d)
        //     d->unk99 = true;
        delete mCurLoader;
    }
}

MergeFilter::Action
FileMerger::MergeAction(Hmx::Object *o1, Hmx::Object *o2, ObjectDir *dir) {
    if (!o2) {
        return (Action)1;
    }
    const char *name = o1->Name();
    DirLoader *dl = static_cast<DirLoader *>(mCurLoader);
    ObjectDir *dlDir = dl->GetDir();
    if (o1 == dlDir) {
        o2->MergeSinks(dlDir);
        return (Action)2;
    } else {
        if (strnicmp("spot_", name, 5) == 0 || strnicmp("bone_", name, 5) == 0
            || dynamic_cast<RndGroup *>(o2) || dynamic_cast<CharClipGroup *>(o2)
            || dynamic_cast<CharPollGroup *>(o2)) {
            return (Action)0;
        }
        if (!dynamic_cast<RndMat *>(o2)) {
            RndTex *tex = dynamic_cast<RndTex *>(o2);
            if (tex && !tex->File().empty()) {
                MILO_LOG(
                    "%s replacing texture %s with %s\n",
                    PathName(this),
                    PathName(o2),
                    PathName(o1)
                );
            } else {
                if (o2->Dir() != dir) {
                    MILO_NOTIFY(
                        "%s trying to replace subdir'd object %s with %s, bad because subdirs are shared",
                        PathName(this),
                        PathName(o2),
                        PathName(o1)
                    );
                    return (Action)2;
                }
            }
            return (Action)1;
        }
    }
    return (Action)2;
}

bool FileMerger::NeedsLoading(FileMerger::Merger &merger) {
    FOREACH (it, mFilesPending) {
        if (*it == &merger) {
            return merger.mSelected != merger.loading || merger.unk21;
        }
    }
    return merger.mLoaded != merger.mSelected || merger.unk21;
}

void FileMerger::LaunchNextLoader() {
    MILO_ASSERT(!mFilesPending.empty(), 0x182);
    MILO_ASSERT(!mCurLoader, 0x183);
    bool b1 = false;
    if (Dir()->Loader() && !Dir()->Loader()->IsLoaded()) {
        if (Dir()->Loader()->GetPos() != kLoadStayBack) {
            if (Dir()->Loader()->GetPos() != kLoadFrontStayBack)
                goto next;
        }
        b1 = true;
    }

next:
    int pos = 0;
    if (b1)
        pos = 2;
    FilePath &fp = mFilesPending.front()->loading;
    MemHeapTracker tmp(mHeap);
    if (fp.empty()) {
        mCurLoader = new NullLoader(fp, (LoaderPos)pos, mOrganizer);
    } else if (DirLoader::ShouldBlockSubdirLoad(fp)) {
        mCurLoader = new NullLoader(fp, (LoaderPos)pos, mOrganizer);
    } else {
        mCurLoader = new DirLoader(
            fp, (LoaderPos)pos, mOrganizer, nullptr, nullptr, false, nullptr
        );
    }
}

void FileMerger::Select(Symbol name, const FilePath &fp, bool b3) {
    Merger *merger = FindMerger(name, true);
    if (merger) {
        merger->SetSelected(fp, b3);
    }
}

bool FileMerger::StartLoadInternal(bool async, bool loading) {
    mAsyncLoad = async;
    mLoadingLoad = loading;
    static Message msg("change_files", 0, 0);
    msg[0] = async;
    msg[1] = loading;
    HandleType(msg);
    for (int i = 0; i < mMergers.size(); i++) {
        Merger &cur = mMergers[i];
        if (NeedsLoading(cur)) {
            AppendLoader(cur);
        }
    }
    Merger *tmp = nullptr;
    if (mCurLoader) {
        tmp = mFilesPending.front();
        mFilesPending.pop_front();
    }
    mFilesPending.sort(FileMergerSort());
    if (mCurLoader)
        mFilesPending.push_front(tmp);
    if (mFilesPending.empty() || mCurLoader || mOrganizer != this)
        return false;
    else {
        if (async) {
            // TheFileMergerOrganizer->AddFileMerger(this);
        } else {
            LaunchNextLoader();
            while (!mFilesPending.empty()) {
                TheLoadMgr.Poll();
            }
        }
        return true;
    }
}

FileMerger::Merger *FileMerger::NotifyFileLoaded(Loader *l, DirLoader *dl) {
    MILO_ASSERT_FMT(
        l->LoaderFile() == mFilesPending.front()->loading,
        "%s != %s",
        l->LoaderFile(),
        mFilesPending.front()->loading
    );
    MILO_ASSERT(l == mCurLoader, 0x217);
    Merger *m = mFilesPending.front();
    m->Clear(false);
    if (!sDisableAll) {
        static Message msg("on_pre_merge", 0, 0, 0);
        msg[0] = m->mName;
        ObjectDir *dir = dl ? dl->GetDir() : nullptr;
        msg[1] = dir;
        msg[2] = m->MergerDir();
        HandleType(msg);
        m->mLoaded = m->loading;
        m->loading.SetRoot("");
    }
    return m;
}

void FileMerger::AppendLoader(FileMerger::Merger &merger) {
    merger.unk21 = false;
    FOREACH (it, mFilesPending) {
        if (*it == &merger) {
            if (mCurLoader) {
                if (it == mFilesPending.begin()) {
                    DeleteCurLoader();
                    break;
                }
            }
            mFilesPending.erase(it);
            break;
        }
    }
    merger.loading = merger.mSelected;
    if (merger.mPreClear)
        merger.Clear(!mAsyncLoad);
    mFilesPending.push_back(&merger);
    if (TheLoadMgr.EditMode()) {
        static Message checkSync("check_sync", "", "");
        checkSync[0] = merger.loading;
        checkSync[1] = merger.mName;
        HandleType(checkSync);
    }
}

void FileMerger::PostMerge(FileMerger::Merger *merger, DirLoader *dl, bool b3) {
    mCurLoader = nullptr;
    mFilesPending.pop_front();
    mFilter = nullptr;
    if (b3) {
        {
            static Message msg("on_post_merge", 0, 0, 0);
            msg[0] = merger->mName;
            ObjectDir *dir = dl ? dl->GetDir() : nullptr;
            msg[1] = dir;
            msg[2] = merger->MergerDir();
            HandleType(msg);
        }
        if (dl) {
            if (!merger->mProxy) {
                ObjectDir *dir = dl->GetDir();
                if (dir) {
                    delete dir;
                }
            } else {
                delete dl;
            }
        }
        {
            static Message msg("on_post_delete", 0, 0, 0);
            msg[0] = merger->mName;
            msg[1] = merger->MergerDir();
            msg[2] = mFilesPending.empty();
            HandleType(msg);
        }
    }
    if (b3 || mOrganizer == this) {
        if (mFilesPending.empty()) {
            MILO_ASSERT(!mCurLoader, 0x290);
        } else if (!mCurLoader)
            LaunchNextLoader();
    }
    if (!b3 && dl && !dl->IsLoaded()) {
        dl->SetDeleteSelf(true);
    }
}

DataNode FileMerger::OnSelect(const DataArray *a) {
    FilePath fp(a->Str(3));
    Select(a->Sym(2), fp, false);
    return 0;
}

DataNode FileMerger::OnStartLoad(const DataArray *a) {
    StartLoadInternal(a->Size() == 3 ? a->Int(2) : true, false);
    return 0;
}
