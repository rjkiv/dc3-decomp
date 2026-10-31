#include "flow/FlowPickOne.h"
#include "flow/FlowNode.h"
#include "flow/DrivenPropertyEntry.h"
#include "obj/Object.h"
#include "math/Rand.h"
#include "os/Debug.h"
#include "utl/Std.h"

FlowPickOne::FlowPickOne()
    : mJukeList(this), mChoiceType(kChoiceRandom), mChildIdx(0), mChance(1) {}
FlowPickOne::~FlowPickOne() {}

BEGIN_HANDLERS(FlowPickOne)
    HANDLE_SUPERCLASS(FlowNode)
END_HANDLERS

BEGIN_PROPSYNCS(FlowPickOne)
    SYNC_PROP_MODIFY(choice_type, (int &)mChoiceType, OnChoiceTypeChanged())
    SYNC_PROP(index, mChildIdx)
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
        int idx = mChildIdx;
        bool valid = idx >= 0 && idx < mChildNodes.size();
        if (!valid) {
            mChildIdx = 0;
        }
        ActivateChild(mChildNodes[mChildIdx]);
        mChildIdx++;
        break;
    }
    case kChoiceRandom: {
        mChildIdx = RandomInt(0, mChildNodes.size());
        ActivateChild(mChildNodes[mChildIdx]);
        break;
    }
    case kChoiceRandomNoRepeat: {
        if (mChildNodes.size() > 1) {
            int randInt;
            do {
                randInt = RandomInt(0, mChildNodes.size());
            } while (randInt == mChildIdx);
            mChildIdx = randInt;
        } else {
            mChildIdx = 0;
        }
        ActivateChild(mChildNodes[mChildIdx]);
        break;
    }
    case kChoiceRandomJukeBox: {
        int numNodes = mChildNodes.size();
        if (numNodes > 1) {
            bool valid = mChildIdx >= 0 && mChildIdx < mJukeList.size();
            if (!valid) {
                FlowNode *last = mJukeList.empty() ? nullptr : mJukeList.back();

                mJukeList.clear();
                std::vector<FlowNode *> nodes;
                FOREACH (it, mChildNodes) {
                    nodes.push_back(*it);
                }
                std::random_shuffle(nodes.begin(), nodes.end());
                while (nodes.size() > 0) {
                    mJukeList.push_back(nodes.back());
                    nodes.pop_back();
                }
                mChildIdx = 0;
                if (last == mJukeList[0]) {
                    mChildIdx = 1;
                }
            }
            ActivateChild(mJukeList[mChildIdx]);
            mChildIdx++;
        } else if (numNodes == 1) {
            ActivateChild(mChildNodes[0]);
        }
        break;
    }
    case kChoiceUseIndex: {
        mChildIdx = mChildIdx % mChildNodes.size();
        auto it = mChildNodes.begin();
        for (int i = 0; i < mChildIdx; i++, ++it)
            ;
        ActivateChild(*it);
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
        FOREACH (it, mDrivenProperties) {
            if (it->mDrivenProperty.Array()->Sym(0) == "index") {
                mDrivenProperties.erase(it);
                return;
            }
        }
    }
}
