#pragma once
#include "types.h"
#include "utl/Str.h"

class NetLoader {
public:
    virtual ~NetLoader();

    char *DetachBuffer();
    static NetLoader *Create(String const &);
    bool IsLoaded();

    String unk4;
    bool mIsLoaded; // 0xc
    char *unk10;
    u32 unk14;
    int unk18;
    int unk1c;

protected:
    NetLoader(String const &);
    void SetSize(int);
    void AttachBuffer(char *);
    void PostDownload();
};

class NetLoaderStub {
public:
    virtual ~NetLoaderStub();
    virtual void PollLoading();

    NetLoaderStub(String const &);
};

class DataNetLoader {
public:
    DataNetLoader(String const &);
    ~DataNetLoader();
    bool IsLoaded();
    bool HasFailed();
    void PollLoading();

    NetLoader *unk0;
    int unk4;
};
