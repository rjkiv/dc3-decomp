#include "HolmesUtl.h"
#include "os/File.h"
#include "utl/MakeString.h"
#include "xdk/xbdm/xbdm.h"

String HolmesXboxPath(const char *cc1, const char *cc2) {
    String s;
    DmMapDevkitDrive();
    FileQualifiedFilename(s, cc2);
    s = MakeString("devkit:\\holmes\\%s\\%s", cc1, s);

    // Convert path characters: ':' -> '_', validate '\' positions
    char *startPos = (char *)s.c_str() + 7; // After "devkit:"
    unsigned char c = ((unsigned char *)s.c_str())[7];
    char *currentPos = startPos;

    while (c != '\0') {
        // Replace ':' with '_'
        if (c == ':') {
            *currentPos = '_';
        }

        // Reload character and check for backslash
        c = *currentPos;
        if (c == '\\') {
            // Check segment length
            if (currentPos - startPos - 1 > 42) {
                return String();
            }
            startPos = currentPos;
        }

        // Pre-increment (matches lbzu r9, 0x1(r11))
        c = *++currentPos;
    }

    // Check final segment length
    if (currentPos - startPos - 1 > 42) {
        return String();
    }

    return s;
}
