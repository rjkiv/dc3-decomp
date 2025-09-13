#include "world/PhysicsManager.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/System.h"
#include "rndobj/Dir.h"
#include "rndobj/Draw.h"
#include "rndobj/Mesh.h"

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

void PhysicsManager::HarvestCollidables(ObjectDir *parentProxy) {
    MILO_ASSERT(NULL != parentProxy, 0x86);
    static Symbol collidable("collidable");
    for (ObjDirItr<RndDrawable> it(parentProxy, true); it != nullptr; ++it) {
        RndMesh *mesh = dynamic_cast<RndMesh *>(&*it);
        if (mesh) {
            const DataNode *prop = it->Property(collidable, false);
            if (prop) {
            }
        } else {
        }
    }
}

bool PhysicsManager::IsShowing(Hmx::Object *obj) {
    RndDir *dir = dynamic_cast<RndDir *>(GetCollidableDir(obj));
    if (dir)
        return false;
}
