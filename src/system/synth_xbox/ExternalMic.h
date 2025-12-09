#pragma once
#include "xdk/win_types.h"
#include "xdk/xapilibi/xbase.h"

class ExternalMic {
public:
    ~ExternalMic();
    ExternalMic(unsigned long);
    long gatherGainAttribs(unsigned long);
    long processGain(unsigned long);
    void dataReady(unsigned long, unsigned long, _XOVERLAPPED *);
    unsigned long sampleProcessThread();

    static int NumConnectedMics();
    static void Terminate();
    static void Init();

    HANDLE mThread; // 0x0
    unsigned long unk4;
    bool unk8;
    bool unk9;
    float unkc;
};
