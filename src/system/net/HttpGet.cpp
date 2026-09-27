#include "net/HttpGet.h"
#include "macros.h"
#include "os/Debug.h"
#include "os/NetworkSocket.h"
#include "utl/MakeString.h"
#include "utl/MemMgr.h"
#include "utl/Std.h"
#include "utl/Str.h"
#include <cstdlib>
#include <cstring>

const float HttpGet::kDefaultTimeoutMs = 5000.0f;
const int HttpGet::kMaxRetries = 3;
const int HttpGet::kRecvBufSize = 0x1000;

namespace {
    bool ValidateHeader(char *c, int i1, int *i2, int *i3);
    char *GetNextLine(char *c, int *i) {
        if (c && i) {
            int temp = *i;
            while (temp > 0) {
                if (*c == '\r' || *c == '\n') {
                    break;
                }
                temp--;
                c++;
            }
            if (temp > 0 && *c == '\r') {
                c++;
                temp--;
            }
            if (temp > 0 && *c == '\n') {
                c++;
                temp--;
            }
            *i = temp;
            return temp > 0 ? c : 0;
        }
        return 0;
    }
    int LineLength(char *pBuf, int i) {
        MILO_ASSERT(pBuf, 0x54);
        char *original = pBuf;
        for (; i > 0; i--) {
            if (*pBuf == '\r' || *pBuf == '\n')
                break;
            pBuf++;
        }
        return pBuf - original;
    }
    bool StrIStartsWith(String const &, const char *);
    char *ParseHeader(char *c, int val, std::vector<String> *pHeader) {
        MILO_ASSERT(pHeader, 0x83);
        int size = pHeader->size();
        if (size > 0) {
            for (int i = 0; size != 0; i++) {
                int lineLen = LineLength(c, val);
                MILO_ASSERT(lineLen > 0, 0x8c);
                (*pHeader)[i].resize(lineLen + 1);
                strncpy((char *)(*pHeader)[i].c_str(), c, lineLen);
                (*pHeader)[i].erase(lineLen);
                c = GetNextLine(c, &val);
                size--;
            }
        }

        return c;
    }
    unsigned int ParseStatusCode(std::vector<String> const &strings) {
        String code;
        if (StrIStartsWith(strings.front(), "HTTP/1.0")
            || StrIStartsWith(strings.front(), "HTTP/1.1")) {
            const char *str = strings[0].c_str() + 8;
            while ((*str < '0' || '9' < *str) && (*str != '\0' && *str != '\n')) {
                str++;
            }
            while ('/' <= *str && (*str >= ':')) {
                code += *str;
                str++;
            }
            if (!code.empty()) {
                return atoi(code.c_str());
            }
        } else
            return 0;
    }
    int GetContentLength(std::vector<String> const &strings) {
        int size = strings.size();
        for (int i = 1; i < size; i++) {
            if (StrIStartsWith(strings[i], "Content-Length")) {
                const char *str = strings[i].c_str() + 0xe;
                while ((*str < '0' || '9' < *str) && (*str != '\0' && *str != '\n')) {
                    str++;
                }
                return atoi(str);
            }
        }
        return -1;
    }
};

HttpGet::HttpGet(unsigned int ip, unsigned short port, const char *c1, const char *c2)
    : mSocket(0), unkc(c1), mPort(port), mState(kHttpGet_Nil), unk1c(false),
      mTimeoutMs(kDefaultTimeoutMs), mIP(ip), unk58(c2), unk60(0), mRecvBufPos(0),
      mFileBuf(0), mFileBufSize(0), mFileBufRecvPos(0), unk78(0), mFailType(),
      mPrevState(kHttpGet_Nil) {
    SetState((State)0);
    AddRequiredHeaders();
}

HttpGet::HttpGet(
    unsigned int ip, unsigned short port, const char *c1, unsigned char uc, const char *c2
)
    : mSocket(0), unkc(c1), mPort(port), mState(kHttpGet_Nil), unk1c(uc & 3),
      mTimeoutMs(kDefaultTimeoutMs), mIP(ip), unk58(c2), unk60(0), mRecvBufPos(0),
      mFileBuf(0), mFileBufSize(0), mFileBufRecvPos(0), unk78(0), mFailType() {
    State s;
    if ((uc & 4) == 0) {
        s = (State)8;
    } else {
        s = (State)0;
    }
    SetState(s);
    AddRequiredHeaders();
}

HttpGet::~HttpGet() { SafeShutdown(); }

void HttpGet::StartSending() {
    MILO_ASSERT(mSocket, 0x311);
    if (!mSocket->CanSend()) {
        mFailType = (HttpGetFailType)1;
        SetState((State)7);
        return;
    }
    String str("GET ");
    str += unkc;
    str += " ";
    str += "HTTP/1.1";
    if (!unk58.empty()) {
        str += "\r\n";
        str += unk58;
    }
    str += "\r\n\r\n";
    int len = str.length();
    if (mSocket->Send(str.c_str(), len) != len) {
        mFailType = (HttpGetFailType)1;
        SetState((State)7);
        return;
    }
    SetState((State)3);
}

void HttpGet::SafeShutdown() {
    SafeDisconnect();
    if (mFileBuf != 0) {
        MemFree(mFileBuf, __FILE__, 0x359);
        mFileBuf = 0;
    }
    mFileBufSize = 0;
    mFileBufRecvPos = 0;
}

void HttpGet::Send() {
    if (mState == 8) {
        SetState((State)0);
    }
}

