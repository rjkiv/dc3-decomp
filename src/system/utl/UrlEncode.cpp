#include "UrlEncode.h"
#include <utl/Str.h>

namespace {
    bool IsCharInString(char c, char const *str) {
        int length = strlen(str);
        for (int i = 0; i < length; ++i) {
            if (c == str[i]) {
                return true;
            }
        }
        return false;
    }
}

void URLEncode(char const *input, String &output, bool escapeUnsafe) {
    char const *reserved = "$&+,/:;=?@\"<>";
    char const *unsafe = "#%{}|\\^~[]`";
    char const *hexmap = "0123456789ABCDEF";
    int length = strlen(input);

    for (int i = 0; i < length; ++i) {
        char c = input[i];
        if (IsCharInString(c, unsafe) || IsCharInString(c, reserved) || c < ' '
            || c > '~') {
            output += '%';

            if (escapeUnsafe && (c < ' ' || c > '~')) {
                output += "2";
            } else {
                output += hexmap[c >> 4];
                c = hexmap[c & 0xf];
            }
            output += c;
        }
    }
}
