#pragma once
#include "os/ThreadCall.h"
#include "os/Timer.h"
#include "utl/Str.h"
#include "xdk/XNET.h"

class XLSPConnection : public ThreadCallback {
public:
    enum State {
    };
    XLSPConnection();
    virtual ~XLSPConnection();
    virtual int ThreadStart();
    virtual void ThreadDone(int);

    State GetState() { return unk4; }
    int GetUnk8() const { return unk8; }
    void Poll();
    unsigned int GetServiceIP();
    void Connect(const char *, unsigned int);
    void Disconnect();

    static std::map<unsigned long, int> mXLSPRefCountMap;
    static bool SecureDisconnect(IN_ADDR);
    static int StartGatewayConnection(IN_ADDR);

private:
    void SetState(State);
    void StartEnumeration();

    static const int kTitleServerEnumMaxCount;

    State unk4;
    int unk8;
    String unkc;
    unsigned int unk14;
    HANDLE unk18;
    void *unk1c;
    DWORD unk20;
    int unk24;
    XOVERLAPPED mXOverlapped; // 0x28
    int unk44;
    Timer unk48;
};
