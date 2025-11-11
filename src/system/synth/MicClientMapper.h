#pragma once
#include "synth/MicManagerInterface.h"
#include <vector>

class MicClientMapper {
public:
    struct MicMappingData {
        MicMappingData() : unk0(0), unk4(-1), mMicID(-1), bLocked(false) {}

        int unk0;
        int unk4;
        int mMicID; // 0x8
        bool bLocked; // 0xc
    };

    struct PlayerMappingData {
        PlayerMappingData() : iActualMicID(-1), iPreferredMicID(-1) {}
        int iActualMicID; // 0x0
        int iPreferredMicID; // 0x4
    };

    MicClientMapper();
    ~MicClientMapper();
    void HandleMicsChanged();
    int GetMicIDForClientID(const MicClientID &) const;
    int GetMicIDForPlayerID(int) const;
    int GetPlayerIDForMicID(int) const;
    bool HasMicID(int) const;
    void GetAllConnectedMics(std::vector<int> &) const;
    void SetNumberOfPlayers(int);
    void UnlockAllMicIDs();
    bool IsMicIDLocked(int) const;
    bool GetFirstUnlockedMicID(int &) const;
    MicManagerInterface *GetMicMgrInterface() const { return mMicManager; }

private:
    void LockMicID(int);
    void UnlockMicID(int);
    void RefreshPlayerMapping();
    void RefreshMics();

protected:
    MicManagerInterface *mMicManager; // 0x0
    std::vector<MicMappingData> mMappingData; // 0x4
    std::vector<PlayerMappingData> mPlayers; // 0xc
    int mNumPlayers; // 0x14
};
