#include "net/HttpGet.h"
#include "os/Debug.h"
#include "os/NetworkSocket.h"
#include "stl/_vector.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"

const float HttpGet::kDefaultTimeoutMs = 5000.0f;
const int HttpGet::kMaxRetries = 3;
const int HttpGet::kRecvBufSize = 0x1000;

namespace {
    bool ValidateHeader(char *, int, int *, int *) { return false; }
    char *GetNextLine(char *, int *) { return 0; }
    int LineLength(char *, int) { return 1; }
    bool StrIStartsWith(String const &, const char *) { return false; }
    char *ParseHeader(char *p, int lineLen, std::vector<String> *pHeader) {
        MILO_ASSERT(pHeader, 0x83);
        int count = (((int **)pHeader)[1] - ((int **)pHeader)[0]) >> 3;
        if (count > 0) {
            int idx = 0;
            while (count != 0) {
                int len = LineLength(p, lineLen);
                MILO_ASSERT(len > 0, 0x8C);
                (*pHeader)[idx].resize(len + 1);
                strncpy((char *)(*pHeader)[idx].c_str(), p, len);
                (*pHeader)[idx].erase(len);
                count = count - 1;
                p = GetNextLine(p, &lineLen);
                idx = idx + 1;
            }
        }
        return p;
    }

    unsigned int ParseStatusCode(std::vector<String> const &lines) {
        String status;

        if ((StrIStartsWith(lines[0], "HTTP/1.0") == 0) && (StrIStartsWith(lines[0], "HTTP/1.1") == 0)) {
            return 0;
        }

        const char *ptr = lines[0].c_str();
        ptr += 8;

        char c = *ptr;
        while (((c < '0') || (c > '9')) && (c != '\0') && (c != '\n')) {
            ptr++;
            c = *ptr;
        }

        if (c >= '0') {
            do {
                status += c;
                ptr++;
                c = *ptr;
            } while ((c >= '0') && (c <= '9'));
        }

        if (status.c_str()[0] == '\0') {
            return 0;
        }

        return atoi(status.c_str());
    }

    int GetContentLength(std::vector<String> const &) { return 1; }
};

HttpGet::HttpGet(unsigned int ip, unsigned short port, const char *c1, const char *c2)
    : mSocket(nullptr), unkc(c1), mPort(port), mState(-1), unk1c(false),
      mTimeoutMs(kDefaultTimeoutMs), mIP(ip), unk58(c2), unk60(nullptr), mRecvBufPos(0),
      mFileBuf(nullptr), mFileBufSize(0), mFileBufRecvPos(0), unk78(0), mFailType(),
      mPrevState(kHttpGet_Nil) {
    SetState((State)0);
    AddRequiredHeaders();
}

HttpGet::HttpGet(
    unsigned int ip, unsigned short port, const char *c1, unsigned char uc, const char *c2
)
    : mSocket(nullptr), unkc(c1), mPort(port), mState(-1), unk1c(uc & 3),
      mTimeoutMs(kDefaultTimeoutMs), mIP(ip), unk58(c2), unk60(nullptr), mRecvBufPos(0),
      mFileBuf(nullptr), mFileBufSize(0), mFileBufRecvPos(0), unk78(0), mFailType() {
    SetState((uc & 4) == 0 ? (State)8 : (State)0);
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
    String str = "GET ";
    str += unkc;
    str += " ";
    str += "HTTP/1.1";
    if (!unk58.empty()) {
        str += "\r\n";
        str += unk58;
    }
    str += "\r\n\r\n";
    int len = (int)str.length();
    if (mSocket->Send(str.c_str(), len) != len) {
        mFailType = (HttpGetFailType)1;
        SetState((State)7);
    } else {
        SetState((State)3);
    }
}

void HttpGet::SafeShutdown() {
    SafeDisconnect();
    if (mFileBuf) {
        MemFree(mFileBuf);
        mFileBuf = nullptr;
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
    }
    char *buffer = mFileBuf;
    mFileBuf = nullptr;
    return buffer;
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
    MILO_ASSERT(!mSocket, 0x2FF);
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
    }
    return false;
}

void HttpPost::StartSending() {
    MILO_ASSERT(mSocket, 0x3CD);
    if (mSocket->CanSend()) {
        unk9c = unk94.length();
        if (mSocket->Send(unk94.c_str(), unk9c) == unk9c) {
            SetState((State)2);
            return;
        }
    }
    mFailType = (HttpGetFailType)1;
    SetState((State)7);
}
