#include "meta/Meta.h"
#include "HeldButtonPanel.h"
#include "meta/Achievements.h"
#include "meta/ConnectionStatusPanel.h"
#include "meta/DeJitterPanel.h"
#include "meta/PreloadPanel.h"
#include "obj/Object.h"
#include "os/Debug.h"

void MetaInit() {
    Achievements::Init();
    ConnectionStatusPanel::Register();
    DeJitterPanel::Register();
    HeldButtonPanel::Register();
    PreloadPanel::Register();
    TheDebug.AddExitCallback(MetaTerminate);
}

void MetaTerminate() { Achievements::Terminate(); }
