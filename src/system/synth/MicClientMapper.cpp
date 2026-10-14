#include "synth/MicClientMapper.h"
#include "os/Debug.h"
#include "synth/Synth.h"

MicClientMapper::MicClientMapper() : mMicManager(0), mNumberOfPlayers(2) {
    for (int i = 0; i < 4; i++) {
        MicMappingData data;
        data.cClientID.SetID(i);
        mMappingData.push_back(data);
    }
    for (int i = 0; i < mNumberOfPlayers; i++) {
        mPlayers.push_back(PlayerMappingData());
    }
}

MicClientMapper::~MicClientMapper() {}

void MicClientMapper::LockMicID(int micID) {
    FOREACH (it, mMappingData) {
        if (it->mMicID == micID) {
            if (it->bLocked) {
                MILO_NOTIFY(
                    "MicClientMapper::UnlockMicID - Trying to lock an already locked mic!"
                );
            }
            it->bLocked = true;
        }
    }
}

void MicClientMapper::UnlockMicID(int micID) {
    FOREACH (it, mMappingData) {
        if (it->mMicID == micID) {
            if (!it->bLocked) {
                MILO_NOTIFY(
                    "MicClientMapper::UnlockMicID - Trying to unlock an already unlocked mic!"
                );
            }
            it->bLocked = false;
        }
    }
}

bool MicClientMapper::HasMicID(int micID) const {
    FOREACH (it, mMappingData) {
        if (it->mMicID == micID)
            return true;
    }
    return false;
}

bool MicClientMapper::IsMicIDLocked(int micID) const {
    FOREACH (it, mMappingData) {
        if (it->mMicID == micID) {
            return it->bLocked;
        }
    }
    return false;
}

bool MicClientMapper::GetFirstUnlockedMicID(int &micID) const {
    FOREACH (it, mMappingData) {
        if (it->mMicID != -1 && !it->bLocked) {
            micID = it->mMicID;
            return true;
        }
    }
    return false;
}

void MicClientMapper::RefreshPlayerMapping() {
    FOREACH (iter, mPlayers) {
        if (iter->iPreferredMicID != -1 && HasMicID(iter->iPreferredMicID)) {
            if (iter->iPreferredMicID == iter->iActualMicID) {
                MILO_ASSERT(IsMicIDLocked( iter->iPreferredMicID ), 0x1A2);
            } else {
                if (!IsMicIDLocked(iter->iPreferredMicID)) {
                    if (iter->iActualMicID != -1)
                        UnlockMicID(iter->iActualMicID);
                    iter->iActualMicID = iter->iPreferredMicID;
                    LockMicID(iter->iPreferredMicID);
                }
            }
        }
    }
    FOREACH (iter, mPlayers) {
        if (iter->iActualMicID != -1 && HasMicID(iter->iActualMicID)) {
            MILO_ASSERT(IsMicIDLocked( iter->iActualMicID ), 0x1C0);
        } else {
            int firstUnlockedID = -1;
            if (GetFirstUnlockedMicID(firstUnlockedID)) {
                iter->iActualMicID = firstUnlockedID;
                LockMicID(firstUnlockedID);
            } else
                iter->iActualMicID = -1;
        }
    }
}

void MicClientMapper::RefreshMics() {
    FOREACH (it, mMappingData) {
        if (it->cClientID.GetID() != -1 && it->mMicID != -1
            && !TheSynth->IsMicConnected(it->mMicID)) {
            TheSynth->ReleaseMic(it->mMicID);
            it->mMicID = -1;
            it->bLocked = false;
        }
    }
    FOREACH (iter, mMappingData) {
        if (iter->cClientID.GetID() != -1 && iter->mMicID == -1) {
            iter->mMicID = TheSynth->GetNextAvailableMicID();
            MILO_ASSERT(iter->bLocked == false, 0x161);
            if (iter->mMicID != -1) {
                TheSynth->CaptureMic(iter->mMicID);
            }
        }
    }
    RefreshPlayerMapping();
}

void MicClientMapper::HandleMicsChanged() {
    RefreshMics();
    if (mMicManager)
        mMicManager->HandleMicsChanged();
}

int MicClientMapper::GetMicIDForPlayerID(int playerID) const {
    if (playerID >= mNumberOfPlayers)
        return -1;
    else
        return mPlayers[playerID].iActualMicID;
}

int MicClientMapper::GetMicIDForClientID(const MicClientID &clientID) const {
    if (clientID.GetPlayerNum() == -1) {
        FOREACH (it, mMappingData) {
            if (it->cClientID.GetID() == clientID.GetID())
                return it->mMicID;
        }
        return -1;
    } else
        return GetMicIDForPlayerID(clientID.GetPlayerNum());
}
