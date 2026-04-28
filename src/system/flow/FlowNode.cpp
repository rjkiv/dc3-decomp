#include "flow/FlowNode.h"
#include "flow/DrivenPropertyEntry.h"
#include "flow/FlowLabel.h"
#include "math/Utl.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "obj/Utl.h"
#include "os/Debug.h"
#include "flow/Flow.h"
#include "utl/Str.h"

float FlowNode::sIntensity = 1.0f;
bool FlowNode::sPushDrivenProperties = false;

#pragma region Hmx::Object

FlowNode::FlowNode()
    : mChildNodes(this, (EraseMode)0, kObjListNoNull), mRunningNodes(this),
      mFlowParent(nullptr), mDrivenPropEntries(this), unk58(0) {
    mDebugOutput = false;
}

FlowNode::~FlowNode() {
    if (!mRunningNodes.empty()) {
        Deactivate(true);
    }
    while (!mChildNodes.empty()) {
        FlowNode *cur = mChildNodes.front();
        delete cur;
    }
}

BEGIN_HANDLERS(FlowNode)
    HANDLE_ACTION(activate, Activate());
    HANDLE_ACTION(deactivate, Deactivate(false));
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(FlowNode)
    SYNC_PROP_SET(comment, Note(), SetNote(_val.Str()))
    SYNC_PROP(debug_output, mDebugOutput)
    SYNC_PROP(debug_comment, mDebugComment)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(FlowNode)
    SAVE_REVS(2, 0)
    if (!dynamic_cast<Flow *>(this)) {
        SAVE_SUPERCLASS(Hmx::Object)
    }
    ObjPtrVec<FlowNode> flowNodes(this);
    FOREACH (it, mChildNodes) {
        if ((*it)->Dir() == Dir()) {
            flowNodes.push_back(*it);
        }
    }
    bs << flowNodes;
    bs << (int)mDrivenPropEntries.size();
    FOREACH (it, mDrivenPropEntries) {
        it->Save(bs);
    }
    bs << mDebugOutput;
    bs << mDebugComment;
END_SAVES

BEGIN_COPYS(FlowNode)
    COPY_SUPERCLASS(Hmx::Object)
    CREATE_COPY(FlowNode)
    BEGIN_COPYING_MEMBERS
        if (!dynamic_cast<Flow *>(this)) {
            FOREACH (it, c->mChildNodes) {
                FlowNode *n = DuplicateChild(*it);
                if (n) {
                    n->SetParent(this, true);
                }
            }
        }
        COPY_MEMBER(mDrivenPropEntries)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(2, 0)

BEGIN_LOADS(FlowNode)
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    if (!dynamic_cast<Flow *>(this)) {
        LOAD_SUPERCLASS(Hmx::Object)
    }
    d >> mChildNodes;
    FOREACH (it, mChildNodes) {
        (*it)->SetParent(this, false);
    }
    int numEntries;
    d >> numEntries;
    mDrivenPropEntries.clear();
    mDrivenPropEntries.reserve(numEntries);
    for (int i = 0; i < numEntries; i++) {
        DrivenPropertyEntry entry(this);
        entry.Load(d.stream, this);
        mDrivenPropEntries.push_back(entry);
    }
    if (d.rev > 0) {
        bool output;
        d >> output;
        mDebugOutput = output;
    }
    if (d.rev > 1) {
        String comment;
        d >> comment;
        mDebugComment = comment;
    }
END_LOADS

const char *FlowNode::FindPathName() {
    ObjectDir *dir = dynamic_cast<ObjectDir *>(this);
    if (dir) {
        return dir->Hmx::Object::FindPathName();
    } else {
        Flow *flow = GetOwnerFlow();
        return MakeString("%s:%s:%s", Name(), ClassName(), flow->FindPathName());
    }
}

#pragma endregion
#pragma region FlowNode

void FlowNode::SetParent(class FlowNode *new_parent, bool b) {
    if (mFlowParent != new_parent) {
        if (mFlowParent != nullptr) {
            mFlowParent->mChildNodes.remove(this);
        }
        mFlowParent = new_parent;
        if (new_parent != nullptr && b) {
            new_parent->mChildNodes.push_back(this);
        }
    }
}

bool FlowNode::Activate() {
    FLOW_LOG("Activating Children\n");
    unk58 = false;
    FOREACH (it, mChildNodes) {
        ActivateChild(*it);
        if (unk58)
            break;
    }
    return !mRunningNodes.empty();
}

void FlowNode::Deactivate(bool b1) {
    FLOW_LOG("Deactivated\n");
    FOREACH (it, mRunningNodes) {
        (*it)->Deactivate(b1);
    }
    mRunningNodes.clear();
}

void FlowNode::ChildFinished(FlowNode *node) {
    FLOW_LOG("Child Finished of class:%s\n", node->ClassName());
    mRunningNodes.remove(node);
    if (mRunningNodes.empty()) {
        FLOW_LOG("Releasing\n");
        if (mFlowParent)
            mFlowParent->ChildFinished(this);
    }
}

