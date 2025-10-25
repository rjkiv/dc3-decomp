#pragma once

#include "stl/_map.h"
#include "utl/Str.h"
enum ReqType {
};

class HttpReq {
public:
    virtual ~HttpReq(); // 0x0
    virtual void SetContentLength(unsigned int); // 0x8
    virtual void SetCookies(std::map<String, String> const &); // 0xc
    virtual void Start() = 0; // 0x10
    virtual void Do() = 0; // 0x18
    virtual void Reset(); // 0x1c
    virtual void *GetRequest() = 0; // 0x20
    virtual void SetTimeout(unsigned int) = 0; // 0x30
    virtual void SetUserAgent(char const *); // 0x38
    virtual bool HasFailed(); // 0x4c
    virtual bool HasSucceeded(); // 0x50, last vfunc

    // unsure about the placement of these two
    virtual void SetContent(char const *);
    virtual void SetType(ReqType);

    HttpReq(ReqType, unsigned int, unsigned short, char const *);
    HttpReq(ReqType, char const *, unsigned short, char const *);
    unsigned int GetStatusCode(void);

    String mHostName; // 0x4
    unsigned int mIPAddr; // 0xc
    unsigned int unk10;
    String mURL; // 0x14
    ReqType unk1c;
    String unk20;
    char const *unk28;
    int unk2c;
    unsigned int unk30;
    std::map<String, String> unk34;
    int unk4c;
};
