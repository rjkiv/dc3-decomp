#pragma once

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

#ifdef __cplusplus
}
#endif
