#pragma once

class MicClientID {
public:
    MicClientID(int i = -1, int j = -1) : mID(i), mPlayerNum(j) {}
    MicClientID(const MicClientID &id) : mID(id.mID), mPlayerNum(id.mPlayerNum) {}

    MicClientID &operator=(const MicClientID &id) {
        if (this != &id) {
            mID = id.mID;
            mPlayerNum = id.mPlayerNum;
        }
        return *this;
    }

    static const int kNoID;
    static const int kNoPlayerNum;

    int GetID() const { return mID; }
    int GetPlayerNum() const { return mPlayerNum; }

    void SetID(int id) { mID = id; }
    void SetPlayerNum(int num) { mPlayerNum = num; }

private:
    int mID; // 0x0
    int mPlayerNum; // 0x4
};

class MicManagerInterface {
public:
    MicManagerInterface() {}
    virtual ~MicManagerInterface() {}
    virtual void HandleMicsChanged() = 0;
    virtual void SetPlayback(bool) = 0;
    virtual float GetEnergyForMic(const MicClientID &) = 0;
};
