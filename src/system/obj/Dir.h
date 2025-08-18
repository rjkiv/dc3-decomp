#pragma once
#include "math/Mtx.h"
#include "obj/DirLoader.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "utl/KeylessHash.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "utl/StringTable.h"
#include <vector>

enum InlineDirType {
    kInlineNever = 0,
    kInlineCached = 1,
    kInlineAlways = 2,
    kInlineCachedShared = 3
};

template <class C>
class ObjDirPtr : public ObjRefConcrete<C> {
public:
    ObjDirPtr() : ObjRefConcrete(nullptr), mLoader(nullptr) {}
    ObjDirPtr(C *);
    ObjDirPtr(const ObjDirPtr &o) : ObjRefConcrete<C>(o.mObject), mLoader(nullptr) {}
    virtual ~ObjDirPtr() { *this = nullptr; }
    virtual bool IsDirPtr() { return true; }
    virtual void Replace(Hmx::Object *o) {
        MILO_ASSERT(ObjRefConcrete<C>::mObject, 0x70);
        *this = o ? dynamic_cast<C *>(o) : nullptr;
    }

    DirLoader *mLoader; // 0x10

    bool IsLoaded() const;
    ObjDirPtr &operator=(C *);
    operator C *() const { return mObject; }
    C *operator->() const {
        MILO_ASSERT(ObjRefConcrete<C>::mObject, 0x5F);
        return mObject;
    }
    void LoadFile(const FilePath &, bool, bool, LoaderPos, bool);

    void LoadInlinedFile(const FilePath &fp, BinStream &bs) {
        *this = nullptr;
        // there's more
        mLoader = new DirLoader(
            fp,
            TheLoadMgr.GetLoaderPos() == kLoadStayBack
                    || TheLoadMgr.GetLoaderPos() == kLoadFrontStayBack
                ? kLoadFrontStayBack
                : kLoadFront,
            nullptr,
            &bs,
            nullptr,
            false,
            nullptr
        );
    }
};

template <class C>
BinStream &operator<<(BinStream &bs, const ObjDirPtr<C> &ptr) {
    return bs;
}

class ObjectDir : public virtual Hmx::Object {
    friend class Hmx::Object;

public:
    enum ViewportId {
        kNumViewports = 7
    };

    class Viewport {
    public:
        Transform mXfm;
    };

    struct Entry {
        Entry() : name(0), obj(0) {}
        Entry &operator=(const Entry &entry) {
            name = entry.name;
            obj = entry.obj;
            return *this;
        }
        bool operator==(const Entry &e) const { return name == e.name; }
        bool operator!=(const Entry &e) const { return name != e.name; }
        operator const char *() const { return name; }

        const char *name;
        Hmx::Object *obj;
    };

protected:
    struct InlinedDir {
        InlinedDir();
        ~InlinedDir();
        ObjDirPtr<ObjectDir> dir; // 0x0
        FilePath file; // 0x14
        bool shared; // 0x1c
        InlineDirType inlineDirType; // 0x20
    };

    KeylessHash<const char *, Entry> mHashTable; // 0x8
    StringTable mStringTable; // 0x28
    FilePath mProxyFile; // 0x3c
    bool mProxyOverride; // 0x44
    InlineDirType mInlineProxyType; // 0x48
    DirLoader *mLoader; // 0x4c
    std::vector<ObjDirPtr<ObjectDir> > mSubDirs; // 0x50
    bool mIsSubDir; // 0x5c
    InlineDirType mInlineSubDirType; // 0x60
    const char *mPathName; // 0x64
    FilePath mStoredFile; // 0x68
    std::vector<InlinedDir> mInlinedDirs; // 0x70
    std::vector<Viewport> mViewports; // 0x7c
    ViewportId mCurViewportID; // 0x88
    Hmx::Object *unk8c; // 0x8c
    Hmx::Object *mCurCam; // 0x90
    int mAlwaysInlined; // 0x94 / -0xC
    const char *mAlwaysInlineHash; // 0x98

    ObjectDir();
    static ObjectDir *sMainDir;

public:
    // Hmx::Object
    virtual ~ObjectDir();
    OBJ_CLASSNAME(ObjectDir);
    OBJ_SET_TYPE(ObjectDir);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void Load(BinStream &);
    virtual void PostSave(BinStream &);
    virtual void SetName(const char *name, ObjectDir *dir) {
        Hmx::Object::SetName(name, dir);
    }
    virtual ObjectDir *DataDir();
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // ObjectDir
    virtual void SetProxyFile(const FilePath &, bool);
    virtual const FilePath &ProxyFile();
    virtual void SetSubDir(bool);
    virtual DataArrayPtr GetExposedProperties();
    virtual void SyncObjects();
    virtual void ResetEditorState();
    virtual InlineDirType InlineSubDirType();

