#pragma once
#include "char/Character.h"
#include "hamobj/HamCharacter.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "rndobj/Overlay.h"
#include "utl/MemMgr.h"

/** "Manager class that clothes characters and loads animations
    under different conditions" */
class HamWardrobe : public virtual Hmx::Object {
public:
    // Hmx::Object
    virtual ~HamWardrobe();
    OBJ_CLASSNAME(HamWardrobe);
    OBJ_SET_TYPE(HamWardrobe);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x2B)
    NEW_OBJ(HamWardrobe)

    HamCharacter *GetCharacter(int idx) const;
    HamCharacter *GetBackup(int idx) const;
    void PlayCrowdAnimation(Symbol, int, bool);
    void EndCrowdOverride();
    void ForceCrowdAnimationStart(Symbol);
    void ForceCrowdAnimationEnd();
    void SetBackupOverrideOutfits(Symbol outfit1, Symbol outfit2);
    void LoadCharacters(
        Symbol outfit1,
        Symbol outfit2,
        Symbol crew1,
        Symbol crew2,
        HamBackupDancers dancers,
        Symbol tempo,
        Symbol venue,
        bool async
    );
    Symbol GetBackupOutfitOverride(int idx);
    bool AllCharsLoaded();
    void ClearCrowdClips();
    void ClearCrowd();
    void UpdateOverlay();
    void SetDir(ObjectDir *);

protected:
    HamWardrobe();

    Symbol GetCrewChar(Symbol, int);
    HamCharacter *LoadMainCharacter(int index, Symbol outfit, bool async);
    void LoadCrowdClips(Symbol tempo, Symbol venue, bool async);
    void SyncInterestObjects(ObjectDir *);

    DataNode OnSetVenue(DataArray *);
    DataNode OnAddCrowd(DataArray *);
    DataNode OnLoadCharacters(DataArray *);

    ObjPtrList<Character> mCrowdMembers; // 0x4
    ObjPtrVec<HamCharacter> mMainCharacters; // 0x18
    /** How fast the crowd moves. */
    Symbol mTempo; // 0x34
    bool unk38; // 0x38 - active crowd override?
    Symbol mCrowdForceState; // 0x3c
    int unk40; // 0x40 - some sort of flags
    Symbol unk44; // 0x44 - current crowd state?
    Symbol mCharOverrideOutfit; // 0x48
    Symbol mBackupOverrideOutfits[2]; // 0x4c
    RndOverlay *mOverlay; // 0x54
};

extern HamWardrobe *TheHamWardrobe;
