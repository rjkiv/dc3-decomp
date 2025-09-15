#include "world/ThreeDSoundManager.h"
#include "os/Debug.h"
#include "synth/ThreeDSound.h"
#include "world/Dir.h"

ThreeDSoundManager::ThreeDSoundManager(WorldDir *dir)
    : mParent(dir), unk4(dir), unk58(dir), unk6c(0), unk70(1) {}

ThreeDSoundManager::~ThreeDSoundManager() {}

void ThreeDSoundManager::SyncObjects() {
    ObjPtrList<ThreeDSound> sounds(mParent);
    HarvestSounds(mParent, sounds);
    for (ObjPtrList<ThreeDSound>::iterator it = sounds.begin(); it != sounds.end();
         ++it) {
        unk4.remove(*it);
    }
    for (ObjPtrList<ThreeDSound>::iterator it = unk4.begin(); it != unk4.end(); ++it) {
        (*it)->Stop(nullptr, false);
    }
    unk4 = sounds;
}

void ThreeDSoundManager::HarvestSounds(ObjectDir *dir, ObjPtrList<ThreeDSound> &sounds) {
    MILO_ASSERT(dir, 0x31);
    for (ObjDirItr<ThreeDSound> it(dir, true); it != nullptr; ++it) {
        sounds.push_back(&*it);
        MILO_NOTIFY(
            "Warning, found ThreeDSound object %s in %s!", it->Name(), PathName(dir)
        );
    }
}
