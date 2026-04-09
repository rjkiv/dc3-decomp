#pragma once
#include "flow/PropertyEventProvider.h"
#include "gesture/BaseSkeleton.h"
#include "gesture/Skeleton.h"
#include "hamobj/Difficulty.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "utl/Str.h"
#include "utl/Symbol.h"

class HamPlayerData : public Hmx::Object {
public:
    HamPlayerData(int);
    // Hmx::Object
    virtual ~HamPlayerData() {}
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);

    void SetCrew(Symbol);
    void SetDifficulty(Difficulty);
    void SetPlaying(bool);
    bool IsPlaying() const;
    void SetCharacterOutfit(Symbol);
    void SetCharacter(Symbol);
    bool SetAssociatedPadNum(int, String);
    float TrackingAgeSeconds() const;
    const Skeleton *GetSkeleton() const;
    const Skeleton *GetSkeleton(const Skeleton *const (&)[6]) const;
    void SetPreferredOutfit(Symbol);
    void SetOutfit(Symbol);
    Symbol CharacterOutfit(Symbol) const;
    String GetPlayerName() const;
    SkeletonSide Side() const;
    void SetUsingFitness(bool);
    Symbol GetPreferredOutfit() const;

    Difficulty GetDifficulty() { return mDifficulty; }
    PropertyEventProvider *Provider() const { return mProvider; }
    bool InFreestyle() const { return mSkeletonTrackingID >= 0; }
    Symbol Crew() const { return mCrew; }
    Symbol Char() const { return mChar; }
    Symbol Outfit() const { return mOutfit; }
    Symbol Autoplay() const { return mAutoplay; }
    int GetSkeletonTrackingID() const { return mSkeletonTrackingID; }
    void AssignSkeleton(int id) { SetSkeletonTrackingID(id); }
    int PadNum() const { return mPadNum; }
    bool IsAutoplaying() const { return !mAutoplay.Null(); }
    void SetUnk48(Symbol s) { unk48 = s; }
    Symbol Unk48() const { return unk48; }
    void SetAutoplay(Symbol s) { mAutoplay = s; }
    const String &Unk2c() const { return unk2c; }
    void SetUnk2C(String &s) { unk2c = s; }
    int Unk40() const { return unk40; }
    std::vector<String> &GetUnk34() { return unk34; }

private:
    void SetSkeletonTrackingID(int);

protected:
    String unk2c; // 0x2c - current dancer?
    std::vector<String> unk34; // 0x34 - dancers?
    int unk40; // 0x40 - character index in the big char array?
    Symbol mChar; // 0x44
    Symbol unk48; // 0x48
    Symbol mPreferredOutfit; // 0x4c
    Symbol mOutfit; // 0x50
    Symbol mCrew; // 0x54
    Difficulty mDifficulty; // 0x58
    float unk5c; // 0x5c
    int mSkeletonTrackingID; // 0x60
    Symbol mAutoplay; // 0x64
    ObjPtr<PropertyEventProvider> mProvider; // 0x68
    int mPadNum; // 0x7c
    int mSameRatingCount; // 0x80
    int mLastRatingIdx; // 0x84
};
