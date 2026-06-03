#pragma once
#include "xdk/XAPILIB.h"
#include "synth360/Mic.h"

// size 0x18
class ExternalMic {
public:
    ExternalMic(DWORD);
    ~ExternalMic();
    HRESULT gatherGainAttribs(DWORD);
    HRESULT processGain(DWORD);
    void dataReady(DWORD, DWORD, _XOVERLAPPED *);
    DWORD sampleProcessThread();

    static int NumConnectedMics();
    static void Terminate();
    static void Init();

private:
    HANDLE mThread; // 0x0
    DWORD mMicIndex; // 0x4
    bool unk8;
    bool mConnected; // 0x9
    float mGain; // 0xc
    float mMinGain; // 0x10
    float mMaxGain; // 0x14
};

// size 0x8
class ExternalMicClientProxy {
public:
    ExternalMicClientProxy(DWORD dw) : unk0(dw) {}
    HRESULT OnMicConnected(DWORD, bool, const Symbol &);

    DWORD unk0; // 0x0
    bool mConnected; // 0x4
};

class ExternalMicClientMgr {
public:
    static void Init();
    static void Terminate();
    static bool ConnectedForClient(const MicXbox *);
    static void Associate(int, MicXbox *);
    static float GetRequiredGain(DWORD);
    static ExternalMicClientProxy *GetMasterForIndex(DWORD);
    static void AddAudio(DWORD, BYTE *, DWORD);
    static void OnMicDisconnected(DWORD);

    static MicXbox *AssociatedMic(int idx) { return mAssocMicXbox[idx]; }

private:
    static std::vector<MicXbox *> mAssocMicXbox;
    static std::vector<ExternalMicClientProxy *> mMicMasters;
    static std::vector<DWORD> mDevToMicMaster;
    static std::vector<DWORD> mMicMasterToDev;
};
