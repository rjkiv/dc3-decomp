#include "obj/Task.h"
#include "Dir.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/DataUtl.h"

#include "obj/Object.h"
#include "os/Debug.h"
#include "utl/BeatMap.h"
#include "utl/TempoMap.h"

TaskMgr TheTaskMgr;

void TaskMgr::Terminate() { SetName(nullptr, nullptr); }
void TaskMgr::SetUISeconds(float f, bool b) { mTimelines[kTaskUISeconds].SetTime(f, b); }
float TaskMgr::UISeconds() const { return mTimelines[kTaskUISeconds].GetTime(); }
float TaskMgr::DeltaUISeconds() const { return mTimelines[kTaskUISeconds].DeltaTime(); }
float TaskMgr::DeltaTime(TaskUnits u) const { return mTimelines[u].DeltaTime(); }

void TaskMgr::SetDeltaTime(TaskUnits u, float delta) {
    mTimelines[u].SetDeltaTime(delta);
}

void TaskMgr::SetTimeAndDelta(TaskUnits u, float time, float delta) {
    mTimelines[u].SetTimeAndDelta(time, delta);
}

void TaskMgr::SetSecondsAndBeat(float f1, float f2, bool b) {
    mAutoSecondsBeats = false;
    mTimelines[kTaskSeconds].SetTime(f1, b);
    mTimelines[kTaskBeats].SetTime(f2, b);
}

void TaskMgr::SetSeconds(float f, bool b) {
    mAutoSecondsBeats = false;
    mTimelines[kTaskSeconds].SetTime(f, b);
    mTimelines[kTaskBeats].SetTime(
        TheBeatMap->Beat(TheTempoMap->TimeToTick(f * 1000.0f)), b
    );
}

float TaskMgr::Time(TaskUnits u) const { return mTimelines[u].GetTime(); }
void TaskMgr::SetAVOffset(float offset) { mAVOffset = offset; }

float TaskMgr::Seconds(TimeReference ref) const {
    float time = mTimelines[kTaskSeconds].GetTime();
    if (ref == 0) {
        time -= mAVOffset;
    }
    return time;
}

float TaskMgr::DeltaSeconds() const { return mTimelines[kTaskSeconds].DeltaTime(); }
float TaskMgr::Beat() const { return mTimelines[kTaskBeats].GetTime(); }
float TaskMgr::DeltaBeat() const { return mTimelines[kTaskBeats].DeltaTime(); }
float TaskMgr::TutorialSeconds() const {
    return mTimelines[kTaskTutorialSeconds].GetTime();
}

const char *TaskMgr::GetMBT() {
    return MakeString(
        "%d:%d:%03d", mSongPos.GetMeasure(), mSongPos.GetBeat(), mSongPos.GetTick()
    );
}

void ScriptTask::Var::Swap() {
    DataNode tmp = value;
    value = *var;
    *var = tmp;
}

MessageTask::~MessageTask() {
    if (mMsg) {
        mMsg->Release();
        mMsg = nullptr;
    }
}

bool MessageTask::Replace(ObjRef *ref, Hmx::Object *o) {
    if (ref == &mObj) {
        if (!o) {
            delete this;
        } else {
            mObj.SetObjConcrete(o);
        }
        return true;
    } else
        return Hmx::Object::Replace(ref, o);
}

void MessageTask::Poll(float f1) {
    if (mObj && mMsg) {
        DataNode handled = mObj->Handle(mMsg, false);
        if (handled.Type() != kDataUnhandled) {
            delete this;
        }
    }
}

void ScriptTask::SwapVars() {
    for (std::list<Var>::iterator it = mVars.begin(); it != mVars.end(); ++it) {
        it->Swap();
    }
}

