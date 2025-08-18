#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"
#include "utl/SongPos.h"

enum TaskUnits {
    kTaskSeconds = 0,
    kTaskBeats = 1,
    kTaskUISeconds = 2,
    kTaskTutorialSeconds = 3,
    kTaskNumUnits = 4
};

/**
 * @brief Generic task type for simplistic multithreading.
 * Usually used for non-realtime tasks, such as scripting, message passing, etc
 */
class Task : public Hmx::Object {
public:
    Task() {}
    // virtual ~Task() {}
    virtual void Poll(float) = 0;

    MEM_OVERLOAD(Task, 0x1A);
};

class MessageTask : public Task {
private:
    ObjOwnerPtr<Hmx::Object> mObj; // 0x2c
    DataArray *mMsg; // 0x40
public:
    MessageTask(Hmx::Object *, DataArray *);
    virtual ~MessageTask();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    virtual void Poll(float);
};

class ScriptTask : public Task {
public:
    struct Var {
        Var(DataNode *np, const DataNode &n) : var(np), value(n) {}
        Var(DataNode *np) : var(np), value(*np) {}
        void Swap();

        DataNode *var; // 0x0
        DataNode value; // 0x4
    };
    ScriptTask(DataArray *, bool, DataArray *);
    virtual ~ScriptTask();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    virtual void Poll(float);

protected:
    std::list<Var> mVars; // 0x2c
    ObjPtrList<Hmx::Object> mObjects; // 0x34
    ObjOwnerPtr<Hmx::Object> mThis; // 0x48
    DataArray *mScript; // 0x5c
    bool mOnce; // 0x60

    void SwapVars();
    void UpdateVarsObjects(DataArray *);
};

class ThreadTask : public ScriptTask {
private:
    bool mWait; // 0x64
    int mCurrent; // 0x68
    float mTime; // 0x6c
    bool mExecuting; // 0x70
    float mTimeout; // 0x74

    DataNode OnWait(DataArray *);
    DataNode OnWaitTimeout(DataArray *);
    DataNode OnSleep(DataArray *);
    DataNode OnLoop(DataArray *);
    DataNode OnExit(DataArray *);
    DataNode OnCurrent(DataArray *);
    DataNode OnSetCurrent(DataArray *);

public:
    ThreadTask(DataArray *, DataArray *);
    virtual ~ThreadTask() {}
    virtual bool Replace(ObjRef *, Hmx::Object *);
    virtual DataNode Handle(DataArray *, bool);
    virtual void Poll(float);
};

class TaskTimeline {
public:
    struct TaskInfo {
        TaskInfo(Task *t, float f) : unk0(nullptr, t), unk14(f) {}
        ObjPtr<Task> unk0;
        float unk14;
    };

    TaskTimeline();
    ~TaskTimeline();
    void Poll();
    void AddTask(Task *, float);
    void ClearTasks();
    void ResetTaskTime(float);

    void SetTime(float f, bool b) {
        mLastTime = b ? f : mTime;
        mTime = f;
    }

    float GetTime() const { return mTime; }
    float DeltaTime() const { return mTime - mLastTime; }

    void SetDeltaTime(float delta) { mLastTime = mTime - delta; }
    void SetTimeAndDelta(float time, float delta) {
        mLastTime = time - delta;
        mTime = time;
    }

private:
    std::list<TaskInfo> mTasks; // 0x0
    std::list<TaskInfo> mAddedTasks; // 0x8
    float mTime; // 0x10
    float mLastTime; // 0x14
    Task *mPollingTask; // 0x18

    void AddTask(const TaskInfo &);
};

class TaskMgr : public Hmx::Object {
public:
    enum TimeReference {
        kDelayedTime = 0,
        kRealTime = 1
    };

    TaskMgr();
    virtual ~TaskMgr();
    virtual DataNode Handle(DataArray *, bool);

    float UISeconds() const;
    float DeltaSeconds() const;
    float DeltaBeat() const;
    float DeltaUISeconds() const;
    float Beat() const;
    float Seconds(TimeReference) const;
    float TutorialSeconds() const;
    float DeltaTime(TaskUnits) const;
    void Start(Task *, TaskUnits, float);
    float Time(TaskUnits) const;
    const char *GetMBT();
    void ClearTasks();
    void SetSeconds(float, bool);
    void SetDeltaTime(TaskUnits, float);
    void SetTimeAndDelta(TaskUnits, float, float);
    void Poll();
    void Init();
    void Terminate();
    void SetUISeconds(float, bool);
    void SetSecondsAndBeat(float, float, bool);
    void SetAVOffset(float);
    float DeltaTutorialSeconds() const;
    void ClearTimelineTasks(TaskUnits);
    void QueueTaskDelete(Task *);
    void ResetTaskTime(float, float);
    void ResetBeatTaskTime(float);

private:
    DataNode OnTimeTilNext(DataArray *);
    // const SongPos &GetSongPos() const { return mSongPos; }

    TaskTimeline *mTimelines; // 0x2c
    SongPos mSongPos; // 0x30
    bool mAutoSecondsBeats; // 0x48
    int unk4c; // 0x4c
    Timer mTime; // 0x50
    float mAVOffset; // 0x80
    std::vector<ObjPtr<Task> > unk84; // 0x84
};

extern TaskMgr TheTaskMgr;
