#pragma once
#include "game/HamUser.h"
#include "meta/Profile.h"
#include "meta_ham/SongStatusMgr.h"

class HamProfile : public Profile {
public:
    HamProfile(int);
    virtual ~HamProfile();
    // Hmx::Object
    virtual DataNode Handle(DataArray *, bool);
    // FixedSizeSaveable
    virtual void SaveFixed(FixedSizeSaveableStream &) const;
    virtual void LoadFixed(FixedSizeSaveableStream &, int);
    virtual bool HasCheated() const;
    virtual bool IsUnsaved() const;
    virtual void SaveLoadComplete(ProfileSaveState);
    virtual bool HasSomethingToUpload();
    virtual void DeleteAll();

    static int SaveSize(int);

    void CheckForIconManUnlock();
    void CheckForNinjaUnlock();
    void SetFitnessMode(bool);
    HamUser *GetHamUser() const;
    SongStatusMgr *GetSongStatusMgr() const;

protected:
};
