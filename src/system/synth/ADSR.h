#pragma once
#include "obj/Object.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

class ADSRImpl {
    friend class ADSR;

public:
    enum AttackMode {
        kAttackLinear = 0,
        kAttackExp = 1,
    };
    enum SustainMode {
        kSustainLinInc = 0,
        kSustainLinDec = 2,
        kSustainExpInc = 4,
        kSustainExpDec = 6,
    };
    enum ReleaseMode {
        kReleaseLinear = 0,
        kReleaseExp = 1,
    };

    ADSRImpl();
    float GetAttackRate() const;
    float GetReleaseRate() const;
    void Save(BinStream &) const;
    void Load(BinStream &, ADSR *);

private:
    /** "Duration of attack in seconds" */
    float mAttackRate; // 0x0
    /** "Duration of decay in seconds" */
    float mDecayRate; // 0x4
    /** "Duration of sustain in seconds" */
    float mSustainRate; // 0x8
    /** "Duration of release in seconds" */
    float mReleaseRate; // 0xc
    /** "Level of sustain volume (0-1)" */
    float mSustainLevel; // 0x10
    /** "Attack mode" */
    AttackMode mAttackMode; // 0x14
    /** "Sustain mode" */
    SustainMode mSustainMode; // 0x18
    /** "Release mode" */
    ReleaseMode mReleaseMode; // 0x1c
    bool mSynced; // 0x20
};

/** "Attack, decay, sustain, and release. Envelope settings to modify sounds." */
class ADSR : public Hmx::Object {
public:
    virtual ~ADSR() {}
    OBJ_CLASSNAME(ADSR);
    OBJ_SET_TYPE(ADSR);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x61);
    NEW_OBJ(ADSR)

protected:
    ADSR();

    ADSRImpl mADSR; // 0x2c
};

BinStream &operator<<(BinStream &, const ADSRImpl &);
BinStream &operator>>(BinStream &, ADSRImpl &);
