#include "utl/Str.h"
#include "os/Debug.h"

FixedString::FixedString(char* str, int bufferSize){
    
    MILO_ASSERT(bufferSize >= 5, 0x1C);
    *unk0 = 0;
}

// FixedString * __thiscall FixedString::FixedString(FixedString *this,char *param_1,int bufferSize)

// {
//   char *pcVar1;
//   int local_20 [2];
  
//   *this = param_1 + 4;
//   if (bufferSize < 5) {
//     local_20[0] = 0x1c;
//     pcVar1 = MakeString<>(kAssertStr,"Str.cpp",local_20,"bufferSize >= 5");
//     Debug::Fail(&TheDebug,pcVar1,0x0);
//   }
//   *(*this + -4) = bufferSize + -5;
//   **this = 0;
//   return this;
// }
