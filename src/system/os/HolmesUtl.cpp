#include "HolmesUtl.h"
#include "os/File.h"
#include "utl/MakeString.h"
#include "xdk/xbdm/xbdm.h"

String HolmesXboxPath(const char *cc1, const char *cc2) {
    String s;
    DmMapDevkitDrive();
    FileQualifiedFilename(s, cc2);
    s = MakeString("devkit:\\holmes\\%s\\%s", cc1, s);
    // turns fwdslash -> backslash
    // i don't wanna write this. so i won't :)
    return s;
}