bool HttpGet::IsDownloaded() { return mState == 5; }
bool HttpGet::HasFailed() { return mState == 6; }

char *HttpGet::DetachBuffer() {
    if (mState != 5) {
        return nullptr;
    } else {
        char *buffer = mFileBuf;
        mFileBuf = nullptr;
        return buffer;
    }
}

void HttpGet::StartReceiving() {
    if (unk60) {
        MemFree(unk60, __FILE__, 0x344);
        unk60 = nullptr;
    }
    unk60 = _MemAllocTemp(0x1000, __FILE__, 0x346, "HttpGet", 0);
}

void HttpGet::SafeDisconnect() {
    if (mSocket) {
        mSocket->Disconnect();
        RELEASE(mSocket);
    }
    if (unk60) {
        MemFree(unk60, __FILE__, 0x351);
        unk60 = nullptr;
    }
    mRecvBufPos = 0;
}

void HttpGet::StartConnection() {
    MILO_ASSERT(mSocket == NULL, 0x2FF);
    mSocket = NetworkSocket::Create(true);
    if (mSocket->Fail()) {
        mFailType = (HttpGetFailType)1;
        SetState((State)6);
    } else {
        mSocket->Connect(mIP, mPort);
    }
}

bool HttpGet::HasTimedOut() {
    unk20.Split();
    return unk20.Ms() > mTimeoutMs;
}

void HttpGet::SetTimeout(float timeout) { mTimeoutMs = timeout; }

HttpPost::HttpPost(unsigned int ip, unsigned short port, const char *cc, unsigned char uc)
    : HttpGet(ip, port, cc, uc, nullptr) {
    String newLine;
    newLine = MakeString("\r\n");
    String post("POST ");
    post += unkc.c_str();
    post += " ";
    post += "HTTP/1.1";
    post += newLine;
    post += "Host: ";
    post += NetworkSocket::IPIntToString(ip);
    post += ":";
    post += MakeString("%d", mPort);
    post += newLine;
    post += "Content-Type: application/x-www-form-urlencoded";
    post += newLine;
    post += "Connection: close";
    post += newLine;
    unk94 = post.c_str();
}

HttpPost::~HttpPost() {}

void HttpPost::SetContentLength(unsigned int len) {
    MILO_ASSERT(mContent, 0x3C1);
    mContentLength = len;
    unk90 = len;
    unk94 += "Content-Length: ";
    unk94 += MakeString("%d\r\n", mContentLength);
    unk94 += MakeString("\r\n");
}

bool HttpPost::CanRetry() {
    if (unk78 < 3) {
        unk90 = mContentLength;
        return true;
    } else {
        return false;
    }
}

void HttpPost::StartSending() {
    MILO_ASSERT(mSocket, 0x3CD);
    if (!mSocket->CanSend()) {
        mFailType = (HttpGetFailType)1;
        SetState((State)7);
    } else {
        unk9c = unk94.length();
        if (mSocket->Send(unk94.c_str(), unk9c) != unk9c) {
            mFailType = (HttpGetFailType)1;
            SetState((State)7);
        } else {
            SetState((State)2);
        }
    }
}

void HttpPost::Sending() {
    MILO_ASSERT(mSocket, 0x3ef);
    String str;
    for (int i = (int)mContentLength - unk90; i < (int)mContentLength; i++) {
        str += MakeString("%c", mContent[i]);
    }

    int socketSend = mSocket->Send(mContent + mContentLength - unk90, unk90);
    State s;
    if (socketSend == -1) {
        s = (State)7;
        mFailType = (HttpGetFailType)1;
    } else if (socketSend != unk90) {
        unk90 -= socketSend;
        return;
    } else {
        s = (State)3;
        unk90 = 0;
    }
    SetState(s);
}

void HttpGet::AddRequiredHeaders() {
    String newLine;
    newLine = MakeString("\r\n");
    String header("Host: ");
    header += NetworkSocket::IPIntToString(mIP);
    header += ":";
    header += MakeString("%d", mPort);
    header += newLine;
    header += "Content-Type: application/octet-stream";
    header += newLine;
    header += "Connection: close";
    header += newLine;
    unk58 += header.c_str();
}

bool HttpGet::CanRetry() { return unk78 < 3; }

unsigned int HttpGet::GetBufferSize() { return mFileBufSize; }

void HttpGet::SetState(State s) {
    while (mState != s) {
        switch (mState) {
        case 0:
            if (s != (State)1)
                SafeShutdown();
            break;
        case 1:
            if (s == (State)2)
                break;
        case 2:
            if (s != (State)3)
                SafeShutdown();
            break;
        case 3:
            if (s != (State)4)
                SafeShutdown();
            break;
        case 4: {
            if (s == (State)5) {
                SafeDisconnect();
            } else
                SafeShutdown();
        } break;
        }
        if ((s == (State)6 || s == (State)7) && (mState != (State)6)
            && (mState != (State)7)) {
            mPrevState = mState;
        }

        mState = s;
        unk20.Restart();

        switch (s) {
        case kHttpGet_Nil:
            SafeShutdown();
            return;
        case 0:
            StartConnection();
            return;
        case 1:
            StartSending();
            return;
        case 4:
            StartReceiving();
            return;
        case 6:
            return;
        case 7: // wrong
            return;
        default: {
            if (CanRetry()) {
                s = (State)0;
                unk78++;
            } else {
                s = (State)6;
            }
        } break;
        }
    }
}
