#include "flow/FlowCommand.h"
#include "flow/FlowNode.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"

FlowCommand::FlowCommand() : mObject(this), mHandler(0) {}
FlowCommand::~FlowCommand() {}

BEGIN_HANDLERS(FlowCommand)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowCommand)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowCommand)
    SAVE_REVS(3, 0)
    int size = mTypeProps ? mTypeProps->Map()->Size() : 0;
    DataArrayPtr ptr(new DataArray(size));
    bs << size;
    for (int i = 0; i < size; i++) {
        bs << mTypeProps->Map()->Evaluate(i);
        ptr->Node(i) = mTypeProps->Map()->Evaluate(i);
    }
    ClearAllTypeProps();
    SAVE_SUPERCLASS(FlowNode)
    for (int i = 0; i < size; i += 2) {
        SetProperty(ptr->Sym(i), ptr->Node(i + 1));
    }
    bs << mObject;
    bs << mHandler;
END_SAVES

bool FlowCommand::Activate() {
    FLOW_LOG("Activate\n");
    unk58 = false;
    PushDrivenProperties();
    if (mObject && !mHandler.Null()) {
        int size = mTypeProps ? mTypeProps->Size() : 0;
        Message msg(size);
        msg.SetType(mHandler);
        for (int i = 0; i < size; i += 2) {
            msg[i] = mTypeProps->Map()->Evaluate(i + 1);
        }
        mObject->Handle(msg, false);
    }
    return false;
}