void ThreadTask::Poll(float f1) {
    if (mExecuting) {
        MILO_FAIL("Can't re-enter ThreadTask::Poll");
    }
    MILO_ASSERT(mScript, 0x11A);
    if (f1 >= mTime) {
        mTime = f1;
        mExecuting = true;
        mWait = false;
        SwapVars();
        Hmx::Object *oldThis = DataSetThis(mThis);
        for (; mCurrent < mScript->Size(); mCurrent++) {
            mScript->Command(mCurrent)->Execute(true);
            if (mWait)
                break;
        }
        DataSetThis(oldThis);
        SwapVars();
        mExecuting = false;
        if (mCurrent == mScript->Size()) {
            delete this;
        }
    }
}

DataNode ThreadTask::OnSetCurrent(DataArray *arr) {
    mCurrent = arr->Int(2);
    return 0;
}

DataNode ThreadTask::OnCurrent(DataArray *) { return mCurrent; }

DataNode ThreadTask::OnWait(DataArray *arr) {
    mWait = arr->Int(2);
    return 0;
}

DataNode ThreadTask::OnWaitTimeout(DataArray *a) {
    MILO_ASSERT(a->Size() > 2, 0x161);
    if (mTimeout == -1.0f) {
        mTimeout = mTime + a->Float(3);
    }
    bool d1 = mTime >= mTimeout;
    mWait = !d1 && a->Int(2);
    if (!mWait) {
        mTimeout = -1.0f;
        if (a->Size() > 3) {
            *a->Var(4) = d1;
        }
    }
    return 0;
}

DataNode ThreadTask::OnSleep(DataArray *arr) {
    mWait = true;
    mTime += arr->Float(2);
    mCurrent++;
    return 0;
}

DataNode ThreadTask::OnLoop(DataArray *arr) {
    mCurrent = 1;
    return 0;
}

DataNode ThreadTask::OnExit(DataArray *arr) {
    mWait = true;
    mCurrent = mScript->Size();
    return 0;
}

void TaskTimeline::ClearTasks() {
    for (std::list<TaskInfo>::iterator it = mTasks.begin(); it != mTasks.end(); ++it) {
        Task *task = it->unk0;
        if (task != mPollingTask) {
            delete task;
        }
    }
}

void TaskTimeline::ResetTaskTime(float time) {
    float delta = time - mTime;
    for (std::list<TaskInfo>::iterator it = mTasks.begin(); it != mTasks.end(); ++it) {
        it->unk14 += delta;
    }
    for (std::list<TaskInfo>::iterator it = mAddedTasks.begin(); it != mAddedTasks.end();
         ++it) {
        it->unk14 += delta;
    }
    mTime += delta;
    mLastTime += delta;
}

void TaskMgr::ClearTasks() {
    for (int i = 0; i < kTaskNumUnits; i++) {
        mTimelines[i].ClearTasks();
    }
}

void TaskMgr::ClearTimelineTasks(TaskUnits u) { mTimelines[u].ClearTasks(); }

void TaskMgr::ResetTaskTime(float f1, float f2) {
    mTimelines[kTaskSeconds].ResetTaskTime(f1);
    mTimelines[kTaskBeats].ResetTaskTime(f2);
}

void TaskMgr::ResetBeatTaskTime(float time) {
    mTimelines[kTaskBeats].ResetTaskTime(time);
}

DataNode TaskMgr::OnTimeTilNext(DataArray *arr) {
    float f2 = arr->Float(2);
    float f3 = arr->Float(3);
    float beat = Beat();
    float floored = floor(beat / f2);
    float f1 = f3 * (1.0f - (beat / f2 - floored));
    if (f2 - f1 <= f3) {
        return 0.0f;
    } else
        return f1;
}

MessageTask::MessageTask(Hmx::Object *o, DataArray *msg) : mObj(this, o), mMsg(msg) {
    MILO_ASSERT(msg, 0x1D);
    msg->AddRef();
}

