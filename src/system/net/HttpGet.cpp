#include "net/HttpGet.h"
#include "HttpGet.h"
#include "macros.h"
#include "os/Debug.h"
#include "os/NetworkSocket.h"
#include "stl/_vector.h"
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

HttpGet::HttpGet(
    unsigned int ip, unsigned short port, const char *strRemoteFile, const char *strHeader
)
    : mSocket(0), mStrRemoteFile(strRemoteFile), mPort(port), mState(kS_Nil),
      mIgnoreResponse(0), mTimeoutMs(kDefaultTimeoutMs), mIP(ip),
      mOptionalHeader(strHeader), mRecvBuf(0), mRecvBufPos(0), mFileBuf(0),
      mFileBufSize(0), mFileBufRecvPos(0), mRetryCount(0), mFailType(),
      mFailState(kS_Nil) {
    SetState(kS_Connecting);
    AddRequiredHeaders();
}

HttpGet::HttpGet(
    unsigned int ip,
    unsigned short port,
    const char *strRemoteFile,
    unsigned char flags,
    const char *strHeader
)
    : mSocket(0), mIP(ip), mPort(port), mStrRemoteFile(strRemoteFile),
      mIgnoreResponse(flags & 3), mOptionalHeader(strHeader), mState(kS_Nil),
      mTimeoutMs(kDefaultTimeoutMs), mRecvBuf(0), mRecvBufPos(0), mFileBuf(0),
      mFileBufSize(0), mFileBufRecvPos(0), mRetryCount(0),
      mFailType(kHttpGetFail_Unknown) {
    if (flags & 4) {
        SetState(kS_Connecting);
    } else {
        SetState(kS_WaitToSend);
    }
    AddRequiredHeaders();
}

HttpGet::~HttpGet() { SafeShutdown(); }

void HttpGet::StartSending() {
    MILO_ASSERT(mSocket, 0x311);
    if (!mSocket->CanSend()) {
        mFailType = kHttpGetFail_SocketError;
        SetState(kS_FailureRetry);
        return;
    }
    String str("GET ");
    str += mStrRemoteFile;
    str += " ";
    str += "HTTP/1.1";
    if (!mOptionalHeader.empty()) {
        str += "\r\n";
        str += mOptionalHeader;
    }
    str += "\r\n\r\n";
    int len = str.length();
    if (mSocket->Send(str.c_str(), len) != len) {
        mFailType = kHttpGetFail_SocketError;
        SetState(kS_FailureRetry);
        return;
    }
    SetState(kS_WaitToReceive);
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
    if (mState == kS_WaitToSend) {
        SetState(kS_Connecting);
    }
}

bool HttpGet::IsDownloaded() { return mState == kS_Done; }
bool HttpGet::HasFailed() { return mState == kS_Failure; }

char *HttpGet::DetachBuffer() {
    if (mState != kS_Done) {
        return nullptr;
    } else {
        char *buffer = mFileBuf;
        mFileBuf = nullptr;
        return buffer;
    }
}

void HttpGet::StartReceiving() {
    if (mRecvBuf) {
        MemFree(mRecvBuf, __FILE__, 0x344);
        mRecvBuf = nullptr;
    }
    mRecvBuf = (char *)_MemAllocTemp(0x1000, __FILE__, 0x346, "HttpGet", 0);
}

void HttpGet::SafeDisconnect() {
    if (mSocket) {
        mSocket->Disconnect();
        RELEASE(mSocket);
    }
    if (mRecvBuf) {
        MemFree(mRecvBuf, __FILE__, 0x351);
        mRecvBuf = nullptr;
    }
    mRecvBufPos = 0;
}

void HttpGet::StartConnection() {
    MILO_ASSERT(mSocket == NULL, 0x2FF);
    mSocket = NetworkSocket::Create(true);
    if (mSocket->Fail()) {
        mFailType = kHttpGetFail_SocketError;
        SetState(kS_Failure);
    } else {
        mSocket->Connect(mIP, mPort);
    }
}