void FlowNode::RequestStop() {
    FLOW_LOG("RequestStop\n");
    unk58 = true;
    FOREACH (it, mRunningNodes) {
        (*it)->RequestStop();
    }
}

void FlowNode::RequestStopCancel() {
    FLOW_LOG("RequestStopCancel\n");
    unk58 = false;
    FOREACH (it, mRunningNodes) {
        (*it)->RequestStopCancel();
    }
}

Flow *FlowNode::GetOwnerFlow() {
    ObjectDir *dir = Dir();
    return dir ? static_cast<Flow *>(dir) : nullptr;
}

void FlowNode::MiloPreRun() {
    FOREACH (it, mChildNodes) {
        (*it)->MiloPreRun();
    }
}

void FlowNode::MoveIntoDir(ObjectDir *o1, ObjectDir *o2) {
    if (!Dir() || Dir() == o2) {
        String str("a");
        str[0] = (rand() % 25) + 'a';
        const char *name = NextName(MakeString("%s", str.c_str()), o1);
        if (o2) {
            while (streq(o2->Name(), name) || streq(o1->Name(), name)) {
                str[0] = (rand() % 25) + 'a';
                name = MakeString("%s%s", name, str.c_str());
            }
        }
        SetName(NextName(name, o1), o1);
        FOREACH (it, mChildNodes) {
            (*it)->MoveIntoDir(o1, o2);
        }
        FOREACH (it, mDrivenPropEntries) {
            FOREACH (op, it->MathOps()) {
                FlowPtr<Hmx::Object> &ptr = op->GetUnk18();
                if (ptr == o2) {
                    ptr = o1;
                }
            }
        }
    }
}

void FlowNode::UpdateIntensity() {
    FOREACH (it, mRunningNodes) {
        (*it)->UpdateIntensity();
    }
}

// FlowNode *FlowNode::DuplicateChild(FlowNode *) { return nullptr; }

void FlowNode::PushDrivenProperties() {
    sPushDrivenProperties = true;
    FOREACH (it, mDrivenPropEntries) {
        DataNode n;
        auto op = it->MathOps().begin();
        Hmx::Object *obj = op->GetUnk18();
        if (obj) {
            const DataNode *prop = obj->Property(op->RHS().Array(), false);
            if (prop) {
                n = *prop;
            } else {
                n = op->GetUnk0();
            }
        } else {
            n = op->GetUnk0();
        }
        if (op != it->MathOps().end()) {
            if (n.CompatibleType(kDataFloat)) {
                float sum = n.LiteralFloat();
                while (op != it->MathOps().end()) {
                    sum += op->Apply(sum);
                }
                n = sum;
            }
            const DataNode *prop = Property(it->Node().Array());
            if (prop->Type() == n.Type()) {
                SetProperty(it->Node().Array(), n);
            } else if (n.Type() == kDataFloat || n.Type() == kDataInt) {
                if (prop->Type() == kDataFloat) {
                    SetProperty(it->Node().Array(), n);
                } else {
                    SetProperty(it->Node().Array(), Round(n.LiteralFloat()));
                }
            }
        } else {
            SetProperty(it->Node().Array(), n);
        }
    }
    sPushDrivenProperties = false;
}

void FlowNode::ActivateChild(FlowNode *child) {
    mRunningNodes.push_back(child);
    if (!child->Activate()) {
        FLOW_LOG(
            "Activated Child %s, which ran in full immediately.\n", child->ClassName()
        );
        mRunningNodes.remove(child);
    }
}

bool FlowNode::HasRunningNode(FlowNode *node) {
    return mRunningNodes.find(node) != mRunningNodes.end();
}

DrivenPropertyEntry *FlowNode::GetDrivenEntry(Symbol s) {
    DataArrayPtr ptr(new DataArray(1));
    ptr->Node(0) = s;
    return GetDrivenEntry(ptr);
}

DrivenPropertyEntry *FlowNode::GetDrivenEntry(DataArray *a) {
    FOREACH (it, mDrivenPropEntries) {
        if (it->Node().Type() == kDataArray) {
            DataArray *curArr = it->Node().Array();
            if (curArr->Size() == a->Size()) {
                bool b1 = true;
                for (int i = 0; i < curArr->Size(); i++) {
                    if (curArr->Node(i) != a->Node(i)) {
                        b1 = false;
                    }
                }
                if (b1) {
                    return it;
                }
            }
        }
    }
    return nullptr;
}

Flow *FlowNode::GetTopFlow() {
    Flow *flow = GetOwnerFlow();
    if (flow) {
        for (; GetOwnerFlow() && GetOwnerFlow() != flow; flow = flow->GetOwnerFlow())
            ;
    }
    return flow;
}

void FlowNode::ActivateLabel(FlowLabel *label) {
    FLOW_LOG("Activating Label:%s\n", label->Label());
    unk58 = false;
    mRunningNodes.push_back(label);
    if (!label->Activate(this)) {
        mRunningNodes.remove(label);
    }
}
