#include "flow/PropertyEventListener.h"
#include "flow/FlowNode.h"
#include "obj/Object.h"

PropertyEventListener::PropertyEventListener(Hmx::Object *owner)
    : mAutoPropEntries(owner), unk14(0) {}

void PropertyEventListener::RegisterEvents(FlowNode *node) {
    static Symbol reactivate("reactivate");
    if (!unk14) {
        if (!mAutoPropEntries.empty()) {
            for (ObjVector<AutoPropEntry>::iterator it = mAutoPropEntries.begin();
                 it != mAutoPropEntries.end();
                 ++it) {
                if (it->unk4) {
                    it->unk4->AddPropertySink(node, it->unk0, reactivate);
                }
            }
        }
    }
    unk14 = true;
}

void PropertyEventListener::UnregisterEvents(FlowNode *node) {
    if (unk14) {
        if (!mAutoPropEntries.empty()) {
            for (ObjVector<AutoPropEntry>::iterator it = mAutoPropEntries.begin();
                 it != mAutoPropEntries.end();
                 ++it) {
                if (it->unk4) {
                    it->unk4->RemovePropertySink(node, it->unk0);
                }
            }
        }
    }
    unk14 = false;
}

void PropertyEventListener::GenerateAutoNames(FlowNode *node, bool clear) {
    if (clear)
        mAutoPropEntries.clear();
}
