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
    struct Merger {
        struct SortBySelected {
            bool operator()(const Merger &a, const Merger &b) {
                return stricmp(a.selected.c_str(), b.selected.c_str()) < 0;
            }
        };

        Merger(Hmx::Object *o)
            : proxy(0), preclear(0), subdirs(MergeFilter::kMergeInlinedMoveSharedSubdirs),
              dir(o), loadedObjects(o), loadedSubdirs(o) {}
        Merger(const Merger &m)
            : dir(m.dir.Owner()), loadedObjects(m.loadedObjects.Owner()),
              loadedSubdirs(m.loadedSubdirs.Owner()) {
            *this = m;
        }
        ~Merger() {}
        Merger &operator=(const Merger &m) {
            name = m.name;
            uniqueName = m.uniqueName;
            selected = m.selected;
            loading = m.loading;
            loaded = m.loaded;
            dir = m.dir;
            proxy = m.proxy;
            subdirs = m.subdirs;
            loadedObjects = m.loadedObjects;
            loadedSubdirs = m.loadedSubdirs;
            preclear = m.preclear;
            force = m.force;
            return *this;
        }

        void Clear(bool);

        ObjectDir *ToDir() {
            if (dir)
                return dir;
            else
                return dir.Owner()->Dir();
        }

        // gross and convoluted way to basically check if this object is in loadedObjects
        bool Has(Hmx::Object *obj) const { return !loadedObjects.find(obj).Null(); }

        void Select(const FilePath &fp, bool b) {
            selected = fp;
            force = b;
        }

        bool Proxy() const { return proxy; }

        /** "Name of the merger, just for identification" */
        Symbol name; // 0x0
        Symbol uniqueName; // 0x4
        /** "The file you want to merge" */
        FilePath selected; // 0x8
        FilePath loading; // 0x10
        /** "currently loaded file" */
        FilePath loaded; // 0x18
        /** "If true, merges the Dir in as a proxy, rather than the individual objects" */
        bool proxy; // 0x20
        bool force; // 0x21
        /** "Delete the old objects right at StartLoad time" */
        bool preclear; // 0x22
        /** "How to treat subdirs in the source" */
        MergeFilter::Subdirs subdirs; // 0x24
        /** "Dir to merge into, proxy, for instance" */
        ObjPtr<ObjectDir> dir; // 0x28
        /** "loaded objects that will be deleted when file changes" */
        ObjPtrList<Hmx::Object> loadedObjects; // 0x3c
        /** "moved subdirs that will be removed when file changes" */
        ObjPtrList<ObjectDir> loadedSubdirs; // 0x50
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
    std::vector<Merger> &Mergers() { return mMergers; }

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
