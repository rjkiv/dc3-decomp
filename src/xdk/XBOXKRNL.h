#pragma once
#include "xdk/win_types.h"

// size 0x8
struct _LIST_ENTRY {
    struct _LIST_ENTRY *Flink;
    struct _LIST_ENTRY *Blink;
};

// size 0x1C
struct _RTL_CRITICAL_SECTION {
    union {
        struct {
            unsigned char Type;
            unsigned char SpinCount;
            unsigned char Size;
            unsigned char Inserted;
            int SignalState;
            struct _LIST_ENTRY WaitListHead;
        } Event;
        struct {
            unsigned int SpinCount;
            void *Handle;
        } Usermode;
        unsigned int RawEvent[4];
    } Synchronization; // 0x0
    int LockCount; // 0x10
    int RecursionCount; // 0x14
    void *OwningThread; // 0x18
};

typedef struct _RTL_CRITICAL_SECTION RTL_CRITICAL_SECTION;

#ifdef __cplusplus
extern "C" {
#endif

void RtlInitializeCriticalSection(RTL_CRITICAL_SECTION *);
void RtlEnterCriticalSection(RTL_CRITICAL_SECTION *);
void RtlLeaveCriticalSection(RTL_CRITICAL_SECTION *);
int RtlTryEnterCriticalSection(RTL_CRITICAL_SECTION *);

// huuuuuge shoutout to xenia canary
enum XCONFIG_USER_AUDIO_FLAGS {
    // Audio Mode Analog
    DolbyProLogic = 0x00000001,
    AnalogMono = 0x00000002,
    // Audio Mode Digital
    DigitalStereo = 0x00000000,
    DolbyDigital = 0x00010000,
    DolbyDigitalWithWMAPRO = 0x00030000,
    // Special Flags
    StereoBypass = 0x00000003,
    LowLatency = 0x80000000,
};

VOID XAudioGetSpeakerConfig(LPDWORD pConfig);
VOID XAudioOverrideSpeakerConfig(DWORD dwConfig);

#ifdef __cplusplus
}
#endif
