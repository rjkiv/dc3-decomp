#pragma once
#include "synth/MicManagerInterface.h"
#include <vector>

class MicClientMapper {
public:
    struct MicMappingData {
        MicMappingData() : cClientID(0, -1), mMicID(-1), bLocked(false) {}
        MicMappingData(const MicMappingData &d)
            : cClientID(d.cClientID), mMicID(d.mMicID), bLocked(d.bLocked) {}

        MicClientID cClientID;
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
    MicManagerInterface *GetMicManager() { return mMicManager; }

    static const int kNumberOfMicClients;

private:
    void LockMicID(int);
    void UnlockMicID(int);
    void RefreshPlayerMapping();
    void RefreshMics();

    MicManagerInterface *mMicManager; // 0x0
    std::vector<MicMappingData> mMappingData; // 0x4
    std::vector<PlayerMappingData> mPlayers; // 0xc
    int mNumberOfPlayers; // 0x14
};
