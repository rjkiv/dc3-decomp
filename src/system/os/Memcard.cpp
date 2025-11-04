#include "os/Memcard.h"
#include "obj/Dir.h"
#include "os/Debug.h"

void Memcard::Init() { SetName("memcard", ObjectDir::Main()); }

void Memcard::ShowDeviceSelector(const ContainerId &, Hmx::Object *obj, int, bool) {
    if (obj) {
        DeviceChosenMsg msg(-1);
        obj->Handle(msg, false);
    }
}

void Memcard::DestroyContainer(MCContainer *pContainer) {
    MILO_ASSERT(pContainer, 0x34);
    MILO_ASSERT(!pContainer->IsMounted(), 0x35);
    delete pContainer;
}

void MCContainer::DestroyMCFile(MCFile *pFile) {
    MILO_ASSERT(pFile, 0x45);
    if (pFile->IsOpen()) {
        pFile->Close();
    }
    delete pFile;
}
