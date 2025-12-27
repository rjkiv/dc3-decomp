#pragma once
#include "gesture/GestureMgr.h"
#include "obj/Data.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/PlatformMgr.h"
#include "utl/Str.h"

DECLARE_MESSAGE(SkeletonIdentifiedMsg, "skeleton_identified")
END_MESSAGE

enum IdentityStatus {
    kIdentityStatus_0,
    kIdentityStatus_Identifying,
    kIdentityStatus_2
};

class SkeletonIdentifier : public Hmx::Object {
public:
    class EnrolledPlayer {
    public:
        EnrolledPlayer();
        bool UpdatePlayerBinding();

        int unk0;
        String unk4;
        int mEnrollmentIndex; // 0xc
    };

    // Hmx::Object
    virtual ~SkeletonIdentifier();
    virtual DataNode Handle(DataArray *, bool);

    SkeletonIdentifier();
    void Init();
    void Poll();
    String GetPlayerName(int) const;
    IdentityStatus GetIdentityStatus(int);
    void CorrectIdentity(int);
    bool IsAssociatedWithProfile(int) const;
    void SetUpInitialProfiles();
    void UpdateIdentityStatus();
    void UpdateEnrolledPlayers();
    void DrawDebug();

protected:
    IdentityStatus mIdentityStatus; // 0x2c
    int unk30;
    int unk34;
    int unk38;
    int unk3c;
    int unk40;
    int unk44;
    EnrolledPlayer unk48[8];
    bool unkc8;

private:
    void SetEnrolling();
    void RequestIdentity();
    void SearchForIdentity();
    void NotifyOfRecognition(int) const;
    DataNode OnMsg(SigninChangedMsg const &);
    DataNode OnMsg(SkeletonEnrollmentChangedMsg const &);
    DataNode OnMsg(SkeletonIdentifiedMsg const &);
};

String EnrollmentIndexString(int idx);

extern SkeletonIdentifier *TheSkeletonIdentifier;
