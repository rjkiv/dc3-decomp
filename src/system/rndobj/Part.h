#pragma once
#include "math/Color.h"
#include "math/Mtx.h"
#include "math/Vec.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Draw.h"
#include "rndobj/Mat.h"
#include "rndobj/Mesh.h"
#include "rndobj/Poll.h"
#include "rndobj/Trans.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

// size 0x68
class RndParticle {
public:
    MEM_ARRAY_OVERLOAD(Particle, 0x1E);

    RndParticle *Next() const { return next; }
    // bool Active() const;
    const Vector3 &Pos() const { return pos.AsVector3(); }
    Vector3 &Pos() { return pos.AsVector3(); }
    const Vector3 &Vel() const { return vel.AsVector3(); }
    Vector3 &Vel() { return vel.AsVector3(); }
    // RndParticle();

    Hmx::Color col; // 0x0
    Hmx::Color colVel; // 0x10
    Vector4 pos; // 0x20
    Vector4 vel; // 0x30
    float deathFrame; // 0x40
    float birthFrame; // 0x44
    float size; // 0x48
    float sizeVel; // 0x4c
    float angle; // 0x50
    float swingArm; // 0x54
    RndParticle *prev; // 0x58
    RndParticle *next; // 0x5c
    int tileIndex; // 0x60
    float animTime; // 0x64
};

// size 0xc8
class RndFancyParticle : public RndParticle {
public:
    const Vector3 &BubbleDir() const { return bubbleDir.AsVector3(); }
    Vector3 &BubbleDir() { return bubbleDir.AsVector3(); }

    float growFrame; // 0x68
    float growVel; // 0x6c
    float shrinkFrame; // 0x70
    float shrinkVel; // 0x74
    Hmx::Color midcolVel; // 0x78
    float midcolFrame; // 0x88
    float beginGrow; // 0x8c
    float midGrow; // 0x90
    float endGrow; // 0x94
    Vector4 bubbleDir; // 0x98
    float bubbleFreq; // 0xa8
    float bubblePhase; // 0xac
    float RPF; // 0xb0
    float swingArmVel; // 0xb4
    Vector3 momentum; // 0xb8
};

class ParticleCommonPool {
public:
    ParticleCommonPool()
        : mPoolParticles(nullptr), mPoolFreeParticles(nullptr), mNumUsed(0),
          mHighWaterMark(0) {}
    void InitPool();
    RndParticle *AllocateParticle();
    RndParticle *FreeParticle(RndParticle *);
    int NumUsed() const { return mNumUsed; }
    int HighWaterMark() const { return mHighWaterMark; }

    MEM_OVERLOAD(ParticleCommonPool, 0x254)

    RndFancyParticle *mPoolParticles; // 0x0
    RndFancyParticle *mPoolFreeParticles; // 0x4
protected:
    int mNumUsed; // 0x8
    int mHighWaterMark; // 0xc
};

class PartOverride {
public:
    enum ParticleOverride {
        kLife = 0x0001,
        kSpeed = 0x0002,
        kSize = 0x0004,
        kDeltaSize = 0x0008,
        kStartColor = 0x0010,
        kMidColor = 0x0020,
        kEndColor = 0x0040,
        kEmitDirection = 0x0080,
        kMeshEmitter = 0x0100,
        kBoxEmitter = 0x0200,
    };

    PartOverride()
        : mask(0), life(0), speed(0), deltaSize(0), startColor(0), midColor(0),
          endColor(0), pitch(0, 0), yaw(0, 0), mesh(0),
          box(Vector3(0, 0, 0), Vector3(0, 0, 0)) {}

    unsigned int mask; // 0x0
    float life; // 0x4
    float speed; // 0x8
    float size; // 0xc
    float deltaSize; // 0x10
    Hmx::Color startColor; // 0x14
    Hmx::Color midColor; // 0x24
    Hmx::Color endColor; // 0x34
    Vector2 pitch; // 0x44
    Vector2 yaw; // 0x4c
    RndMesh *mesh; // 0x54
    Box box; // 0x58
};

// Attractor
class Attractor {
public:
    Attractor(Hmx::Object *owner) : mAttractor(owner), mStrength(1) {}
    Attractor(const Attractor &a, Hmx::Object *owner)
        : mAttractor(owner, a.mAttractor), mStrength(a.mStrength) {}
    void Save(BinStream &) const;
    void Load(BinStreamRev &);

