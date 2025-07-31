#include "utl/Str.h"
#include "Str.h"
#include "os/Debug.h"

String& String::erase(){
    *mStr = 0;
    return *this;
}

FixedString::FixedString(char* str, int bufferSize){
    mStr = str + 4;
    MILO_ASSERT(bufferSize >= 5, 0x1C);
    int* buffer_size = (int*)(mStr - 4);
    *buffer_size = bufferSize - 5;
    *mStr = 0;
}

String::String(){
    
}

bool String::operator==(const FixedString& str) const {
    const char* this_p = mStr;
    const char* other_p = str.Str();
    int i1;
    do {
        i1 = *other_p - *this_p;
        if (*other_p == 0) break;
        other_p++;
        this_p++;
    } while (i1 == 0);
    return i1;
}

String::~String(){}
