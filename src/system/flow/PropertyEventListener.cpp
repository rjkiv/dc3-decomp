#include "flow/PropertyEventListener.h"
#include "flow/FlowNode.h"
#include "flow/DrivenPropertyEntry.h"
#include "obj/Object.h"

PropertyEventListener::PropertyEventListener(Hmx::Object *owner)
    : mAutoPropEntries(owner), mRegistered(0) {}

void PropertyEventListener::RegisterEvents(FlowNode *node) {
    static Symbol reactivate("reactivate");
    if (!mRegistered) {
        if (!mAutoPropEntries.empty()) {
            FOREACH (it, mAutoPropEntries) {
                if (it->mProvider) {
                    it->mProvider->AddPropertySink(node, it->mPropPath, reactivate);
                }
            }
        }
    }
    mRegistered = true;
}

void PropertyEventListener::UnregisterEvents(FlowNode *node) {
    if (mRegistered) {
        if (!mAutoPropEntries.empty()) {
            FOREACH (it, mAutoPropEntries) {
                if (it->mProvider) {
                    it->mProvider->RemovePropertySink(node, it->mPropPath);
                }
            }
        }
    }
    mRegistered = false;
}

void PropertyEventListener::GenerateAutoNames(FlowNode *node, bool clear) {
    if (clear) {
        mAutoPropEntries.clear();
    }
    auto &props = node->GetDrivenProperties();
    FOREACH (it, props) {
        FOREACH (op, it->mMathOps) {
            Hmx::Object *prov = op->mPropertyProvider;
            if (prov) {
                AutoPropEntry entry(node);
                entry.mProvider = prov;
                entry.mPropPath = op->mPropertyPath.Array();
                mAutoPropEntries.push_back(entry);
            }
        }
    }
}
