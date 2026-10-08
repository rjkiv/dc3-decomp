#include "char/CharPollGroup.h"
#include "char/CharWeightable.h"
#include "obj/Object.h"

CharPollGroup::CharPollGroup() : mPolls(this), mChangedBy(this), mChanges(this) {}

CharPollGroup::~CharPollGroup() {}

BEGIN_HANDLERS(CharPollGroup)
    HANDLE_ACTION(sort_polls, SortPolls())
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

BEGIN_PROPSYNCS(CharPollGroup)
    SYNC_PROP(polls, mPolls)
    SYNC_PROP(changed_by, mChangedBy)
    SYNC_PROP(changes, mChanges)
    SYNC_SUPERCLASS(CharWeightable)
    SYNC_SUPERCLASS(Hmx::Object)
END_PROPSYNCS

BEGIN_SAVES(CharPollGroup)
    SAVE_REVS(3, 0)
    SAVE_SUPERCLASS(Hmx::Object)
    SAVE_SUPERCLASS(CharWeightable)
    bs << mPolls;
    bs << mChangedBy;
    bs << mChanges;
END_SAVES

BEGIN_COPYS(CharPollGroup)
    COPY_SUPERCLASS(Hmx : Object)
    COPY_SUPERCLASS(CharWeightable)
    CREATE_COPY(CharPollGroup)
    BEGIN_COPYING_MEMBERS
        if (ty == kCopyFromMax) {
            FOREACH (it, c->mPolls) {
                if (mPolls.find(*it).Null()) {
                    mPolls.push_back(*it);
                }
            }
        } else {
            COPY_MEMBER(mPolls)
            COPY_MEMBER(mChangedBy)
            COPY_MEMBER(mChanges)
        }
    END_COPYING_MEMBERS
END_COPYS

INIT_REVS(3, 0)

BEGIN_LOADS(CharPollGroup)
    LOAD_REVS(bs);
    ASSERT_REVS(3, 0);
    LOAD_SUPERCLASS(Hmx::Object)
    if (d.rev > 2) {
        LOAD_SUPERCLASS(CharWeightable)
    }
    d >> mPolls;
    if (d.rev > 1) {
        d >> mChangedBy;
        d >> mChanges;
    }
END_LOADS

void CharPollGroup::Poll() {
    if (Weight()) {
        FOREACH (it, mPolls) {
            (*it)->Poll();
        }
    }
}

void CharPollGroup::Enter() {
    FOREACH (it, mPolls) {
        (*it)->Enter();
    }
}

void CharPollGroup::Exit() {
    FOREACH (it, mPolls) {
        (*it)->Exit();
    }
}

void CharPollGroup::ListPollChildren(std::list<RndPollable *> &l) const {
    FOREACH (it, mPolls) {
        l.push_back(*it);
    }
}

void CharPollGroup::PollDeps(
    std::list<Hmx::Object *> &changedBy, std::list<Hmx::Object *> &change
) {
    if (mChangedBy || mChanges) {
        changedBy.push_back(mChangedBy);
        change.push_back(mChanges);
    } else {
        FOREACH (it, mPolls) {
            (*it)->PollDeps(changedBy, change);
        }
    }
}

void CharPollGroup::SortPolls() { // incomplete
    CharPollableSorter sorter;
    std::vector<RndPollable *> polls;
    polls.reserve(mPolls.size());
    FOREACH (it, mPolls) {
        polls.push_back(*it);
    }
    sorter.Sort(polls);
    mPolls.clear();
    for (int i = 0; i < polls.size(); i++) {
        mPolls.push_back(dynamic_cast<CharPollable *>(polls[i]));
    }
}
