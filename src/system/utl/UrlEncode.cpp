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

const char *const reserved = "$&+,/:;=?@";
const char *const forbidden = " \"<>#%{}|\\^~[]`";
const char *const hexstr = "0123456789ABCDEF";

void URLEncode(char const *input, String &output, bool escapeUnsafe) {
    int length = strlen(input);
    for (int i = 0; i < length; i++) {
        char c = input[i];
        if (!IsCharInString(c, forbidden) && !IsCharInString(c, reserved) && c >= ' '
            && c <= '~') {
            output += c;
        } else {
            output += "%";
            if (escapeUnsafe && (c < ' ' || c > '~')) {
                output += hexstr[(' ' >> 4) & 0xF];
                output += hexstr[(' ') & 0xF];
            } else {
                output += hexstr[(c >> 4) & 0xF];
                output += hexstr[c & 0xF];
            }
        }
    }
}
