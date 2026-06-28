#pragma once
#include "os/NetworkSocket.h"
#include "utl/Str.h"
#include <list>

class OSCMessenger {
public:
    struct OSCValue {
        String unk0; // 0x0
        char buffer[0x80]; // 0x8
        char mType; // 0x88
        bool unk89; // 0x89
    };
    virtual ~OSCMessenger();

    void Connect();
    void Poll();
    int GetInt(String, int);
    float GetFloat(String, float);
    void SendOSCFloat(String, float);

private:
    OSCValue *GetValue(String);
    int MakeOSCAddress(String, char *);

    NetworkSocket *mSocket1; // 0x4
    NetworkSocket *mSocket2; // 0x8
    std::list<OSCValue> mValues; // 0xc
};

extern OSCMessenger TheOSCMessenger;
