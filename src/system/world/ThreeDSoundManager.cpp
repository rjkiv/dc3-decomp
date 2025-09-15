#include "world/ThreeDSoundManager.h"
#include "os/Debug.h"
#include "synth/ThreeDSound.h"
#include "world/Dir.h"

ThreeDSoundManager::ThreeDSoundManager(WorldDir *dir)
    : mParent(dir), mSounds(dir), mListener(dir), unk6c(0), mDopplerPower(1) {}

ThreeDSoundManager::~ThreeDSoundManager() {}

void ThreeDSoundManager::SyncObjects() {
    ObjPtrList<ThreeDSound> sounds(mParent);
    HarvestSounds(mParent, sounds);
    for (ObjPtrList<ThreeDSound>::iterator it = sounds.begin(); it != sounds.end();
         ++it) {
        mSounds.remove(*it);
    }
    for (ObjPtrList<ThreeDSound>::iterator it = mSounds.begin(); it != mSounds.end();
         ++it) {
        (*it)->Stop(nullptr, false);
    }
    mSounds = sounds;
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
