#include "os/Joypad_Xinput.h"
#include "Joypad_Xinput.h"
#include "obj/Data.h"
#include "os/Joypad.h"
#include "os/User.h"
#include "os/UserMgr.h"


float deadzone;
UserMgr* user;

void JoypadInitXboxPCDeadzone(DataArray *arr){
    const char * dz = "deadzone";
    bool b = arr->FindData(dz, deadzone,true);
    //not sure what b does after this point
    deadzone *= (1.0f/256.0f);
}

//not sure what the bool's signify yet
void TranslateStick(char *keys, short s, bool param_a, bool param_b){

    float var1 = (s+0.5f) / 32768.0f;

    if(param_b){
        if(var1 <= deadzone){
            if(-deadzone <= var1){
                var1=0.0;
            }
            var1 += deadzone;
        }
        else{
            var1 -= deadzone;
        }
        var1 /= (1.0-deadzone);
    }
    
    int c = (var1*127.0);
    *keys = c;

    if(param_a){
        *keys = -c;
    }
}

void TranslateButtons(unsigned int * buttons, unsigned short s){
    static int var2[16] = {
        0xC, 0xE, 0xF, 0xD, 
        0xB, 8, 9, 0xA, 
        2, 3, 0, 0, 
        6, 5, 7, 4};
    *buttons = 0;

    for(int i=0;i<16;i++){
        if(s & 1 << i){
            *buttons = 1 << var2[i]| *buttons;
        }
    }
}

bool JoypadGetCachedXInputCaps(int i, struct _XINPUT_CAPABILITIES * xinput, bool b){
    int var1;

    return true;
}

JoypadType ReadSingleXinputJoypad(int , int , unsigned int * , char *, char *, char *, char *, 
    char *, char *, char *, float *, float *, unsigned char *){

    JoypadType j;    
    return j;

}

void JoypadResetXboxPC(int p){
    //ResetAllUsersPads(); Function doesn't exist yet
}
