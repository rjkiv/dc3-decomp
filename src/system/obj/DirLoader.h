#pragma once
#include "obj/Dir.h"
#include "obj/Object.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/MemPoint.h"
#include "utl/PoolAlloc.h"
#include "utl/TextFileStream.h"

typedef bool PathEvalFunc(const char *);

class DirLoader : public Loader, public ObjRefOwner {
    typedef void (DirLoader::*DirLoaderStateFunc)(void);

public:
    struct ClassAndNameSort {
        ClassAndNameSort() {}
        bool operator()(Hmx::Object *, Hmx::Object *);

    protected:
        int ClassIndex(Hmx::Object *);
    };

    DirLoader(
        const FilePath &file,
        LoaderPos pos,
        Loader::Callback *callback,
        BinStream *stream,
        class ObjectDir *proxy,
        bool subdir,
        class ObjectDir *editorTargetDir
    );
    virtual ~DirLoader();
    virtual Hmx::Object *RefOwner() const { return nullptr; }
    virtual bool Replace(ObjRef *, Hmx::Object *);
    virtual const char *DebugText();
    virtual bool IsLoaded() const;
    virtual const char *StateName() const;

    ObjectDir *GetDir();
    void Cleanup(const char *msg);

    // public: ObjectDir* GetDirWithoutTakingOwnership() const;
    Callback *GetCallback() { return mCallback; }
    const char *ProxyName() const { return mProxyName; }
    ObjectDir *ProxyDir() { return mProxyDir; }
    void SetProxyDir(ObjectDir *dir) { mProxyDir = dir; }
    void SetDeleteSelf() { mDeleteSelf = true; }
    void SetExternalFailed() { mExternalFailed = true; }

    POOL_OVERLOAD(DirLoader, 0x2A);

    static bool sPrintTimes;
    static void SetCacheMode(bool);

    static Symbol GetDirClass(const char *file);
    static const char *CachedPath(const char *file, bool force);
    static bool ShouldBlockSubdirLoad(const FilePath &);
    static bool SaveObjects(const char *file, ObjectDir *dir, bool saveEditorDir);
    static void SaveObjects(BinStream &d, ObjectDir *dir);
    static void WriteTypeMemDump(TextFileStream *file);
    static Loader *New(const FilePath &file, LoaderPos pos) {
        return new DirLoader(file, pos, nullptr, nullptr, nullptr, false, nullptr);
    }
    static DirLoader *Find(const FilePath &file);
    static DirLoader *FindLast(const FilePath &file);
    static ObjectDir *
    LoadObjects(const FilePath &file, Callback *callback, BinStream *stream);
    static void SetPathEvalFunc(PathEvalFunc *func) { sPathEval = func; }
    static ObjectDir *TopSaveDir() { return sTopSaveDir; }

private:
    virtual void PollLoading() { (this->*mState)(); }

    Symbol FixClassName(Symbol c);
    bool SetupDir(Symbol className);
    void
    DumpObjectMemDelta(const Hmx::Object *object, const MemPointDelta &memDelta) const;
    void
    AddTypeObjectMemDelta(const Hmx::Object *object, const MemPointDelta &memDelta) const;

    void LoadObjs();
    void LoadDir();
    void LoadResources();
    void CreateObjects();
    void LoadHeader();
    void OpenFile();
    void DoneLoading() {}

    DirLoaderStateFunc mState; // 0x20
    class String mRoot; // 0x28
    bool mOwnStream; // 0x30
    BinStream *mStream; // 0x34
    int mRev; // 0x38
    int mCounter; // 0x3c
    ObjPtrList<Hmx::Object> mObjects; // 0x40
    Callback *mCallback; // 0x54
    class ObjectDir *mDir; // 0x58
    bool mPostLoad; // 0x5c
    bool mLoadDir; // 0x5d
    bool mDeleteSelf; // 0x5e
    const char *mProxyName; // 0x60
    Timer mTimer; // 0x68
    bool mAccessed; // 0x98
    bool mExternalFailed; // 0x99
    bool mHasEditorDir; // 0x9a
    bool mSubDir; // 0x9b
    class ObjectDir *mEditorTargetDir; // 0x9c
    ObjOwnerPtr<ObjectDir> mProxyDir; // 0xa0

    static bool sCacheMode;
    static PathEvalFunc *sPathEval;
    static ObjectDir *sTopSaveDir;
    static TextFileStream *sObjectMemDumpFile;
    static TextFileStream *sTypeMemDumpFile;
    static std::map<String, MemPointDelta> sMemPointMap;
};