bool ScriptTask::Replace(ObjRef *ref, Hmx::Object *obj) {
    if (ref == &mThis) {
        mThis.SetObjConcrete(obj);
        if (mThis)
            return true;
    } else {
        if (ref->Parent() != &mObjects) {
            // mObjects.SetObjConcrete(obj);
            return true;
        } else
            return Hmx::Object::Replace(ref, obj);
    }
    delete this;
    return true;
}

void ScriptTask::Poll(float f1) {
    MILO_ASSERT(mScript, 0xCF);
    SwapVars();
    static DataArrayPtr args(new DataArray(1));
    args->Node(0) = f1;
    DataNode handled = mScript->ExecuteScript(1, mThis, args, 0);
    SwapVars();
    if (mOnce || !handled.Int()) {
        delete this;
    }
}

bool ThreadTask::Replace(ObjRef *ref, Hmx::Object *obj) {
    if (mExecuting) {
        if (&mObjects == ref->Parent() && ref) {
            mObjects.remove(obj);
            return true;
        }
    }
    return ScriptTask::Replace(ref, obj);
}

BEGIN_HANDLERS(ThreadTask)
    HANDLE(wait, OnWait)
    HANDLE(wait_timeout, OnWaitTimeout)
    HANDLE(sleep, OnSleep)
    HANDLE(loop, OnLoop)
    HANDLE(exit, OnExit)
    HANDLE(current, OnCurrent)
    HANDLE(set_current, OnSetCurrent)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_HANDLERS(TaskMgr)
    HANDLE_ACTION(clear_tasks, ClearTasks())
    HANDLE_EXPR(seconds, Seconds(kRealTime))
    HANDLE_EXPR(ms, Seconds(kRealTime) * 1000.0f)
    HANDLE_EXPR(delta_seconds, mTimelines[kTaskSeconds].DeltaTime())
    HANDLE_EXPR(beat, mTimelines[kTaskBeats].GetTime())
    HANDLE_EXPR(delta_beat, mTimelines[kTaskBeats].DeltaTime())
    HANDLE_EXPR(ui_seconds, mTimelines[kTaskUISeconds].GetTime())
    HANDLE_EXPR(ui_delta_seconds, mTimelines[kTaskUISeconds].DeltaTime())
    HANDLE_EXPR(tutorial_seconds, mTimelines[kTaskTutorialSeconds].GetTime())
    HANDLE_EXPR(delta_tutorial_seconds, mTimelines[kTaskTutorialSeconds].DeltaTime())
    HANDLE_EXPR(mbt, GetMBT())
    HANDLE_EXPR(total_tick, mSongPos.GetTotalTick())
    HANDLE(time_til_next, OnTimeTilNext)
    HANDLE_ACTION(set_seconds, SetSeconds(_msg->Float(2), _msg->Int(3)))
    HANDLE_ACTION(set_auto_seconds_beats, mAutoSecondsBeats = _msg->Int(2))
END_HANDLERS

void TaskTimeline::AddTask(const TaskInfo &info) {
    if (info.unk14 > mTime || info.unk0) {
        if (mPollingTask) {
            mAddedTasks.push_back(info);
        } else {
            for (std::list<TaskInfo>::iterator it = mTasks.begin(); it != mTasks.end();
                 ++it) {
                if (info.unk14 < (*it).unk14) {
                    mTasks.insert(it, info);
                    return;
                }
            }
            mTasks.push_back(info);
        }
    }
}

ScriptTask::ScriptTask(DataArray *script, bool once, DataArray *updateVarsObjs)
    : mObjects(this, kObjListOwnerControl), mThis(this, DataThis()), mScript(script),
      mOnce(once) {
    script->AddRef();
    static DataNode &taskvar = DataVariable("task");
    DataNode old = taskvar;
    taskvar = this;
    mVars.push_back(Var(&taskvar));
    if (!updateVarsObjs)
        updateVarsObjs = script;
    UpdateVarsObjects(updateVarsObjs);
    taskvar = old;
}

