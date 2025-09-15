#pragma once
#include "math/Mtx.h"
#include "obj/Object.h"
#include "world/PhysicsVolume.h"

class DetectionVolume : public Hmx::Object {
public:
    virtual ~DetectionVolume() {}
    virtual void SetActiveState(bool) = 0;
    virtual bool GetActiveState() const = 0;
    virtual void Reset(const Transform &) = 0;
    // 0x64
    // 0x68
    // 0x6c
    // 0x70
    // 0x74
    // 0x78
    virtual void SetCollisionFilter(CollisionFilter) {} // 0x7c
    // 0x80
};

// 820ef014 82  99  fa  d0    addr       _purecall
// 820ef018 82  99  fa  d0    addr       _purecall
// 820ef01c 82  99  fa  d0    addr       _purecall
// 820ef020 82  3e  3b  70    addr       Song::Copy
// 820ef024 82  3e  3b  70    addr       Song::Copy
// 820ef028 82  3e  3b  70    addr       Song::Copy
// 820ef02c 82  3e  3b  70    addr       Song::Copy
// 820ef030 82  3e  3b  70    addr       Song::Copy
// 820ef034 82  3e  3b  70    addr       Song::Copy
// 820ef038 82  3e  3b  70    addr       Song::Copy
// 820ef03c 82  3e  3b  70    addr       Song::Copy

// clang-format off
// 0x823e3b70: public: virtual void __cdecl DetectionVolume::GetOverlaps(class stlpmtx_std::list<struct stlpmtx_std::pair<class Hmx::Object *, class ObjectDir *>, class stlpmtx_std::StlNodeAlloc<struct stlpmtx_std::pair<class Hmx::Object *, class ObjectDir *> > > &)
// 0x823e3b70: public: virtual void __cdecl DetectionVolume::ApplyDirectionalImpulse(class Vector3const &)
// 0x823e3b70: public: virtual void __cdecl DetectionVolume::ApplyRadialForce(float)
// 0x823e3b70: public: virtual void __cdecl DetectionVolume::SetCollisionFilter(enum CollisionFilter) 0x7c
// 0x823e3b70: public: virtual void __cdecl DetectionVolume::ApplyRadialImpulse(float)
// 0x823e3b70: public: virtual void __cdecl DetectionVolume::ApplyTangentialForce(class Vector3)
// 0x823e3b70: public: virtual void __cdecl DetectionVolume::ApplyDirectionalLinearVelocity(class Vector3const &)
// 0x823e3b70: public: virtual void __cdecl DetectionVolume::ApplyDirectionalForce(class Vector3const &)

// 0x823e3b70: public: virtual void __cdecl DefaultDetectionVolume::ApplyRadialForce(float)
// 0x823e3b70: public: virtual void __cdecl DefaultDetectionVolume::Reset(class Transform const &)
// 0x826abba0: public: virtual bool __cdecl DefaultDetectionVolume::GetActiveState(void) const
// 0x8285d838: public: virtual void __cdecl DefaultDetectionVolume::SetActiveState(bool)
// 0x8285d818: public: virtual void __cdecl DefaultDetectionVolume::Release(void)
// clang-format on
