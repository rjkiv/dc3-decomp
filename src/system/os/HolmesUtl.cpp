#include "HolmesUtl.h"
#include "os/File.h"
#include "utl/MakeString.h"
#include "xdk/xbdm/xbdm.h"

String HolmesXboxPath(const char *hostName, const char *filePath) {
    String path;
    DmMapDevkitDrive();
    FileQualifiedFilename(path, filePath);
    path = MakeString("devkit:\\holmes\\%s\\%s", hostName, path);
    char *start = (char *)&path.c_str()[7];
    char *ptr = (char *)&path.c_str()[7];
    for (; *ptr != '\0'; ptr++) {
        if (*ptr == ':') {
            *ptr = '_';
        }
        if (*ptr == '\\') {
            int diff = ptr - start - 1;
            if (diff > 42) {
                return 0;
            }
            start = ptr;
        }
    }
    if (ptr - start - 1 > 42) {
        return 0;
    } else {
        return path;
    }
}
