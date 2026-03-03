#pragma once
#include "obj/Data.h"
#include "types.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"

enum NetLoaderFailType {
    // file not found = 1
};

class NetLoader {
public:
    virtual ~NetLoader();
    virtual void PollLoading() = 0;
    virtual bool HasFailed() = 0;
    virtual bool IsSafeToDelete() const = 0;

    char *DetachBuffer();
    int GetSize();
    bool IsLoaded();
    char *GetBuffer();
    const char *GetRemotePath() const;
    NetLoaderFailType GetFailType() const { return mFailType; }
    MEM_OVERLOAD(NetLoader, 0x18);
    static NetLoader *Create(const String &);

protected:
    NetLoader(const String &);
    void SetSize(int);
    void AttachBuffer(char *);
    void PostDownload();
    void SetFailType(NetLoaderFailType);

    String mStrRemotePath; // 0x4
    bool mIsLoaded; // 0xc
    char *mBuffer; // 0x10
    int unk14; // 0x14
    int mSize; // 0x18
    NetLoaderFailType mFailType; // 0x1c
};

class NetLoaderStub : public NetLoader {
public:
    NetLoaderStub(const String &);
    virtual ~NetLoaderStub();
    virtual void PollLoading();
    virtual bool HasFailed() { return false; };
    virtual bool IsSafeToDelete() const { return true; }

private:
    static const float kNetSimInitialDelay;
    static const float kNetSimKbPerSecond;

    FileLoader *mFileLoader; // 0x20
    float mNetSimEndTime; // 0x24
};

class DataNetLoader {
public:
    DataNetLoader(const String &);
    ~DataNetLoader();
    bool IsLoaded();
    bool HasFailed();
    void PollLoading();

    DataArray *GetUnk4() const { return unk4; }

private:
    NetLoader *unk0; // 0x0
    DataArray *unk4; // 0x4
};
