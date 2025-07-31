#include "utl/Str.h"
#include "os/Debug.h"

FixedString::FixedString(char* str, int bufferSize){
    mStr = str + 4;
    MILO_ASSERT(bufferSize >= 5, 0x1C);
    int* buffer_size = (int*)(mStr - 4);
    *buffer_size = bufferSize - 5;
    *mStr = 0;
}