    /** "Attracts/repulses particles" */
    ObjPtr<RndTransformable> mAttractor; // 0x0
    /** "Positive value means attract particles, negative means repulse" */
    float mStrength; // 0x14
};

/** "A ParticleSys object generates, animates, and draws large
    numbers of similar sprites. Currently particles are rendered only
    as points on the PC." */
class RndParticleSys : public RndAnimatable,
                       public RndPollable,
                       public RndTransformable,
                       public RndDrawable {
public:
    enum Type {
        kBasic = 0,
        kFancy = 1
    };
    class Burst {
    public:
        bool Set(float peak, float length);
        float Emit(float deltaFrame);

    protected:
        float mPeak; // 0x0
        float mHalfLength; // 0x4
        float mNorm; // 0x8
        float mTimeToLive; // 0xc
    };
    // Hmx::Object
    virtual ~RndParticleSys();
    virtual bool Replace(ObjRef *, Hmx::Object *);
    OBJ_CLASSNAME(ParticleSys);
    OBJ_SET_TYPE(ParticleSys);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // RndAnimatable
    virtual void SetFrame(float, float);
    virtual float EndFrame();
    // RndPollable
    virtual void Poll();
    virtual void Enter();
    // RndDrawable
    virtual void UpdateSphere();
    virtual bool MakeWorldSphere(Sphere &, bool);
    virtual void Mats(std::list<class RndMat *> &, bool);
    virtual void DrawShowing();
    // RndHighlightable
    virtual void Highlight() { RndDrawable::Highlight(); }

    virtual void SetPreserveParticles(bool b) { mPreserveParticles = b; }
    virtual void SetPool(int n, Type t);
    virtual void SetPersistentPool(int n, Type t);

    OBJ_MEM_OVERLOAD(0x90)
    NEW_OBJ(RndParticleSys)
    REGISTER_OBJ(RndParticleSys)

    void SetRelativeMotion(float r, RndTransformable *parent);
    void SetSubSamples(int s);
    void FreeAllParticles();
    void SetAnimatedUV(bool v);
    void SetTileHoldTime(float);
    void SetNumTiles(int v);
    void SetGrowRatio(float);
    void SetShrinkRatio(float);
    void SetFrameDrive(bool);
    void SetPauseOffscreen(bool);
    RndParticle *AllocParticle();
    RndParticle *FreeParticle(RndParticle *p);
    void MakeLocToRel(Transform &xfm);
    void ExplicitParticles(int cnt, bool now, PartOverride &po);
    void InitParticle(RndParticle *p, const Transform *locToRel);
    Type GetType() const { return mType; }
    RndMat *Mat() const { return mMat; }
    void SetMat(RndMat *mat) { mMat = mat; }
    RndMesh *Mesh() const { return mMesh; }
    void SetMesh(RndMesh *m);
    RndParticle *ActiveParticles() { return mActiveParticles; }
    bool PreserveParticles() const { return mPreserveParticles; }

    float CalcFrame() {
        if (mFrameDrive) {
            return Frame();
        } else {
            return mElapsedTime;
        }
    }

    // clang-format off
    const Hmx::Color &StartColorLow() const { return mStartColorLow; }
    const Hmx::Color &StartColorHigh() const { return mStartColorHigh; }
    void SetStartColor(const Hmx::Color &low, const Hmx::Color &high) { mStartColorLow = low; mStartColorHigh = high; }
    const Hmx::Color &EndColorLow() const { return mEndColorLow; }
    const Hmx::Color &EndColorHigh() const { return mEndColorHigh; }
    void SetEndColor(const Hmx::Color &low, const Hmx::Color &high) { mEndColorLow = low; mEndColorHigh = high; }
    const Vector2 &EmitRate() const { return mEmitRate; }
    void SetEmitRate(float x, float y) { mEmitRate.x = x; mEmitRate.y = y; }
    const Vector2 &Speed() const { return mSpeed; }
    void SetSpeed(float x, float y) { mSpeed.x = x; mSpeed.y = y; }
    const Vector2 &Life() const { return mLife; }
    void SetLife(float x, float y) { mLife.x = x; mLife.y = y; }
    const Vector2 &StartSize() const { return mStartSize; }
    void SetStartSize(float x, float y) { mStartSize.x = x; mStartSize.y = y; }
    const Vector2 &DeltaSize() const { return mDeltaSize; }
    void SetDeltaSize(float x, float y) { mDeltaSize.x = x; mDeltaSize.y = y; }
    const Vector2 &Pitch() const { return mPitch; }
    void SetPitch(float x, float y) { mPitch.x = x; mPitch.y = y; }
    const Vector2 &Yaw() const { return mYaw; }
    void SetYaw(float x, float y) { mYaw.x = x; mYaw.y = y; }
    const Vector3 &PosLow() const { return mPosLow; }
    const Vector3 &PosHigh() const { return mPosHigh; }
    void SetPos(const Vector3 &low, const Vector3 &high) { mPosLow = low; mPosHigh = high; }
    const Vector2 &BubbleSize() const { return mBubbleSize; }
    void SetBubbleSize(float x, float y) { mBubbleSize.x = x; mBubbleSize.y = y; }
    const Vector2 &BubblePeriod() const { return mBubblePeriod; }
    void SetBubblePeriod(float x, float y) { mBubblePeriod.x = x; mBubblePeriod.y = y; }
    const Vector3 &Force() const { return mForce; }
    void SetForce(const Vector3 &v) { mForce = v; }
    bool Rotate() const { return mRotate; }
    void SetRotate(bool b) { mRotate = b; }
    bool AlignWithVelocity() const { return mAlignWithVelocity; }
    void SetAlignWithVelocity(bool b) { mAlignWithVelocity = b; }
    bool StretchWithVelocity() const { return mStretchWithVelocity; }
    void SetStretchWithVelocity(bool b) { mStretchWithVelocity = b; }
    bool ConstantArea() const { return mConstantArea; }
    void SetConstantArea(bool b) { mConstantArea = b; }
    int MaxBursts() const { return mMaxBurst; }
    void SetMaxBurst(int i) { mMaxBurst = i; }
    const Vector2 &BurstInterval() const { return mBurstInterval; }
    void SetBurstInterval(float x, float y) { mBurstInterval.x = x; mBurstInterval.y = y; }
    const Vector2 &BurstPeak() const { return mBurstPeak; }
    void SetBurstPeak(float x, float y) { mBurstPeak.x = x; mBurstPeak.y = y; }
    const Vector2 &BurstLength() const { return mBurstLength; }
    void SetBurstLength(float x, float y) { mBurstLength.x = x; mBurstPeak.y = y; }
    float AngularDrag() const { return mRotDrag; }
    void SetAngularDrag(float f) { mRotDrag = f; }
    const Vector2 &AngularSpeed() const { return mRotSpeed; }
    void SetAngularSpeed(float x, float y) { mRotSpeed.x = x; mRotSpeed.y = y; }
    const Vector2 &SwingArmStart() const { return mSwingArmStart; }
    void SetSwingArmStart(float x, float y) { mSwingArmStart.x = x; mSwingArmStart.y = y; }
    const Vector2 &SwingArmEnd() const { return mSwingArmEnd; }
    void SetSwingArmEnd(float x, float y) { mSwingArmEnd.x = x; mSwingArmEnd.y = y; }
    float Drag() const { return mDrag; }
    void SetDrag(float f) { mDrag = f; }
    float StretchScale() const { return mStretchScale; }
    void SetStretchScale(float f) { mStretchScale = f; }
    const Hmx::Color &MidColorLow() const { return mMidColorLow; }
    const Hmx::Color &MidColorHigh() const { return mMidColorHigh; }
    void SetMidColor(const Hmx::Color &low, const Hmx::Color &high) { mMidColorLow = low; mMidColorHigh = high; }
    bool Bubble() const { return mBubble; }
    void SetBubble(bool b) { mBubble = b; }
    bool PerspectiveStretch() const { return mPerspectiveStretch; }
    void SetPerspectiveStretch(bool b) { mPerspectiveStretch = b; }
    float HeightRatio() const { return mHeightRatio; }
    void SetHeighRatio(float f) { mHeightRatio = f; }
    // clang-format on

    // bool FrameDrive() const;
    // bool PauseOffscreen() const;
    // int SubSamples() const;
    // void SetFastForward(bool);
    // bool FastForward() const;

    // float RelativeMotion() const;
    // void SetBounce(RndTransformable *);
    // RndTransformable *Bounce() const;

    // void SetAnimatedUV(bool);
    // void SetAnimUVSpeed(float);
    // void SetTilesAcross(int);
    // void SetTilesDown(int);
    // void SetNumTiles(int);

    // int MaxParticles() const;
    // int NumActive() const;
    // bool Inactive() const;
    // void SetGrowRatio(float);
    // float GrowRatio() const;
    // void SetShrinkRatio(float);
    // float ShrinkRatio() const;
    // void SetMidColorRatio(float);
    // float MidColorRatio() const;

    bool CheckParticleLife(float frame, RndParticle *particle) {
        return frame >= particle->deathFrame || frame < particle->birthFrame;
    }

protected:
    RndParticleSys();

    void UpdateParticles();
    void UpdateRelativeXfm();
    void
    InitParticle(float frame, RndParticle *p, const Transform *locToRel, PartOverride &po);
    float CheckBursts(float deltaFrame);
    void CreateParticles(float frame, float deltaFrame, const Transform &xfm);
    void RunFastForward();
    void MoveParticles(float frame, float deltaFrame);

    DataNode OnSetStartColor(const DataArray *);
    DataNode OnSetStartColorInt(const DataArray *);
    DataNode OnSetEndColor(const DataArray *);
    DataNode OnSetEndColorInt(const DataArray *);
    DataNode OnSetEmitRate(const DataArray *);
    DataNode OnAddEmitRate(const DataArray *);
    DataNode OnSetBurstInterval(const DataArray *);
    DataNode OnSetBurstPeak(const DataArray *);
    DataNode OnSetBurstLength(const DataArray *);
    DataNode OnExplicitPart(const DataArray *);
    DataNode OnExplicitParts(const DataArray *);
    DataNode OnSetLife(const DataArray *);
    DataNode OnSetSpeed(const DataArray *);
    DataNode OnSetRotate(const DataArray *);
    DataNode OnSetSwingArm(const DataArray *);
    DataNode OnSetDrag(const DataArray *);
    DataNode OnSetAlignment(const DataArray *);
    DataNode OnSetStartSize(const DataArray *);
    DataNode OnSetMat(const DataArray *);
    DataNode OnSetPos(const DataArray *);
    DataNode OnActiveParticles(const DataArray *);

    Type mType; // 0x118
    /** "maximum number of particles". Ranges from 0 to 3072. */
    int mMaxParticles; // 0x11c
    RndParticle *mPersistentParticles; // 0x120
    RndParticle *mFreeParticles; // 0x124
    RndParticle *mActiveParticles; // 0x128
    int mNumActive; // 0x12c
    float mEmitCount; // 0x130
    bool mFrameDrive; // 0x134
    float mLastFrame; // 0x138
    int mPollsSinceDraw; // 0x13c
    /** "Freezes the particle motion when they are offscreen, CPU savings" */
    bool mPauseOffscreen; // 0x140
    float mSleepTime; // 0x144
    Vector2 mBubblePeriod; // 0x148
    Vector2 mBubbleSize; // 0x150
    /** "Frame range of particle life." */
    Vector2 mLife; // 0x158
    /** "Min point and max point, in object coordinates,
        of box region that particles are emitted from." */
    Vector3 mPosLow; // 0x160
    /** "Min point and max point, in object coordinates,
        of box region that particles are emitted from." */
    Vector3 mPosHigh; // 0x170
    /** "Speed range, in world units per frame, of particles." */
    Vector2 mSpeed; // 0x180
    Vector2 mPitch; // 0x188
    Vector2 mYaw; // 0x190
    /** "Frame range to generate particles." */
    Vector2 mEmitRate; // 0x198
    /** "Size range, in world units, of particles." */
    Vector2 mStartSize; // 0x1a0
    /** "Change in size of particles, in world units." */
    Vector2 mDeltaSize; // 0x1a8
    /** "Random color ranges for start and end color of particles." */
    Hmx::Color mStartColorLow; // 0x1b0
    /** "Random color ranges for start and end color of particles." */
    Hmx::Color mStartColorHigh; // 0x1c0
    /** "Random color ranges for start and end color of particles." */
    Hmx::Color mEndColorLow; // 0x1d0
    /** "Random color ranges for start and end color of particles." */
    Hmx::Color mEndColorHigh; // 0x1e0
    ObjPtr<RndMesh> mMesh; // 0x1f0
    /** "material for particle system" */
    ObjPtr<RndMat> mMat; // 0x204
    bool mPreserveParticles; // 0x218
    Transform mRelativeXfm; // 0x21c
    Transform mLastWorldXfm; // 0x25c
    /** "If [motion_parent] is set, is amount to make movement relative to it,
        0-1, 1 being fully relative to the parent" */
    float mRelativeMotion; // 0x29c
    /** "Makes particles move relative to this Trans" */
    ObjOwnerPtr<RndTransformable> mMotionParent; // 0x2a0
    Vector3 mMomentum; // 0x2b4
    /** "Specify a collide plane to reflect particles.
        Used to bounce particles off surfaces." */
    ObjPtr<RndTransformable> mBounce; // 0x2c4
    /** "Force direction in world coordinates,
        in units per frame added to each particle's velocity.
        Can be used for gravity." */
    Vector3 mForce; // 0x2d8
    float mDrag; // 0x2e8
    bool mBubble; // 0x2ec
    /** "Spawns particles on enter, so we start out with particles there" */
    bool mFastForward; // 0x2ed
    bool mNeedForward; // 0x2ee
    bool mRotate; // 0x2ef
    Vector2 mRotSpeed; // 0x2f0
    float mRotDrag; // 0x2f8
    bool mRotRandomDir; // 0x2fc
    Vector2 mSwingArmStart; // 0x300
    Vector2 mSwingArmEnd; // 0x308
    bool mAlignWithVelocity; // 0x310
    bool mStretchWithVelocity; // 0x311
    bool mConstantArea; // 0x312
    bool mPerspectiveStretch; // 0x313
    float mStretchScale; // 0x314
    /** "Ratio of screen height to width" */
    float mHeightRatio; // 0x318
    int mSubSamples; // 0x31c
    Transform mSubSampleXfm; // 0x320
    float mGrowRatio; // 0x360
    float mShrinkRatio; // 0x364
    float mMidColorRatio; // 0x368
    Hmx::Color mMidColorLow; // 0x36c
    Hmx::Color mMidColorHigh; // 0x37c
    /** "Add relative parent's momentum to each particle's initial speed.
        Fancy property must be true." */
    bool mBirthMomentum; // 0x38c
    /** "Amount of birth momentum to use. 0 is none, 1 is full amount,
        2 is twice the momentum, -1 means use the opposite of the momentum
        relative to the motion parent. Fancy property must be true." */
    float mBirthMomentumAmount; // 0x390
    std::vector<Burst> mBursts; // 0x394
    int mMaxBurst; // 0x3a0
    float mTimeTillBurst; // 0x3a4
    Vector2 mBurstInterval; // 0x3a8
    Vector2 mBurstPeak; // 0x3b0
    Vector2 mBurstLength; // 0x3b8
    int mExplicitParts; // 0x3c0
    float mElapsedTime; // 0x3c4
    /** "uses material texture as page tiles to animated through" */
    bool mAnimatedUVs; // 0x3c8
    /** "animation loops to beginning if true" */
    bool mLoopAnim; // 0x3c9
    /** "particle animation starts at random location" */
    bool mRandomStart; // 0x3ca
    /** "speed in seconds it takes to switch to next tile" */
    float mTileAnimSpeed; // 0x3cc
    /** "number of tiles across in specified texture" */
    int mTilesAcross; // 0x3d0
    /** "number of tiles down in specified texture" */
    int mTilesDown; // 0x3d4
    /** "total number of tiles used for animation" */
    int mNumTiles; // 0x3d8
    /** "starting frame of animation" */
    int mFirstUsedTile; // 0x3dc
    float mLoopSpeed; // 0x3e0
    float mInvLoopSpeed; // 0x3e4
    /** "Add point forces which attract or repel particles" */
    ObjVector<Attractor> mAttractors; // 0x3e8
};

void InitParticleSystem();
