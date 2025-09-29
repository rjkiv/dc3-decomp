#pragma once
#include "obj/Object.h"
#include "obj/Utl.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"

class OriginalPathable {
public:
    virtual ~OriginalPathable() {}
    virtual bool OriginalPath(Hmx::Object *, String &) = 0;
};

class FileMerger : public Hmx::Object,
                   public Loader::Callback,
                   public MergeFilter,
                   public OriginalPathable {
public:
    struct Merger {
        Merger(Hmx::Object *);
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
    // Loader::Callback
    virtual void FinishLoading(Loader *);
    virtual void FailedLoading(Loader *);
    // MergeFilter
    virtual Action Filter(Hmx::Object *, Hmx::Object *, ObjectDir *);
    virtual SubdirAction FilterSubdir(ObjectDir *o1, ObjectDir *);
    // OriginalPathable
    virtual bool OriginalPath(Hmx::Object *, String &);

    bool StartLoad(bool);
    void Clear();
    void Select(Symbol, const FilePath &, bool);
    bool AsyncLoad() const { return mAsyncLoad; }
    bool HasPendingFiles() const { return !mFilesPending.empty(); }

protected:
    FileMerger();

    ObjVector<Merger> mMergers; // 0x40
    bool mAsyncLoad; // 0x50
    bool mLoadingLoad; // 0x51
    Loader *mCurLoader; // 0x54
    std::list<Merger *> mFilesPending; // 0x58
    MergeFilter *mFilter; // 0x60
    int mHeap; // 0x64
    Loader::Callback *mOrganizer; // 0x68
};
