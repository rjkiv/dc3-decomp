#include "obj/Task.h"
#include "Dir.h"
#include "obj/Data.h"
#include "obj/DataFunc.h"
#include "obj/DataUtl.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/Timer.h"
#include "utl/BeatMap.h"
#include "utl/TempoMap.h"

TaskMgr TheTaskMgr;

#pragma region MessageTask

MessageTask::MessageTask(Hmx::Object *o, DataArray *msg) : mObj(this, o), mMsg(msg) {
    MILO_ASSERT(msg, 0x1D);
    msg->AddRef();
}

MessageTask::~MessageTask() {
    if (mMsg) {
        mMsg->Release();
        mMsg = nullptr;
    }
}

bool MessageTask::Replace(ObjRef *from, Hmx::Object *to) {
    if (from == &mObj) {
        if (!to) {
            delete this;
        } else {
            mObj = to;
        }
        return true;
    } else
        return Hmx::Object::Replace(from, to);
}

void MessageTask::Poll(float) {
    if (mObj && mMsg) {
        DataNode handled = mObj->Handle(mMsg, false);
        if (handled.Type() != kDataUnhandled) {
            delete this;
        }
    }
}

#pragma endregion
#pragma region ScriptTask

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

bool ScriptTask::Replace(ObjRef *from, Hmx::Object *to) {
    if (from == &mThis) {
        mThis = to;
        if (mThis) {
            return true;
        }
    } else {
        if (from->Parent() != &mObjects) {
            // mObjects = to;
            return true;
        } else
            return Hmx::Object::Replace(from, to);
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

void ScriptTask::SwapVars() {
    FOREACH (it, mVars) {
        it->Swap();
    }
}

void ScriptTask::Var::Swap() {
    DataNode tmp = value;
    value = *var;
    *var = tmp;
}

void ScriptTask::UpdateVarsObjects(DataArray *d) {
    MILO_ASSERT(d, 0x9E);
    int size = d->Size();
    for (int i = 0; i < size; i++) {
        DataType curType = d->Type(i);
        Hmx::Object *obj = nullptr;
        if (curType == kDataObject) {
            obj = d->UncheckedObj(i);
        } else if (curType == kDataSymbol || curType == kDataString) {
            const char *name = d->LiteralStr(i);
            ObjectDir *search = mThis ? mThis->DataDir() : ObjectDir::Main();
            obj = search->FindObject(name, true, true);
        } else if (curType == kDataVar) {
            DataNode *var = d->UncheckedVar(i);
            FOREACH (it, mVars) {
                if (it->var == var) {
                    var = nullptr;
                    break;
                }
            }
            if (var) {
                mVars.push_back(Var(var));
            }
        } else if (curType == kDataArray || curType == kDataCommand) {
            UpdateVarsObjects(d->UncheckedArray(i));
        }

        if (obj && obj != mThis && mObjects.find(obj) == mObjects.end()) {
            mObjects.push_back(obj);
        }
    }
}

#pragma endregion
#pragma region ThreadTask

ThreadTask::ThreadTask(DataArray *script, DataArray *updateVarsObjs)
    : ScriptTask(script, false, updateVarsObjs), mWait(false), mCurrent(1), mTime(0),
      mExecuting(false), mTimeout(-1) {}

bool ThreadTask::Replace(ObjRef *from, Hmx::Object *to) {
    if (mExecuting) {
        if (&mObjects == from->Parent() && from) {
            mObjects.remove(to);
            return true;
        }
    }
    return ScriptTask::Replace(from, to);
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
        for (; mCurrent < mScript->Size();) {
            int cur = mCurrent;
            mScript->Command(mCurrent)->Execute();
            if (mWait)
                break;
            if (mCurrent == cur) {
                mCurrent++;
            }
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

#pragma endregion
#pragma region TaskTimeline

TaskTimeline::TaskTimeline() : mTime(0), mLastTime(0), mPollingTask(nullptr) {}
TaskTimeline::~TaskTimeline() {}

void TaskTimeline::ClearTasks() {
    FOREACH (it, mTasks) {
        if (it->unk0 != mPollingTask) {
            delete it->unk0;
        }
    }
}

void TaskTimeline::ResetTaskTime(float time) {
    float delta = time - mTime;
    FOREACH (it, mTasks) {
        it->unk14 += delta;
    }
    FOREACH (it, mAddedTasks) {
        it->unk14 += delta;
    }
    mTime += delta;
    mLastTime += delta;
}

void TaskTimeline::AddTask(const TaskInfo &info) {
    if (info.unk14 > mTime || info.unk0) {
        if (mPollingTask) {
            mAddedTasks.push_back(info);
        } else {
            FOREACH (it, mTasks) {
                if (info.unk14 < it->unk14) {
                    mTasks.insert(it, info);
                    return;
                }
            }
            mTasks.push_back(info);
        }
    }
}

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
    FOREACH (it, mAddedTasks) {
        AddTask(*it);
    }
    mAddedTasks.clear();
}

void TaskTimeline::AddTask(Task *task, float f) { AddTask(TaskInfo(task, mTime + f)); }

#pragma endregion
#pragma region TaskMgr

TaskMgr::TaskMgr() { mTimelines = new TaskTimeline[4]; }

TaskMgr::~TaskMgr() {
    delete[] mTimelines;
    mTimelines = nullptr;
}

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

void TaskMgr::Poll() {
    START_AUTO_TIMER("anim");
    mTime.Split();
    if (mAutoSecondsBeats) {
        float secs = mTime.Ms() / 1000.0f;
        mTimelines[kTaskSeconds].SetTime(secs, false);
        mTimelines[kTaskBeats].SetTime(secs * 2.0f, false);
    }
    for (int i = 0; i < kTaskNumUnits; i++) {
        mTimelines[i].Poll();
    }
    for (int i = 0; i < unk84.size(); i++) {
        delete unk84[i];
    }
    unk84.clear();
}

void TaskMgr::ClearTasks() {
    for (int i = 0; i < kTaskNumUnits; i++) {
        mTimelines[i].ClearTasks();
    }
}

void TaskMgr::ClearTimelineTasks(TaskUnits u) { mTimelines[u].ClearTasks(); }

void TaskMgr::ResetTaskTime(float f1, float f2) {
    ResetSecondTaskTime(f1);
    ResetBeatTaskTime(f2);
}

void TaskMgr::ResetSecondTaskTime(float time) {
    mTimelines[kTaskSeconds].ResetTaskTime(time);
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

#pragma endregion
