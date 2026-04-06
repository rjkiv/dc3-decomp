#pragma once
#include "gesture/GestureMgr.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/PlatformMgr.h"
#include "utl/Str.h"

DECLARE_MESSAGE(SkeletonIdentifiedMsg, "skeleton_identified")
int GetVal2() const { return mData->Int(2); }
int GetIndex() const { return mData->Int(3); }
END_MESSAGE

enum IdentityStatus {
    kIdentityStatus_None = 0,
    kIdentityStatus_Identifying = 1,
    kIdentityStatus_Enrolling = 2,
    kIdentityStatus_Correcting = 3,
    kIdentityStatus_WaitingForSignIn = 4,
};

class SkeletonIdentifier : public Hmx::Object {
public:
    enum {
        user_max_count = 4
    };
    class EnrolledPlayer {
    public:
        EnrolledPlayer() : mPadNum(-1) {}
        bool UpdatePlayerBinding();

        int mPadNum; // 0x0 - padnum/user index
        String unk4; // 0x4 - name
        int mEnrollmentIndex; // 0xc
    };

    SkeletonIdentifier();
    // Hmx::Object
    virtual ~SkeletonIdentifier();
    virtual DataNode Handle(DataArray *, bool);

    void Init();
    void Poll();
    String GetPlayerName(int idx) const { return unk48[idx].unk4; }
    int GetPlayerPadNum(int idx) const { return unk48[idx].mPadNum; }
    IdentityStatus GetIdentityStatus(int);
    void CorrectIdentity(int);
    bool IsAssociatedWithProfile(int) const;
    void SetUpInitialProfiles();
    void UpdateIdentityStatus();
    void UpdateEnrolledPlayers();
    void DrawDebug();
    IdentityStatus GetIDStatus() { return mIdentityStatus; }

private:
    void SetEnrolling();
    void RequestIdentity();
    void SearchForIdentity();
    void NotifyOfRecognition(int) const;

    DataNode OnMsg(const SigninChangedMsg &);
    DataNode OnMsg(const SkeletonEnrollmentChangedMsg &);
    DataNode OnMsg(const SkeletonIdentifiedMsg &);

    IdentityStatus mIdentityStatus; // 0x2c
    int unk30; // 0x30 - skeleton idx
    int unk34; // 0x34 - player from skeleton
    int unk38;
    int unk3c;
    int unk40; // 0x40 - skeleton tracking id
    int unk44;
    EnrolledPlayer unk48[8];
    bool mDrawDebug; // 0xc8
};

String EnrollmentIndexString(int idx);

extern SkeletonIdentifier *TheSkeletonIdentifier;
