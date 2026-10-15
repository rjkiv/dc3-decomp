#pragma once
#include "os/ThreadCall.h"
#include "os/Timer.h"
#include "utl/Str.h"
#include "xdk/XNET.h"
#include "xdk/xnet/winsockx.h"
#include "xdk/xonline/xonline.h"

class XLSPConnection : public ThreadCallback {
public:
    enum State {
        kS_Disconnected = 0x0000,
        kS_Enumerating = 0x0001,
        kS_ConnectingToGateway = 0x0002,
        kS_Connected = 0x0003,
        kS_Failure = 0x0004,
        kS_RequestDisconnect = 0x0005,
        kS_Max = 0x0006,
        kS_Nil = 0xff
    };
    XLSPConnection();
    virtual ~XLSPConnection();
    virtual int ThreadStart();
    virtual void ThreadDone(int);

    void Poll();
    unsigned int GetServiceIP();
    void Connect(const char *, unsigned int);
    void Disconnect();

    State GetStateDesired() const { return mStateDesired; }
    State GetState() { return mState; }
    float GetTimerSplitMs() { return mThrottleTimer.SplitMs(); }

    static std::map<unsigned long, int> mXLSPRefCountMap;
    static bool SecureDisconnect(IN_ADDR);
    static int StartGatewayConnection(IN_ADDR);

private:
    void SetState(State);
    void StartEnumeration();

    static const int kTitleServerEnumMaxCount;

    State mState; // 0x4
    State mStateDesired; // 0x8
    String mStrServerFilter; // 0xc
    unsigned int mServiceId; // 0x14
    HANDLE mHEnum; // 0x18
    XTITLE_SERVER_INFO *mEnumBuf; // 0x1c
    DWORD mEnumBufSize; // 0x20
    unsigned int mEnumItemCount; // 0x24
    XOVERLAPPED mEnumXO; // 0x28
    IN_ADDR mSecureAddr; // 0x44
    Timer mThrottleTimer; // 0x48
};
