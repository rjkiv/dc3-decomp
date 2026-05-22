#include "world/PhysicsManager.h"
#include "PhysicsManager.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/Dir.h"
#include "rndobj/Draw.h"
#include "rndobj/Group.h"
#include "rndobj/Mesh.h"
#include "rndobj/Trans.h"
#include "world/DefaultPhysicsManager.h"
#include "world/PhysicsVolume.h"

namespace {
    __declspec(noinline) bool HasKeepMeshData(const RndMesh *mesh) {
        for (; mesh != nullptr; mesh = mesh->GetGeomOwner()) {
            if (mesh->GetKeepMeshData()) {
                return true;
            }
            if (mesh == mesh->GetGeomOwner()) {
                return false;
            }
        }
        return false;
    }
}

PhysicsManager::PhysicsManager(RndDir *dir)
    : unk2c(dir), mPhysicsClampTime(30), unk34(0), unk35(0), unk38(30), unk3c(0) {
    if (SystemConfig()) {
        DataArray *physicsCfg = SystemConfig()->FindArray("physics", false);
        if (physicsCfg) {
            DataArray *clampArr = physicsCfg->FindArray("physics_clamp_time", false);
            if (clampArr) {
                mPhysicsClampTime = clampArr->Float(1);
            }
        }
    }
}

PhysicsManager::~PhysicsManager() {}

BEGIN_HANDLERS(PhysicsManager)
    HANDLE_EXPR(collidable_mass, GetMass(_msg->Obj<RndTransformable>(2)))
    HANDLE(collidable_velocity, OnGetVelocity)
    HANDLE_ACTION(
        set_gravity_dir,
        SetGravityDirection(Vector3(_msg->Float(2), _msg->Float(3), _msg->Float(4)))
    )
    HANDLE(apply_force, OnApplyForce)
    HANDLE_EXPR(collidable_force, GetForce(_msg->Obj<RndTransformable>(2)))
    HANDLE_EXPR(toggle_show_volumes, PhysicsVolume::sShowing = !PhysicsVolume::sShowing)
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void PhysicsManager::HarvestCollidables(ObjectDir *parentProxy) {
    MILO_ASSERT(NULL != parentProxy, 0x86);
    static Symbol collidable("collidable");
    for (ObjDirItr<RndDrawable> it(parentProxy, true); it != nullptr; ++it) {
        RndDrawable *cur = it;
        RndMesh *mesh = dynamic_cast<RndMesh *>(cur);
        if (mesh) {
            const DataNode *prop = cur->Property(collidable, false);
            if (prop) {
                if (prop->Int() == 1) {
                    bool u2 = true;
                    if (!mesh->GetKeepMeshData()) {
                        if (mesh != mesh->GetGeomOwner()) {
                            u2 = HasKeepMeshData(mesh->GetGeomOwner());
                        } else {
                            u2 = false;
                        }
                    }
                    if (u2) {
                        AddCollidable(cur, parentProxy, mesh->Showing());
                    } else {
                        MILO_NOTIFY(
                            "Mesh %s in Dir %s is flagged as collidable, but does not have its 'keep mesh data' set to true. Collision object will not be generated!",
                            mesh->Name(),
                            parentProxy->GetPathName()
                        );
                    }
                }
            }
        } else {
            PhysicsVolume *pv = dynamic_cast<PhysicsVolume *>(cur);
            if (pv) {
                pv->CreatePhysicsVolume(this);
            }
        }
        ObjectDir *proxyProxy = dynamic_cast<ObjectDir *>(cur);
        if (proxyProxy && proxyProxy->IsProxy()) {
            HarvestCollidables(proxyProxy);
        }
    }
}

bool PhysicsManager::IsShowing(Hmx::Object *obj) {
    RndDir *dir = dynamic_cast<RndDir *>(GetCollidableDir(obj));
    if (dir && !dir->Showing())
        return false;
    else {
        RndDrawable *draw = dynamic_cast<RndDrawable *>(obj);
        if (draw && !draw->Showing()) {
            return false;
        } else {
            bool ret = false;
            for (auto it = obj->Refs().begin(); it != obj->Refs().end();) {
                RndGroup *group = dynamic_cast<RndGroup *>(it->RefOwner());
                ++it;
                if (group) {
                    ret = true;
                    if (group->Showing()) {
                        return true;
                    }
                }
            }
            return !ret;
        }
    }
}

void PhysicsManager::SyncObjects(bool b1) {
    RemoveAll();
    Timer timer;
    timer.Start();
    HarvestCollidables(unk2c);
    timer.SplitMs();
    unk34 = true;
}

DataNode PhysicsManager::OnGetVelocity(const DataArray *a) {
    Vector3 v;
    GetVelocity(a->Obj<RndTransformable>(2), v);
    return std::sqrt(v.y * v.y + v.x * v.x + v.z * v.z);
}

DataNode PhysicsManager::OnApplyForce(const DataArray *a) {
    Hmx::Object *obj = a->Obj<Hmx::Object>(2);
    ApplyForce(obj, Vector3(a->Float(3), a->Float(4), a->Float(5)));
    return 0;
}

PhysicsManager *CreateDefaultPhysicsManager(RndDir *d) {
    return new DefaultPhysicsManager(d);
}

PhysicsManagerCreator *CreatePhysicsManager = CreateDefaultPhysicsManager;