bool HttpGet::HasTimedOut() {
    mTimer.Split();
    return mTimer.Ms() > mTimeoutMs;
}

void HttpGet::SetTimeout(float timeout) { mTimeoutMs = timeout; }

HttpPost::HttpPost(unsigned int ip, unsigned short port, const char *cc, unsigned char uc)
    : HttpGet(ip, port, cc, uc, nullptr) {
    String newLine;
    newLine = MakeString("\r\n");
    String post("POST ");
    post += mStrRemoteFile.c_str();
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
    mHeader = post.c_str();
}

HttpPost::~HttpPost() {}

void HttpPost::SetContentLength(unsigned int len) {
    MILO_ASSERT(mContent, 0x3C1);
    mContentLength = len;
    mContentBytesLeftToSend = len;
    mHeader += "Content-Length: ";
    mHeader += MakeString("%d\r\n", mContentLength);
    mHeader += MakeString("\r\n");
}

bool HttpPost::CanRetry() {
    if (GetRetryCount() < 3) {
        mContentBytesLeftToSend = mContentLength;
        return true;
    } else {
        return false;
    }
}

void HttpPost::StartSending() {
    MILO_ASSERT(mSocket, 0x3CD);
    if (!mSocket->CanSend()) {
        SetFailType(kHttpGetFail_SocketError);
        SetState(kS_FailureRetry);
    } else {
        mHeaderLength = mHeader.length();
        if (mSocket->Send(mHeader.c_str(), mHeaderLength) != mHeaderLength) {
            SetFailType(kHttpGetFail_SocketError);
            SetState(kS_FailureRetry);
        } else {
            SetState(kS_Sending);
        }
    }
}

void HttpPost::Sending() {
    MILO_ASSERT(mSocket, 0x3ef);
    String str;
    for (int i = (int)mContentLength - mContentBytesLeftToSend; i < (int)mContentLength;
         i++) {
        str += MakeString("%c", mContent[i]);
    }

    int socketSend = mSocket->Send(
        mContent + mContentLength - mContentBytesLeftToSend, mContentBytesLeftToSend
    );
    State s;
    if (socketSend == -1) {
        s = kS_FailureRetry;
        SetFailType(kHttpGetFail_SocketError);
    } else if (socketSend != mContentBytesLeftToSend) {
        mContentBytesLeftToSend -= socketSend;
        return;
    } else {
        s = kS_WaitToReceive;
        mContentBytesLeftToSend = 0;
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
    mOptionalHeader += header.c_str();
}

bool HttpGet::CanRetry() { return mRetryCount < 3; }

unsigned int HttpGet::GetBufferSize() { return mFileBufSize; }

void HttpGet::SetState(State s) {
    if (mState == s)
        return;
    switch (mState) {
    case kS_Connecting:
        if (s != kS_StartSending)
            SafeShutdown();
        break;
    case kS_StartSending:
        if (s == kS_Sending)
            break;
    case kS_Sending:
        if (s != kS_WaitToReceive)
            SafeShutdown();
        break;
    case kS_WaitToReceive:
        if (s != kS_Receiving)
            SafeShutdown();
        break;
    case kS_Receiving:
        if (s != kS_Done)
            SafeShutdown();
        else
            SafeDisconnect();
        break;
    }
    if ((s == kS_Failure || s == kS_FailureRetry) && (mState != kS_Failure)
        && (mState != kS_FailureRetry)) {
        mFailState = mState;
    }

    mState = s;
    mTimer.Restart();

    switch (s) {
    case kS_Nil:
        SafeShutdown();
        break;
    case kS_Connecting:
        StartConnection();
        break;
    case kS_StartSending:
        StartSending();
        break;
    case kS_Receiving:
        StartReceiving();
        break;
    case kS_Failure:
        return;
    case kS_FailureRetry:
        if (CanRetry()) {
            mRetryCount++;
            SetState(kS_Connecting);
        } else {
            SetState(kS_Failure);
        }
        break;
    }
}
