#pragma once
#include "stl/_vector.h"

class CharTaskMgr {
public:
    class Task {
    protected:
        float mStart; // 0x4
        float mEnd; // 0x8
        float mPriority; // 0xc
        int mConflictFlags; // 0x10
        bool mStarted; // 0x14
    };

    static bool sShowGraph;
    static int sNumInstances;
    static float sGraphPosY;
    static void Init();

    int mInstanceNum; // 0x0

protected:
    std::vector<Task *> mTasks; // 0x4
    Task *mConflict; // 0x10
};
