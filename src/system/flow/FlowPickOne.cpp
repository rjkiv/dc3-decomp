#include "flow/FlowPickOne.h"
#include "flow/FlowNode.h"
#include "flow/DrivenPropertyEntry.h"
#include "obj/Object.h"
#include "math/Rand.h"
#include "os/Debug.h"
#include "utl/Std.h"

FlowPickOne::FlowPickOne()
    : unk5c(this), mChoiceType(kChoiceRandom), mIndex(0), mChance(1) {}
FlowPickOne::~FlowPickOne() {}

BEGIN_HANDLERS(FlowPickOne)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowPickOne)
    SYNC_PROP_MODIFY(choice_type, (int &)mChoiceType, OnChoiceTypeChanged())
    SYNC_PROP(index, mIndex)
    SYNC_PROP(chance, mChance)
    SYNC_SUPERCLASS(FlowNode)
END_PROPSYNCS

BEGIN_SAVES(FlowPickOne)
    SAVE_REVS(1, 0)
    SAVE_SUPERCLASS(FlowNode)
    bs << mChoiceType;
    bs << mChance;
END_SAVES

BEGIN_COPYS(FlowPickOne)
    COPY_SUPERCLASS(FlowNode)
    CREATE_COPY(FlowPickOne)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mChoiceType)
        COPY_MEMBER(mChance)
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(1, 0)

BEGIN_LOADS(FlowPickOne)
    LOAD_REVS(bs)
    ASSERT_REVS(1, 0)
    LOAD_SUPERCLASS(FlowNode)
    d >> (int &)mChoiceType;
    if (d.rev > 0) {
        d >> mChance;
    }
END_LOADS

bool FlowPickOne::Activate() {
    FLOW_LOG("Activate\n");
    mRequestingStop = false;
    PushDrivenProperties();
    if (mChance != 1) {
        if (rand() % 100 > mChance * 100) {
            return false;
        }
    }
    if (mChildNodes.empty()) {
        return false;
    }

    switch (mChoiceType) {
    case kChoiceOrdered: {
        if (!ValidIndex()) {
            mIndex = 0;
        }
        ActivateChild(mChildNodes[mIndex]);
        mIndex++;
        break;
    }
    case kChoiceRandom: {
        mIndex = RandomInt(0, mChildNodes.size());
        ActivateChild(mChildNodes[mIndex]);
        break;
    }
    case kChoiceRandomNoRepeat: {
        if (mChildNodes.size() > 1) {
            int randInt;
            while (randInt = RandomInt(0, mChildNodes.size()), randInt == mIndex)
                ;
            mIndex = randInt;
        } else {
            mIndex = 0;
        }
        ActivateChild(mChildNodes[mIndex]);
        break;
    }
    case kChoiceRandomJukeBox: {
        int numNodes = mChildNodes.size();
        if (numNodes > 1) {
            if (!ValidIndex()) {
                FlowNode *first = unk5c.front();
                unk5c.clear();
                std::vector<FlowNode *> nodes;
                FOREACH (it, mChildNodes) {
                    nodes.push_back(*it);
                }
                std::random_shuffle(nodes.begin(), nodes.end());
                FOREACH_REVERSE(it, nodes) { unk5c.push_back(*it); }
                mIndex = 0;
                if (first == unk5c.front()) {
                    mIndex = 1;
                }
            }
            ActivateChild(mChildNodes[0]);
            mIndex++;
        }
        if (numNodes == 1) {
            ActivateChild(mChildNodes[0]);
        }
        break;
    }
    case kChoiceUseIndex: {
        mIndex = mIndex % mChildNodes.size();
        ActivateChild(*NextItr(mChildNodes.begin(), mIndex));
        break;
    }
    default: {
        MILO_NOTIFY_ONCE("FlowPickOne: bad picking type");
        break;
    }
    }

    return !mRunningNodes.empty();
}

void FlowPickOne::OnChoiceTypeChanged() {
    if (mChoiceType != kChoiceUseIndex) {
        FOREACH (it, mDrivenPropEntries) {
            if (it->Node().Array()->Sym(0) == "index") {
                mDrivenPropEntries.erase(it);
                return;
            }
        }
    }
}
