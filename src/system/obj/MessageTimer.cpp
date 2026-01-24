#include "obj/MessageTimer.h"
#include "obj/DataFunc.h"
#include "utl/Std.h"
#include <vector>

std::vector<EventEntry *> gEntries;
bool MessageTimer::sActive;

DataNode MessageTimerStop(DataArray *) {
    MessageTimer::Stop();
    return 0;
}

DataNode MessageTimerOn(DataArray *) { return MessageTimer::Active(); }

void EventEntry::Add(Hmx::Object *o, float ms) {
    Symbol sym =
        o ? MakeString("%s 0x%x", o->Name(), (int)o) : MakeString("0x%x", (int)o);
    for (int i = 0; i < objs.size(); i++) {
        if (objs[i]->name == sym) {
            ObjEntry *cur = objs[i];
            MaxEq(cur->maxMs, ms);
            cur->totalMs += ms;
            cur->num++;
            return;
        }
    }
    objs.push_back(new ObjEntry(sym, ms, 1));
}

void MessageTimer::AddTime(Hmx::Object *o, Symbol msg, float ms) {
    if (sActive) {
        for (int i = 0; i < gEntries.size(); i++) {
            if (gEntries[i]->msgs == msg) {
                gEntries[i]->Add(o, ms);
                return;
            }
        }
        gEntries.push_back(new EventEntry(msg, o, ms));
    }
}

DataNode MessageTimerStart(DataArray *) {
    MessageTimer::Start();
    return 0;
}

DataNode MessageTimerDump(DataArray *) {
    MessageTimer::Dump();
    return 0;
}

void MessageTimer::Start() {
    sActive = true;
    DeleteAll(gEntries);
}

void MessageTimer::Stop() { sActive = false; }

void MessageTimer::Dump() {
    MILO_LOG("Message Tracker Dump!\n");
    std::sort(gEntries.begin(), gEntries.end(), MaxSort());
    for (int i = 0; i < gEntries.size(); i++) {
        gEntries[i]->Dump();
    }
}

void MessageTimer::Init() {
    DataRegisterFunc("message_timer_start", MessageTimerStart);
    DataRegisterFunc("message_timer_stop", MessageTimerStop);
    DataRegisterFunc("message_timer_dump", MessageTimerDump);
    DataRegisterFunc("message_timer_on", MessageTimerOn);
}
