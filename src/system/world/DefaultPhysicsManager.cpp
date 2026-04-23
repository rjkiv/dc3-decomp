#include "world/DefaultPhysicsManager.h"
#include "math/Geo.h"
#include "math/Mtx.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Draw.h"
#include "rndobj/Mesh.h"
#include "world/PhysicsManager.h"
#include "world/PhysicsVolume.h"

#pragma region RayCastDefaultContainer

RayCastDefaultContainer::RayCastDefaultContainer(
    const Box &box,
    std::list<RndMesh *> meshes,
    std::map<Hmx::Object *, ObjectDir *> &objMap
) {
    FOREACH (it, meshes) {
        RndMesh *d = *it;
        if (d) {
            Sphere s;
            if (d->MakeWorldSphere(s, false)) {
                if (box.Contains(s)) {
                    MILO_ASSERT(objMap.find(d) != objMap.end(), 0x73);
                    unk4.push_back(std::make_pair(d, objMap[d]));
                }
            }
        }
    }
}

Hmx::Object *RayCastDefaultContainer::FindNearest(
    const Segment &s, float &f, Vector3 &v, Hmx::Object *&o
) {
    o = nullptr;
    f = 1;
    Segment localSegment = s;
    Hmx::Object *ret = nullptr;
    FOREACH (it, unk4) {
        RndMesh *curMesh = it->first;
        Vector3 curVec;
        Plane curPlane;
        if (curMesh->Collide(localSegment, curVec.x, curPlane)) {
            float xScalar = curVec.x;
            Interp(localSegment.start, localSegment.end, curVec.x, localSegment.end);
            v = reinterpret_cast<Vector3 &>(curPlane);
            o = curMesh;
            ret = it->second;
            f *= xScalar;
        }
    }
    return ret;
}

void RayCastDefaultContainer::SetFilter(int) {
    MILO_NOTIFY("Filters are unsupported as yet");
}

#pragma endregion
#pragma region DefaultDetectionVolume

DefaultDetectionVolume::DefaultDetectionVolume(DetectionVolumeListener *dvl)
    : mListener(dvl), mActiveState(false) {}

#pragma endregion
#pragma region DefaultPhysicsManager

DefaultPhysicsManager::DefaultPhysicsManager(RndDir *d)
    : PhysicsManager(d), unk40(this, kObjListOwnerControl) {}

bool DefaultPhysicsManager::Replace(ObjRef *from, Hmx::Object *to) {
    if (from->Parent() != &unk40) {
        Hmx::Object *obj = from->GetObj();
        // unk40.erase(it);
        RemoveCollidable(obj);
        return true;
    } else {
        return Hmx::Object::Replace(from, to);
    }
}

void DefaultPhysicsManager::Poll() {
    for (auto it = mActiveCollidables.begin(); it != mActiveCollidables.end();) {
        RndMesh *d = *it;
        if (!IsShowing(d)) {
            auto cur = it;
            ++it;
            mActiveCollidables.erase(cur);
            MILO_ASSERT(std::find( mInactiveCollidables.begin(), mInactiveCollidables.end(), d) == mInactiveCollidables.end(), 0x41);
            mInactiveCollidables.push_front(d);
        } else {
            ++it;
        }
    }
    for (auto it = mInactiveCollidables.begin(); it != mInactiveCollidables.end();) {
        RndMesh *d = *it;
        if (IsShowing(d)) {
            auto cur = it;
            ++it;
            mInactiveCollidables.erase(cur);
            MILO_ASSERT(std::find( mActiveCollidables.begin(), mActiveCollidables.end(), d) == mActiveCollidables.end(), 0x55);
            mActiveCollidables.push_front(d);
        } else {
            ++it;
        }
    }
}

RayCastContainer *DefaultPhysicsManager::MakeContainer(const Box &box, unsigned int ui) {
    return new RayCastDefaultContainer(box, mActiveCollidables, unk54);
}

DetectionVolume *DefaultPhysicsManager::MakeDetectionVolume(
    DetectionVolumeListener *dvl, const Transform &, PhysicsVolumeType, CollisionFilter
) {
    return new DefaultDetectionVolume(dvl);
}

void DefaultPhysicsManager::CastRays(RayCast *, int) { MILO_FAIL("not implemented"); }

void DefaultPhysicsManager::CastRays(
    const Segment *s, RayCastListener *rcl, int i3, unsigned int ui4
) {
    float collideFloat = 1;
    for (int i = 0; i < i3; i++) {
        Segment localSegment = s[i];
        FOREACH (it, mActiveCollidables) {
            Plane curPlane;
            RndDrawable *d = (*it)->Collide(localSegment, collideFloat, curPlane);
            if (d) {
            }
        }
    }
}

void DefaultPhysicsManager::ActivateCollidable(Hmx::Object *o) {
    auto it = std::find(mInactiveCollidables.begin(), mInactiveCollidables.end(), o);
    if (it != mInactiveCollidables.end()) {
        RndMesh *mesh = *it;
        mInactiveCollidables.erase(it);
        mActiveCollidables.push_front(mesh);
    }
}

void DefaultPhysicsManager::DeactivateCollidable(Hmx::Object *o) {
    auto it = std::find(mInactiveCollidables.begin(), mInactiveCollidables.end(), o);
    if (it != mInactiveCollidables.end()) {
        RndMesh *mesh = *it;
        mActiveCollidables.erase(it);
        mInactiveCollidables.push_front(mesh);
    }
}

void DefaultPhysicsManager::RemoveAll() {
    unk54.clear();
    mActiveCollidables.clear();
    mInactiveCollidables.clear();
    unk40.clear();
}

void DefaultPhysicsManager::AddCollidable(Hmx::Object *o, ObjectDir *dir, bool isActive) {
    RndMesh *mesh = dynamic_cast<RndMesh *>(o);
    if (mesh) {
        if (unk54.find(mesh) == unk54.end()) {
            unk54[mesh] = dir;
            if (isActive) {
                mActiveCollidables.push_front(mesh);
            } else {
                mInactiveCollidables.push_front(mesh);
            }
            unk40.insert(unk40.begin(), o);
        }
    }
}

void DefaultPhysicsManager::RemoveCollidable(Hmx::Object *o) {
    auto mapIt = unk54.find(o);
    if (mapIt != unk54.end()) {
        auto activeIt =
            std::find(mActiveCollidables.begin(), mActiveCollidables.end(), o);

        if (activeIt != mActiveCollidables.end()) {
            mActiveCollidables.erase(activeIt);
        } else {
            auto inactiveIt =
                std::find(mInactiveCollidables.begin(), mInactiveCollidables.end(), o);
            if (inactiveIt != mInactiveCollidables.end()) {
                mInactiveCollidables.erase(inactiveIt);
            }
        }
        unk54.erase(mapIt);
        unk40.remove(o);
    }
}