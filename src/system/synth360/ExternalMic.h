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
    volatile bool mShutdown; // 0x8
    bool mIsActive; // 0x9
    float mGain; // 0xc
    float mMinGain; // 0x10
    float mMaxGain; // 0x14
};

// size 0x8
class ExternalMicClientProxy {
public:
    ExternalMicClientProxy(DWORD index) : mIndex(index) {}
    HRESULT OnMicConnected(DWORD, bool, const Symbol &);

    bool IsConnected() const { return mConnected; }
    void SetConnected(bool b) { mConnected = b; }
    DWORD GetIndex() const { return mIndex; }

private:
    DWORD mIndex; // 0x0
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
