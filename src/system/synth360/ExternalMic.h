#pragma once
#include "xdk/XAPILIB.h"
#include "synth360/Mic.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/xbase.h"

// size 0x18
class ExternalMic {
public:
    ExternalMic(DWORD dwIndex);
    ~ExternalMic();

    DWORD sampleProcessThread();
    bool IsActive() { return mIsActive; }
    //   public: int32_t processCapabilities(uint32_t, _XMICCAPABILITIES*);
    HRESULT processGain(DWORD dwMicrophoneIndex);
    HRESULT gatherGainAttribs(DWORD dwMicrophoneIndex);
    void dataReady(
        DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, XOVERLAPPED *pOverlapped
    );

    static void Init();
    static void Terminate();
    static int NumConnectedMics();

private:
    HANDLE mThread; // 0x0
    DWORD mIndex; // 0x4
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

    HRESULT OnMicConnected(DWORD dwSampleRate, bool bIsWireless, const Symbol &symName);
    void OnMicDisconnected();
    void AddAudio(BYTE *pSamples, DWORD numBytes);
    float GetRequiredGain();
    bool IsConnected() const { return mConnected; }

private:
    DWORD mIndex; // 0x0
    bool mConnected; // 0x4
};

class ExternalMicClientMgr {
public:
    static void Init();
    static void Terminate();
    static void OnMicDisconnected(DWORD devIndex);
    static void AddAudio(DWORD devIndex, BYTE *pSamples, DWORD numBytes);
    static float GetRequiredGain(DWORD devIndex);
    static ExternalMicClientProxy *GetMasterForIndex(DWORD devIndex);
    static void Associate(int micID, MicXbox *mic);
    static MicXbox *GetClient(DWORD idx) { return mAssocMicXbox[idx]; }
    static bool ConnectedForClient(const MicXbox *pClient);

private:
    static std::vector<ExternalMicClientProxy *> mMicMasters;
    static std::vector<DWORD> mDevToMicMaster;
    static std::vector<DWORD> mMicMasterToDev;
    static std::vector<MicXbox *> mAssocMicXbox;
};

struct RequestCompletionData {
    DWORD dwMicrophoneIndex; // 0x0
    XOVERLAPPED *pOverlap; // 0x4
    DWORD dwFrameCount; // 0x8
    DWORD dwAudioFrameBufferLength; // 0xc
    PBYTE pAudioFramesBuffer; // 0x10
    PBYTE pAudioFramesBufferAlloc; // 0x14
    WORD wFrameLengths[8]; // 0x18
    ExternalMic *pOwner; // 0x28
};
