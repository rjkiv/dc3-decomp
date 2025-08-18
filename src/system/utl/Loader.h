#pragma once
#include "os/Platform.h"
#include "os/Timer.h"
#include "utl/FilePath.h"
#include "utl/MemMgr.h"
#include <list>

enum LoaderPos {
    kLoadFront = 0,
    kLoadBack = 1,
    kLoadFrontStayBack = 2,
    kLoadStayBack = 3,
};

class Loader {
public:
    class Callback {
    public:
        Callback() {}
        virtual ~Callback() {}
        virtual void FinishLoading(Loader *) {}
        virtual void FailedLoading(Loader *) {}
    };
    Loader(const FilePath &, LoaderPos);
    virtual ~Loader();
    virtual const char *DebugText() = 0;
    virtual bool IsLoaded() const = 0;
    virtual const char *StateName() const { return "Unknown"; }

    LoaderPos GetPos() const { return mPos; }
    FilePath &LoaderFile() { return mFile; }

    MEM_OVERLOAD(Loader, 0xA8);

protected:
    virtual void PollLoading() = 0;

    int unk4; // 0x4
    LoaderPos mPos; // 0x8
    FilePath mFile; // 0xc
    int unk14; // 0x14
    int mHeap; // 0x18
};

typedef Loader *LoaderFactoryFunc(const FilePath &, LoaderPos);

class LoadMgr {
private:
    std::list<Loader *> mLoaders; // 0x0
    unsigned int mPlatform; // 0x8
    bool mEditMode; // 0xC
    bool mCacheMode; // 0xD
    std::list<std::pair<class String, LoaderFactoryFunc *> > mFactories; // 0x10
    float mPeriod; // 0x18
    float unk1c;
    std::list<Loader *> mLoading; // 0x20
    Timer mTimer; // 0x28
    int mAsyncUnload; // 0x58
    LoaderPos mLoaderPos; // 0x5c
public:
    LoadMgr();
    ~LoadMgr();

    void StartAsyncUnload(); // { mAsyncUnload++; }
    void FinishAsyncUnload(); // { mAsyncUnload--; }
    bool EditMode() const { return mEditMode; }
    Platform GetPlatform() const { return (Platform)mPlatform; }
    int AsyncUnload() const; // { return mAsyncUnload; }
    const std::list<Loader *> &Loaders() const { return mLoaders; }
    LoaderPos GetLoaderPos() const { return mLoaderPos; }

    void SetEditMode(bool);
    void RegisterFactory(const char *, LoaderFactoryFunc *);
    void PollUntilLoaded(Loader *, Loader *);
    Loader *GetLoader(const FilePath &) const;
    Loader *ForceGetLoader(const FilePath &);
    Loader *AddLoader(const FilePath &, LoaderPos);
    void Poll();
};

extern LoadMgr TheLoadMgr;

class FileLoader;
typedef void (FileLoader::*FileLoaderStateFunc)(void);

class FileLoader : public Loader {
public:
    FileLoader(const FilePath &, const char *, LoaderPos, int, bool, bool, BinStream *);
    virtual ~FileLoader();
    virtual const char *DebugText();
    virtual bool IsLoaded() const;
    virtual void PollLoading();

    char *GetBuffer(int *);
    int GetSize();

    void AllocBuffer();
    void OpenFile();
    void LoadFile();
    void DoneLoading();
    void LoadStream();

    File *mFile; // 0x18
    BinStream *mStream; // 0x1c
    const char *mBuffer; // 0x20
    int mBufLen; // 0x24
    bool mAccessed; // 0x28
    bool mTemp; // 0x29
    bool mWarn; // 0x2a
    int mFlags; // 0x2c
    class String mFilename; // 0x30

    int unk3c; // 0x3c
    int unk40; // 0x40
    FileLoaderStateFunc mState; // 0x44
};
