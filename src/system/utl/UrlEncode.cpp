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
    int length = strlen(input);
    for (int i = 0; i < length; i++) {
        char c = input[i];
        if (!IsCharInString(c, "#%{}|\\^~[]`") && !IsCharInString(c, "$&+,/:;=?@\"<>")
            && c >= ' ' && c <= '~') {
            output += c;
        } else {
            output += "%";
            if (escapeUnsafe && !(c >= ' ' && c <= '~')) {
                output += "0123456789ABCDEF"[2];
                output += "0123456789ABCDEF"[0];
            } else {
                output += "0123456789ABCDEF"[(c >> 4) & 0xF];
                output += "0123456789ABCDEF"[c & 0xF];
            }
        }
    }
}
