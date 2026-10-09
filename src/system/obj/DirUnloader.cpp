#include "obj/Dir.h"
#include "obj/DirUnloader.h"
#include "os/Debug.h"
#include "utl/Loader.h"

const char *DirUnloader::DebugText() { return MakeString("UnLoader: %s", mFile.c_str()); }

void DirUnloader::PollLoading() {
    TheLoadMgr.StartAsyncUnload();
    if (mObjects.empty()) {
        delete this;
    } else {
        Hmx::Object *obj = mObjects.back();
        if (obj)
            delete obj;
        mObjects.pop_back();
    }
    TheLoadMgr.FinishAsyncUnload();
}

DirUnloader::~DirUnloader() { MILO_ASSERT(mObjects.empty(), 0x20); }

DirUnloader::DirUnloader(ObjectDir *dir)
    : Loader(dir->GetPathName(), kLoadFront), mObjects() {
    mObjects.reserve(dir->HashSize() / 2);
    for (ObjDirItr<Hmx::Object> it(dir, false); it != 0; ++it) {
        Hmx::Object *cur = it;
        if (cur != dir) {
            cur->SetName(nullptr, nullptr);
            mObjects.push_back(ObjPtr<Hmx::Object>(this, cur));
        }
    }
}