    template <class T>
    T *Find(const char *name, bool parentDirs) {
        T *castedObj = dynamic_cast<T *>(FindObject(name, false, true));
        if (!castedObj && parentDirs) {
            MILO_FAIL(
                kNotObjectMsg, name, PathName(this) ? PathName(this) : "**no file**"
            );
        }
        return castedObj;
    }

    void SetCurViewport(ViewportId id, Hmx::Object *o) {
        mCurViewportID = id;
        mCurCam = o;
    }
    void SetSubDirFlag(bool flag) { mIsSubDir = flag; }
    void SetLoader(DirLoader *dl) { mLoader = dl; }
    DirLoader *Loader() const { return mLoader; }
    bool IsProxy() const { return this != Dir(); }
    int HashTableSize() const { return mHashTable.Size(); }
    int StrTableSize() const { return mStringTable.Size(); }
    int HashTableUsedSize() const { return mHashTable.UsedSize(); }
    int StrTableUsedSize() const { return mStringTable.UsedSize(); }
    KeylessHash<const char *, Entry> &HashTable() { return mHashTable; }
    const char *GetPathName() const { return mPathName; }
    const std::vector<ObjDirPtr<ObjectDir> > &SubDirs() const { return mSubDirs; }
    InlineDirType InlineProxyType() const { return mInlineProxyType; }

    void ResetViewports();
    void SetInlineProxyType(InlineDirType);
    void Reserve(int, int);
    Hmx::Object *FindObject(const char *, bool, bool);
    bool InlineProxy(BinStream &);
    bool HasDirPtrs() const;
    void TransferLoaderState(ObjectDir *);
    Viewport &CurViewport();
    bool HasSubDir(ObjectDir *);
    void SaveProxy(BinStream &);
    FilePath GetSubDirPath(const FilePath &, const BinStream &);
    void DeleteObjects();
    void RemoveSubDir(const ObjDirPtr<ObjectDir> &);
    void DeleteSubDirs();
    ObjectDir *FindContainingDir(const char *);
    void AppendSubDir(const ObjDirPtr<ObjectDir> &);

    static ObjectDir *Main() { return sMainDir; }
    static void PreInit(int, int);
    static void Init();
    static void Terminate();
    // key: Symbol child, Symbol parent
    // value: bool for whether child is a subclass of parent
    static std::map<std::pair<Symbol, Symbol>, bool> sSuperClassMap;

    NEW_OBJ(ObjectDir);
    OBJ_MEM_OVERLOAD(0x111);

protected:
    virtual void AddedObject(Hmx::Object *) {}
    virtual void RemovingObject(Hmx::Object *);
    virtual void OldLoadProxies(BinStream &, int);

    bool SaveSubdirs();
    bool ShouldSaveProxy(BinStream &);
    Entry *FindEntry(const char *, bool);
    void SaveInlined(const FilePath &, bool, InlineDirType);
    void PreLoadInlined(const FilePath &, bool, InlineDirType);
    void LoadSubDir(int, const FilePath &, BinStream &, bool);
    void AddedSubDir(ObjDirPtr<ObjectDir> &);
    void RemovingSubDir(ObjDirPtr<ObjectDir> &);
    void Iterate(DataArray *, bool);
    ObjDirPtr<ObjectDir> PostLoadInlined();

    DataNode OnFind(DataArray *);
};

extern const char *kNotObjectMsg;

/** Iterates through each Object in an ObjectDir that is of type T. */
template <class T>
class ObjDirItr {
private:
    // needs work
    void Advance() {
        while (true) {
            if (mEntry) {
                mObj = dynamic_cast<T *>(mEntry->obj);
                if (mObj)
                    return;
                mEntry = mSubDirs.front()->HashTable().Next(mEntry);
            } else {
                if (mSubDirs.empty()) {
                    mObj = nullptr;
                    return;
                }
                ObjectDir *dir = mSubDirs.back();
                mSubDirs.pop_back();
                if (dir) {
                    mEntry = dir->HashTable().Next(mEntry);
                }
            }
        }
    }
    void RecurseSubdirs(ObjectDir *);

    ObjectDir::Entry *mEntry; // 0x0
    T *mObj; // 0x4
    std::list<ObjectDir *> mSubDirs; // 0x8

public:
    ObjDirItr(ObjectDir *dir, bool recurse) {
        if (dir) {
            if (recurse) {
                RecurseSubdirs(dir);
            } else {
                mSubDirs.push_back(dir);
            }
            mEntry = mSubDirs.front()->HashTable().Begin();
            Advance();
        } else {
            mObj = nullptr;
            mEntry = nullptr;
        }
    }
    ObjDirItr &operator++() {
        if (mEntry) {
            mEntry = mSubDirs.front()->HashTable().Next(mEntry);
            Advance();
        }
        return *this;
    }

    operator T *() { return mObj; }
    T *operator->() { return mObj; }
};
