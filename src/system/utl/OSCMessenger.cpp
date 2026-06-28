#include "utl/OSCMessenger.h"
#include "os/Debug.h"
#include "os/HolmesClient.h"
#include "os/NetworkSocket.h"
#include "os/System.h"
#include "utl/Std.h"

OSCMessenger TheOSCMessenger;

OSCMessenger::~OSCMessenger() {
    delete mSocket1;
    delete mSocket2;
}

void OSCMessenger::Connect() {
    if (!UsingCD()) {
        int ip = HolmesResolveIP().GetIP();
        if (ip != 0) {
            mSocket1 = NetworkSocket::Create(false);
            mSocket1->Bind(0x303A);
            mSocket1->Listen();
            mSocket2 = NetworkSocket::Create(false);
            mSocket2->Connect(ip, 0x3039);
        }
    }
}

void OSCMessenger::Poll() {
    if (mSocket1 && mSocket1->CanRead()) {
        char data[0x80];
        mSocket1->Recv(data, 0x80);
        char str[0x80];
        strncpy(str, data, 0x80);
        str[0x7f] = 0;
        int pos = strlen(str) / 4 + 1;
        OSCValue value;
        value.unk0 = str;
        value.unk89 = 1;
        MILO_ASSERT(data[pos] == ',', 0x46);
        char c4 = data[pos + 1];
        if (c4 == 's') {
            MILO_ASSERT(data[pos+2] == 0, 0x49);
            MILO_ASSERT(data[pos+3] == 0, 0x4A);
            strncpy(value.buffer, &data[pos + 4], 0x80);
            value.mType = 's';
        } else if (c4 == 'i') {
            MILO_ASSERT(data[pos+2] == 0, 0x51);
            MILO_ASSERT(data[pos+3] == 0, 0x52);
            value.mType = 'i';
            value.buffer[0] = data[pos + 4];
        } else if (c4 == 'f') {
            if (data[pos + 2] == 'f' && data[pos + 3] == 'f') {
                value.mType = 'v';
                int *valueBuffer = (int *)value.buffer;
                int *dataIn = (int *)data;
                valueBuffer[1] = dataIn[2];
                valueBuffer[0] = dataIn[1];
                valueBuffer[2] = dataIn[3];
            } else {
                MILO_ASSERT(data[pos+2] == 0, 0x67);
                MILO_ASSERT(data[pos+3] == 0, 0x68);
                value.mType = 'f';
                value.buffer[0] = data[pos + 4];
            }
        }
        bool found = false;
        FOREACH (it, mValues) {
            if (it->unk0 == str) {
                memcpy(it->buffer, value.buffer, 0x80);
                found = true;
                it->unk89 = 1;
                break;
            }
        }
        if (!found) {
            mValues.push_back(value);
        }
    }
}

int OSCMessenger::GetInt(String str, int intValue) {
    OSCValue *val = GetValue(str);
    if (val) {
        MILO_ASSERT(val->mType == 'i', 0x131);
        val->unk89 = 0;
        return ((int *)val->buffer)[0];
    } else {
        OSCValue newValue;
        newValue.unk0 = str;
        ((int *)newValue.buffer)[0] = intValue;
        newValue.unk89 = 0;
        newValue.mType = 'i';
        mValues.push_front(newValue);
        return intValue;
    }
}

float OSCMessenger::GetFloat(String str, float floatValue) {
    OSCValue *val = GetValue(str);
    if (val) {
        MILO_ASSERT(val->mType == 'f', 0x149);
        val->unk89 = 0;
        return ((float *)val->buffer)[0];
    } else {
        OSCValue newValue;
        newValue.unk0 = str;
        ((float *)newValue.buffer)[0] = floatValue;
        newValue.unk89 = 0;
        newValue.mType = 'f';
        mValues.push_front(newValue);
        return floatValue;
    }
}

void OSCMessenger::SendOSCFloat(String str, float floatValue) {
    char buffer[256];
    if (mSocket2) {
        int addr = MakeOSCAddress(str, buffer);
        char *bufPtr = &buffer[addr];
        *bufPtr++ = ',';
        *bufPtr++ = 'f';
        *bufPtr++ = '\0';
        *bufPtr++ = '\0';
        memcpy(bufPtr, &floatValue, 4);
        mSocket2->Send(buffer, addr + 8);
    }
}

OSCMessenger::OSCValue *OSCMessenger::GetValue(String str) {
    FOREACH (it, mValues) {
        if (it->unk0 == str) {
            return &*it;
        }
    }
    return nullptr;
}
