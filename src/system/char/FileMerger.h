#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Utl.h"
#include "stl/_vector.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"

class OriginalPathable {
public:
    virtual ~OriginalPathable() {}
    virtual bool OriginalPath(Hmx::Object *, String &) = 0;
};

/** "Merges files into ObjectDirs, much like a milo file merge." */
class FileMerger : public Hmx::Object,
                   public Loader::Callback,
                   public MergeFilter,
                   public OriginalPathable {
    friend class FileMergerOrganizer; // tbh i'd rather do this than make everything
                                      // public

public:
    struct Merger { // taken from RB3 decomp
        struct SortBySelected {
            bool operator()(const Merger &a, const Merger &b) {
                return stricmp(a.mSelected.c_str(), b.mSelected.c_str()) < 0;
            }
        };

        Merger(Hmx::Object *o)
            : mProxy(0), mPreClear(0),
              mSubdirs(MergeFilter::kMergeInlinedMoveSharedSubdirs), mDir(o),
              mLoadedObjects(o), mLoadedSubdirs(o) {}
        Merger(const Merger &m)
            : mDir(m.mDir.Owner()), mLoadedObjects(m.mLoadedObjects.Owner()),
              mLoadedSubdirs(m.mLoadedSubdirs.Owner()) {
            *this = m;
        }
        ~Merger() {}
        Merger &operator=(const Merger &m) {
            mName = m.mName;
            filler = m.filler;
            mSelected = m.mSelected;
            loading = m.loading;
            mLoaded = m.mLoaded;
            mDir = m.mDir;
            mProxy = m.mProxy;
            mSubdirs = m.mSubdirs;
            mLoadedObjects = m.mLoadedObjects;
            mLoadedSubdirs = m.mLoadedSubdirs;
            mPreClear = m.mPreClear;
            unk21 = m.unk21;
            return *this;
        }

        void Clear(bool);

        ObjectDir *MergerDir() {
            if (mDir)
                return mDir;
            else
                return mDir.Owner()->Dir();
        }

        // gross and convoluted way to basically check if this object is in mLoadedObjects
        bool IsObjectLoaded(Hmx::Object *obj) {
            return !mLoadedObjects.find(obj) == false;
        }

        void SetSelected(const FilePath &fp, bool b) {
            mSelected = fp;
            unk21 = b;
        }

        bool IsProxy() const { return mProxy; }

        /** "Name of the merger, just for identification" */
        Symbol mName; // 0x0
        Symbol filler; // 0x4
        /** "The file you want to merge" */
        FilePath mSelected; // 0x8
        FilePath loading; // 0x10
        /** "currently loaded file" */
        FilePath mLoaded; // 0x18
        /** "If true, merges the Dir in as a proxy, rather than the individual objects" */
        bool mProxy; // 0x20
        bool unk21; // 0x21
        /** "Delete the old objects right at StartLoad time" */
        bool mPreClear; // 0x22
        /** "How to treat subdirs in the source" */
        MergeFilter::Subdirs mSubdirs; // 0x24
        /** "Dir to merge into, proxy, for instance" */
        ObjPtr<ObjectDir> mDir; // 0x28
        /** "loaded objects that will be deleted when file changes" */
        ObjPtrList<Hmx::Object> mLoadedObjects; // 0x3c
        /** "moved subdirs that will be removed when file changes" */
        ObjPtrList<ObjectDir> mLoadedSubdirs; // 0x50
    };
    // Hmx::Object
    virtual ~FileMerger();
    OBJ_CLASSNAME(FileMerger);
    OBJ_SET_TYPE(FileMerger);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreSave(BinStream &);
    virtual void PostSave(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &) {}
    // OriginalPathable
    virtual bool OriginalPath(Hmx::Object *, String &);

    OBJ_MEM_OVERLOAD(0x17)
    NEW_OBJ(FileMerger);

    static FileMerger *sFmDeleting;

    Merger *FindMerger(Symbol, bool);
    int FindMergerIndex(Symbol, bool);
    bool StartLoad(bool);
    void Clear();
    void ClearSelections();
    Action MergeAction(Hmx::Object *, Hmx::Object *, ObjectDir *);
    void Select(Symbol, const FilePath &, bool);
    Merger *InMerger(Hmx::Object *);
    bool AsyncLoad() const { return mAsyncLoad; }
    bool HasPendingFiles() const { return !mFilesPending.empty(); }
    void ClearMergers() { mMergers.clear(); }

protected:
    FileMerger();

    // Loader::Callback
    virtual void FinishLoading(Loader *);
    virtual void FailedLoading(Loader *);
    // MergeFilter
    virtual Action Filter(Hmx::Object *, Hmx::Object *, ObjectDir *);
    virtual SubdirAction FilterSubdir(ObjectDir *o1, ObjectDir *);

    void DeleteCurLoader();
    bool StartLoadInternal(bool, bool);
    Merger *NotifyFileLoaded(Loader *, DirLoader *);
    void PostMerge(Merger *, DirLoader *, bool);
    bool NeedsLoading(Merger &);
    void LaunchNextLoader();
    void AppendLoader(Merger &);

    DataNode OnSelect(const DataArray *);
    DataNode OnStartLoad(const DataArray *);

    static bool sDisableAll;

    /** "Array of file mergers" */
    ObjVector<Merger> mMergers; // 0x40
    bool mAsyncLoad; // 0x50
    bool mLoadingLoad; // 0x51
    Loader *mCurLoader; // 0x54
    std::list<Merger *> mFilesPending; // 0x58
    MergeFilter *mFilter; // 0x60
    int mHeap; // 0x64
    Loader::Callback *mOrganizer; // 0x68
};

struct FileMergerSort {
    bool operator()(const FileMerger::Merger *, const FileMerger::Merger *);
};
