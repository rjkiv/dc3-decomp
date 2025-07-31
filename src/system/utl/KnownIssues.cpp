#include "utl/KnownIssues.h"
#include "obj/DataFunc.h"

KnownIssues TheKnownIssues;

KnownIssues::KnownIssues(void) : unk_0x0(), unk_0x8(), unk_0x14(true) {}

void KnownIssues::Display(String, float) {
    if (TheKnownIssues.unk_0x14) {
    }
}

DataNode KnownIssues::OnDisplayKnownIssues(DataArray *msg) {
    if (msg->Size() > 2) {
        TheKnownIssues.Display(msg->Str(1), msg->Float(2));
    } else {
        TheKnownIssues.Display(msg->Str(1), 5.0f);
    }
    return 0;
}
DataNode KnownIssues::OnToggleLastKnownIssues(DataArray *) { // ?????
    if (TheKnownIssues.unk_0x10 == 0.0f) {
        TheKnownIssues.unk_0x10 = -1.0f;
        return 1;
    }
    TheKnownIssues.unk_0x10 = 0.0f;
    return 0;
}
DataNode KnownIssues::OnToggleAllowKnownIssues(DataArray *msg) {
    bool ret = false;
    if (!TheKnownIssues.unk_0x14) {
        ret = true;
    } else {
        ret = false;
        TheKnownIssues.unk_0x10 = 0.0f;
    }
    TheKnownIssues.unk_0x14 = ret;
    return ret;
}

void KnownIssues::Init() {
    DataRegisterFunc("display_known_issues", OnDisplayKnownIssues);
    DataRegisterFunc("toggle_last_known_issues", OnToggleLastKnownIssues);
    DataRegisterFunc("toggle_allow_known_issues", OnToggleAllowKnownIssues);
}
