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
    /** Saves this ADSRImpl to a BinStream.
     *
     * @param [in] bs The BinStream to save to.
     */
    void Save(BinStream &) const;
    /** Loads this ADSRImpl from a BinStream.
     *
     * @param [in] bs The BinStream to load from.
     * @param [in] adsr The ADSR Object owning this implementation.
     * @throws MILO_FAIL if trying to load a version newer than 1.0
     */
    void Load(BinStream &, ADSR *);

private:
    /** Duration of attack in seconds */
    float mAttackRate; // 0x0
    /** Duration of decay in seconds */
    float mDecayRate; // 0x4
    /** Duration of sustain in seconds */
    float mSustainRate; // 0x8
    /** Duration of release in seconds */
    float mReleaseRate; // 0xc
    /** Level of sustain volume (0-1) */
    float mSustainLevel; // 0x10
    /** Attack mode */
    AttackMode mAttackMode; // 0x14
    /** Sustain mode */
    SustainMode mSustainMode; // 0x18
    /** Release mode */
    ReleaseMode mReleaseMode; // 0x1c
    /** Whether the ADSR is synced */
    bool mSynced; // 0x20
};

/**
 * @brief Envelope settings to modify sounds
 *
 * Attack, Decay, Sustain, Release
 */
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
