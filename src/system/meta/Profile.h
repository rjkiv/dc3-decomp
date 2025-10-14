#pragma once
#include "meta/FixedSizeSaveable.h"
#include "obj/Object.h"
#include "os/User.h"

enum ProfileSaveState {
    kMetaProfileUnloaded = 0,
    kMetaProfileLoaded = 1,
    kMetaProfileError = 2,
    kMetaProfileDelete = 3,
    kMetaProfileUnchanged = -1
};

class Profile : public FixedSizeSaveable, public virtual Hmx::Object {
protected:
    bool mDirty; // 0xc
    int mPadNum; // 0x10
    ProfileSaveState mState; // 0x14
public:
    // Hmx::Object
    virtual ~Profile();
    virtual DataNode Handle(DataArray *, bool);
    virtual void PreLoad() {}

    // FixedSizeSaveable
    virtual bool IsUnsaved() const;
    virtual void SaveLoadComplete(ProfileSaveState);
    virtual void SaveFixed(FixedSizeSaveableStream &) const = 0;

    virtual void LoadFixed(FixedSizeSaveableStream &, int) = 0;
    virtual bool HasCheated() const { return false; }
    virtual bool HasSomethingToUpload();
    virtual void DeleteAll();

    Profile(int);
    bool IsAutosaveEnabled() const;
    bool HasValidSaveData() const;
    ProfileSaveState GetSaveState() const;
    void SetSaveState(ProfileSaveState);
    void MakeDirty();

    int GetPadNum() const;
    const char *GetName() const;
};

#include "obj/Msg.h"

DECLARE_MESSAGE(ProfileSwappedMsg, "profile_swapped")
ProfileSwappedMsg(LocalUser *u1, LocalUser *u2) : Message(Type(), u1, u2) {}
LocalUser *GetUser1() const;
LocalUser *GetUser2() const;
END_MESSAGE
