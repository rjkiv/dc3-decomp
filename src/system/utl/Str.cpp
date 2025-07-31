#include "utl/Str.h"
#include "Str.h"
#include "os/Debug.h"

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
    return false;
}

// bool __thiscall String::operator==(String *this,FixedString *param_1)

// {
//   int iVar1;
//   byte *pbVar2;
//   byte *pbVar3;
  
//   pbVar2 = *(this + 4);
//   pbVar3 = param_1->mStr;
//   do {
//     iVar1 = *pbVar3 - *pbVar2;
//     if (*pbVar3 == 0) break;
//     pbVar3 = pbVar3 + 1;
//     pbVar2 = pbVar2 + 1;
//   } while (iVar1 == 0);
//   return SUB81((LZCOUNT(iVar1) << 0x20) >> 0x25,0);
// }

String::~String(){}
