#pragma once
#include "xdk/XAPILIB.h"

// size 0x18
class ExternalMic {
public:
    ~ExternalMic();
    ExternalMic(unsigned long);
    HRESULT gatherGainAttribs(DWORD);
    HRESULT processGain(DWORD);
    void dataReady(unsigned long, unsigned long, _XOVERLAPPED *);
    DWORD sampleProcessThread();

    static int NumConnectedMics();
    static void Terminate();
    static void Init();

private:
    HANDLE mThread; // 0x0
    unsigned long unk4; // 0x4 - id
    bool unk8;
    bool unk9;
    float unkc; // 0xc
    float unk10;
    float unk14;
};

class ExternalMicClientMgr;
class ExternalMicClientProxy;
