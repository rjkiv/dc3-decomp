#include "meta/MemcardMgr.h"
#include "os/Debug.h"

MemcardMgr TheMemcardMgr;

void MemcardMgr::SetProfileSaveBuffer(void *v, int i) {
    unk34 = v;
    unk38 = i;
}

void MemcardMgr::SaveLoadProfileComplete(Profile *pProfile, int state) {
    MILO_ASSERT(pProfile, 0x1B);
    pProfile->SaveLoadComplete((ProfileSaveState)state);
}

void MemcardMgr::SaveLoadAllComplete() {
    static SaveLoadAllCompleteMsg msg;
    Hmx::Object::Handle(msg, false);
}
