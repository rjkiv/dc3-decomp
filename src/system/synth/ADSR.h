#pragma once
#include "obj/Object.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

/** Implementation of ADSR envelope */
class ADSRImpl {
    friend class ADSR;
    friend bool PropSync(ADSRImpl &, DataNode &, DataArray *, int, PropOp);

public:
    /**
     * @enum AttackMode
     * Envelope attack shape
     */
    enum AttackMode {
        /** Linear curve */
        kAttackLinear = 0,
        /** Exponential curve */
        kAttackExp = 1,
    };
    /**
     * @enum SustainMode
     * Envelope sustain shape
     */
    enum SustainMode {
        /** Linearly increasing sustain */
        kSustainLinInc = 0,
        /** Linearly decreasing sustain */
        kSustainLinDec = 2,
        /** Exponentially increasing sustain */
        kSustainExpInc = 4,
        /** Exponentially decreasing sustain */
        kSustainExpDec = 6,
    };
    /**
     * @enum ReleaseMode
     * Envelope release shape
     */
    enum ReleaseMode {
        /** Linear curve */
        kReleaseLinear = 0,
        /** Exponential curve */
        kReleaseExp = 1,
    };

    ADSRImpl();
    /** Returns the attack rate in seconds */
    float GetAttackRate() const;
    /** Returns the release rate in seconds */
    float GetReleaseRate() const;
    /** Serializes this ADSRImpl..
     *
     * @param [in] bs The BinStream to save to.
     */
    void Save(BinStream &) const;
    /** Deserializes this ADSRImpl.
     *
     * @param [in] bs The BinStream to load from.
     * @param [in] adsr The ADSR owning this implementation.
     * @milofail When trying to load where rev > 1 or altRev > 0
     */
    void Load(BinStream &, ADSR *);

private:
    /** @hmx{Duration of attack in seconds} */
    float mAttackRate; // 0x0
    /** @hmx{Duration of decay in seconds} */
    float mDecayRate; // 0x4
    /** @hmx{Duration of sustain in seconds} */
    float mSustainRate; // 0x8
    /** @hmx{Duration of release in seconds} */
    float mReleaseRate; // 0xc
    /** @hmx{Level of sustain volume (0-1)} */
    float mSustainLevel; // 0x10
    /** @hmx{Attack mode} */
    AttackMode mAttackMode; // 0x14
    /** @hmx{Sustain mode} */
    SustainMode mSustainMode; // 0x18
    /** @hmx{Release mode} */
    ReleaseMode mReleaseMode; // 0x1c
    /** Whether the ADSR is synced */
    bool mSynced; // 0x20
};

/** @hmx{Attack, decay, sustain, and release. Envelope settings to modify sounds.} */
class ADSR : public Hmx::Object {
public:
    virtual ~ADSR() {}
    OBJ_CLASSNAME(ADSR);
    OBJ_SET_TYPE(ADSR);
    virtual DataNode Handle(DataArray *, bool);
    /** @copydoc Hmx::Object::SyncProperty
     *
     * Supported properties:
     * - `attack_mode`
     * - `attack_rate`
     * - `decay_rate`
     * - `sustain_mode`
     * - `sustain_rate`
     * - `sustain_level`
     * - `release_mode`
     * - `release_rate`
     */
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);

    OBJ_MEM_OVERLOAD(0x61);
    NEW_OBJ(ADSR)

    /** Returns a reference to this version of Milo's ADSR implementation */
    ADSRImpl &Impl() { return mADSR; }

protected:
    ADSR();

    ADSRImpl mADSR; // 0x2c
};

BinStream &operator<<(BinStream &, const ADSRImpl &);
BinStream &operator>>(BinStream &, ADSRImpl &);
