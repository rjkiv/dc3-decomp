#pragma once
#include "os/NetworkSocket.h"
#include "utl/Str.h"
#include <list>

class OSCMessenger {
public:
    struct OSCValue {
        union Type {
            char s[128];
            int i;
            float f;
            float v[3];
        };

        String mAddress; // 0x0
        Type mValue; // 0x8
        char mType; // 0x88
        bool mFresh; // 0x89
    };
    virtual ~OSCMessenger();

    void Connect();
    void Poll();
    int GetInt(String address, int defaultValue);
    float GetFloat(String address, float defaultValue);
    void SendOSCFloat(String, float);

private:
    OSCValue *GetValue(String address);
    int MakeOSCAddress(String address, char *out);

    NetworkSocket *mSocketIn; // 0x4
    NetworkSocket *mSocketOut; // 0x8
    std::list<OSCValue> mValues; // 0xc
};

extern OSCMessenger TheOSCMessenger;
