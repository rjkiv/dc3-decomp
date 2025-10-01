#pragma once
#include "hamobj/DetectFrame.h"
#include "hamobj/ErrorNode.h"
#include "hamobj/FilterVersion.h"
#include "hamobj/HamMove.h"
#include <vector>

// size 0x34
class FilterQueue {
public:
    // size 0x14
    class FilterInputFrame {
    public:
        int unk0;
        float unk4;
        float unk8;
        DetectFrame *unkc;
        const FilterVersion *unk10;
    };

    // size 0x214
    class FilterOutputFrame {
    public:
        int unk0;
        Vector3 unk4[kMaxNumErrorNodes]; // 0x4
    };

    class QueuedJob {
    public:
    };

    FilterQueue();

    bool GetResults(float &, DetectFrame **, float);
    void EnqueueNewJob(float, float, MoveMode);
    void EnqueueFrame(int, float, float, DetectFrame *, const FilterVersion *);
    bool IsJobFinished() const;
    float LastPollMs() const;
    bool HasJob() const;
    void CancelJob();
    void StartJob();

private:
    float unk0;
    MoveMode unk4;
    float unk8;
    std::vector<FilterInputFrame> qframes; // 0xc
    float unk18;
    MoveMode unk1c;
    std::vector<FilterOutputFrame> oframes; // 0x20
    bool jobFinished; // 0x2c
    float lastPollMs; // 0x30
};