ScriptTask::~ScriptTask() { mScript->Release(); }

ThreadTask::ThreadTask(DataArray *script, DataArray *updateVarsObjs)
    : ScriptTask(script, false, updateVarsObjs), mWait(false), mCurrent(1), mTime(0),
      mExecuting(false), mTimeout(-1) {}

void TaskTimeline::Poll() {
    for (std::list<TaskInfo>::iterator it = mTasks.begin(); it != mTasks.end();) {
        if (it->unk14 > mTime)
            break;
        float f1 = it->unk14;
        float f2 = mTime;
        float diff = f2 - f1;
        if ((*it).unk0) {
            mPollingTask = (*it).unk0;
            (*it).unk0->Poll(diff);
            ++it;
        } else {
            it = mTasks.erase(it);
        }
    }
    mPollingTask = nullptr;
    for (std::list<TaskInfo>::iterator it = mAddedTasks.begin(); it != mAddedTasks.end();
         ++it) {
        AddTask(*it);
    }
    mAddedTasks.clear();
}

void TaskTimeline::AddTask(Task *task, float f) { AddTask(TaskInfo(task, mTime + f)); }

void TaskMgr::Start(Task *t, TaskUnits u, float f) { mTimelines[u].AddTask(t, f); }

DataNode OnScriptTask(DataArray *arr) {
    static Symbol script("script");
    static Symbol once("once");
    static Symbol units("units");
    static Symbol delay("delay");
    static Symbol name("name");
    static Symbol preserve("preserve");
    bool local_once = true;
    float local_delay = 0;
    int local_units = arr->Int(1);
    const char *local_name = nullptr;
    arr->FindData(once, local_once, false);
    arr->FindData(delay, local_delay, false);
    arr->FindData(units, local_units, false);
    arr->FindData(name, local_name, false);

    ScriptTask *task = new ScriptTask(
        arr->FindArray(script), local_once, arr->FindArray(preserve, false)
    );
    if (local_name) {
        MILO_ASSERT(DataThis(), 0x94);
        task->SetName(local_name, DataThis()->DataDir());
    }
    TheTaskMgr.Start(task, (TaskUnits)local_units, local_delay);
    return task;
}

DataNode OnThreadTask(DataArray *arr) {
    static Symbol script("script");
    static Symbol delay("delay");
    static Symbol name("name");
    static Symbol preserve("preserve");

    float local_delay = 0;
    int units = arr->Int(1);
    const char *local_name = nullptr;
    arr->FindData(delay, local_delay, false);
    arr->FindData(name, local_name, false);
    ThreadTask *task =
        new ThreadTask(arr->FindArray(script), arr->FindArray(preserve, false));
    if (local_name) {
        MILO_ASSERT(DataThis(), 0xFF);
        task->SetName(local_name, DataThis()->DataDir());
    }
    TheTaskMgr.Start(task, (TaskUnits)units, local_delay);
    return task;
}

TaskTimeline::TaskTimeline() : mTime(0), mLastTime(0), mPollingTask(nullptr) {}
TaskTimeline::~TaskTimeline() {}

void TaskMgr::Init() {
    mTime.Restart();
    mAutoSecondsBeats = true;
    if (ObjectDir::Main()) {
        SetName("taskmgr", ObjectDir::Main());
        DataRegisterFunc("script_task", OnScriptTask);
        DataRegisterFunc("thread_task", OnThreadTask);
    }
}

void TaskMgr::QueueTaskDelete(Task *task) {
    if (task) {
        for (int i = 0; i < unk84.size(); i++) {
            if (unk84[i] == task)
                return;
        }
        unk84.push_back(ObjPtr<Task>(nullptr, task));
    }
}

TaskMgr::TaskMgr() { mTimelines = new TaskTimeline[4]; }

TaskMgr::~TaskMgr() {
    delete[] mTimelines;
    mTimelines = nullptr;
}
