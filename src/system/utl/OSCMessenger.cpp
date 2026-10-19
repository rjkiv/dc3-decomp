#include "utl/OSCMessenger.h"
#include "os/Debug.h"
#include "os/HolmesClient.h"
#include "os/NetworkSocket.h"
#include "os/System.h"
#include "utl/Std.h"

OSCMessenger TheOSCMessenger;

OSCMessenger::~OSCMessenger() {
    delete mSocketIn;
    delete mSocketOut;
}

void OSCMessenger::Connect() {
    if (!UsingCD()) {
        int ip = HolmesResolveIP().GetIP();
        if (ip != 0) {
            mSocketIn = NetworkSocket::Create(false);
            mSocketIn->Bind(12346);
            mSocketIn->Listen();
            mSocketOut = NetworkSocket::Create(false);
            mSocketOut->Connect(ip, 12345);
        }
    }
}

void OSCMessenger::Poll() {
    if (mSocketIn && mSocketIn->CanRead()) {
        char data[0x80];
        mSocketIn->Recv(data, 0x80);
        char str[0x80];
        strncpy(str, data, 0x80);
        str[0x7f] = 0;
        int pos = strlen(str) / 4 + 1;
        OSCValue value;
        value.mAddress = str;
        value.mFresh = 1;
        MILO_ASSERT(data[pos] == ',', 0x46);
        char c4 = data[pos + 1];
        if (c4 == 's') {
            MILO_ASSERT(data[pos+2] == 0, 0x49);
            MILO_ASSERT(data[pos+3] == 0, 0x4A);
            strncpy(value.mValue.s, &data[pos + 4], 0x80);
            value.mType = 's';
        } else if (c4 == 'i') {
            MILO_ASSERT(data[pos+2] == 0, 0x51);
            MILO_ASSERT(data[pos+3] == 0, 0x52);
            value.mType = 'i';
            value.mValue.i = data[pos + 4];
        } else if (c4 == 'f') {
            if (data[pos + 2] == 'f' && data[pos + 3] == 'f') {
                value.mType = 'v';
                float *dataIn = (float *)data;
                value.mValue.v[0] = dataIn[1];
                value.mValue.v[1] = dataIn[2];
                value.mValue.v[1] = dataIn[3];
            } else {
                MILO_ASSERT(data[pos+2] == 0, 0x67);
                MILO_ASSERT(data[pos+3] == 0, 0x68);
                value.mType = 'f';
                value.mValue.f = data[pos + 4];
            }
        }
        bool found = false;
        FOREACH (it, mValues) {
            if (it->mAddress == str) {
                memcpy(&it->mValue, &value.mValue, 0x80);
                found = true;
                it->mFresh = true;
                break;
            }
        }
        if (!found) {
            mValues.push_back(value);
        }
    }
}

int OSCMessenger::GetInt(String address, int intValue) {
    OSCValue *val = GetValue(address);
    if (val) {
        MILO_ASSERT(val->mType == 'i', 0x131);
        val->mFresh = 0;
        return val->mValue.i;
    } else {
        OSCValue newValue;
        newValue.mAddress = address;
        newValue.mValue.i = intValue;
        newValue.mFresh = false;
        newValue.mType = 'i';
        mValues.push_front(newValue);
        return intValue;
    }
}

float OSCMessenger::GetFloat(String address, float floatValue) {
    OSCValue *val = GetValue(address);
    if (val) {
        MILO_ASSERT(val->mType == 'f', 0x149);
        val->mFresh = 0;
        return val->mValue.f;
    } else {
        OSCValue newValue;
        newValue.mAddress = address;
        newValue.mValue.f = floatValue;
        newValue.mFresh = false;
        newValue.mType = 'f';
        mValues.push_front(newValue);
        return floatValue;
    }
}

void OSCMessenger::SendOSCFloat(String address, float floatValue) {
    char message[256];
    if (mSocketOut) {
        int addr = MakeOSCAddress(address, message);
        message[addr++] = ',';
        message[addr++] = 'f';
        message[addr++] = '\0';
        message[addr++] = '\0';
        memcpy(&message[addr], &floatValue, sizeof(float));
        mSocketOut->Send(message, addr + 4);
    }
}

OSCMessenger::OSCValue *OSCMessenger::GetValue(String address) {
    FOREACH (it, mValues) {
        if (it->mAddress == address) {
            return &*it;
        }
    }
    return nullptr;
}

int OSCMessenger::MakeOSCAddress(String address, char *out) {
    int len = address.length();
    strncpy(out, address.c_str(), 0x80);
    int mod = len % 4;
    memset(&out[len], 0, 4 - mod);
    return len - mod + 4;
}
