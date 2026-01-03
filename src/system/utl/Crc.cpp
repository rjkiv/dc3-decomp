#include "CRC.h"

int Hmx::CRC::ComputeHash(const char *c, unsigned int size) {
    if (size != 0 && c != 0 && *c != '\0') {
        int number = 0x811c9dc5;
        for (int i = 0; i < size; i++) {
            number = (c[i] ^ number) * 0x1000193;
        }
        number = ((number * 0x2001) >> 7 ^ number * 0x2001) * 9;
        return (number >> 0x11 ^ number) * 0x21;
    }
    return 0;
}