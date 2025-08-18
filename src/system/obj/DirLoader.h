#pragma once
#include "obj/Object.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/MemPoint.h"
#include "utl/PoolAlloc.h"
#include "utl/TextFileStream.h"

class DirLoader : public Loader, public ObjRefOwner {
    typedef void (DirLoader::*DirLoaderStateFunc)(void);

public:
    struct ClassAndNameSort {
        bool operator()(Hmx::Object *, Hmx::Object *);

    protected:
        int ClassIndex(Hmx::Object *);
    };

    DirLoader(const FilePath &, LoaderPos, Loader::Callback *, BinStream *, class ObjectDir *, bool, class ObjectDir *);
    virtual ~DirLoader();
    virtual Hmx::Object *RefOwner() const { return nullptr; }
    virtual bool Replace(ObjRef *, Hmx::Object *);
    virtual const char *DebugText();
    virtual bool IsLoaded() const;
    virtual const char *StateName() const;

    ObjectDir *GetDir();
    void Cleanup(const char *);

    const char *ProxyName() const { return mProxyName; }
    ObjectDir *ProxyDir() const { return mProxyDir; }

    POOL_OVERLOAD(DirLoader, 0x2A);

    static bool sPrintTimes;
    static void SetCacheMode(bool);

    static Symbol GetDirClass(const char *);
    static const char *CachedPath(const char *, bool);
    static bool ShouldBlockSubdirLoad(const FilePath &);
    static bool SaveObjects(const char *, ObjectDir *, bool);
    static void SaveObjects(BinStream &, ObjectDir *);
    static void WriteTypeMemDump(TextFileStream *);
    static Loader *New(const FilePath &, LoaderPos);
    static DirLoader *Find(const FilePath &);
    static DirLoader *FindLast(const FilePath &);
    static ObjectDir *LoadObjects(const FilePath &, Callback *, BinStream *);

private:
    virtual void PollLoading() { (this->*mState)(); }

    Symbol FixClassName(Symbol);
    bool SetupDir(Symbol);
    void DumpObjectMemDelta(const Hmx::Object *, const MemPointDelta &) const;
    void AddTypeObjectMemDelta(const Hmx::Object *, const MemPointDelta &) const;

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
    int unk64;
    Timer mTimer; // 0x68
    bool unk98;
    bool unk99;
    bool unk9a;
    bool unk9b;
    class ObjectDir *unk9c;
    ObjOwnerPtr<ObjectDir> mProxyDir; // 0xa0

    static bool sCacheMode;
    static bool (*sPathEval)(const char *);
    static ObjectDir *sTopSaveDir;
    static TextFileStream *sObjectMemDumpFile;
    static TextFileStream *sTypeMemDumpFile;
    static std::map<String, MemPointDelta> sMemPointMap;
};
