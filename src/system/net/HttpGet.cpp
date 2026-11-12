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
#include <cctype>

const float HttpGet::kDefaultTimeoutMs = 5000.0f;
const int HttpGet::kMaxRetries = 3;
const int HttpGet::kRecvBufSize = 0x1000;

namespace {
    bool ValidateHeader(char *pBuf, int len, int *pLastBytePos, int *pLineCount) {
        bool b1 = false;
        int lineCount = 0;
        const char *p = pBuf;
        while (len > 0) {
            if (*pBuf == '\n') {
                if (b1) {
                    if (pLastBytePos) {
                        *pLastBytePos = pBuf - p;
                    }
                    if (pLineCount) {
                        *pLineCount = lineCount;
                    }
                    return true;
                }
                lineCount++;
                b1 = true;
            } else {
                b1 = *pBuf == '\r' ? b1 : false;
            }
            len--;
            pBuf++;
        }
        return false;
    }
    char *GetNextLine(char *pBuf, int *pLen) {
        if (pBuf && pLen) {
            int temp = *pLen;
            while (temp > 0) {
                if (*pBuf == '\r' || *pBuf == '\n') {
                    break;
                }
                temp--;
                pBuf++;
            }
            if (temp > 0 && *pBuf == '\r') {
                pBuf++;
                temp--;
            }
            if (temp > 0 && *pBuf == '\n') {
                pBuf++;
                temp--;
            }
            *pLen = temp;
            return temp > 0 ? pBuf : 0;
        }
        return 0;
    }
    int LineLength(char *pBuf, int len) {
        MILO_ASSERT(pBuf, 0x54);
        char *original = pBuf;
        for (; len > 0; len--) {
            if (*pBuf == '\r' || *pBuf == '\n')
                break;
            pBuf++;
        }
        return pBuf - original;
    }
    bool StrIStartsWith(const String &strLong, const char *pShort) {
        const char *str = strLong.c_str();
        unsigned int shortLen = strlen(pShort);
        if (shortLen > strlen(str)) {
            return false;
        } else if (shortLen != 0) {
            int diff = str - pShort;
            while (shortLen != 0) {
                if (tolower(pShort[diff]) != tolower(*pShort)) {
                    break;
                }
                shortLen--;
                pShort++;
            }
        }
        return shortLen == 0;
    }
    char *ParseHeader(char *pBuf, int len, std::vector<String> *pHeader) {
        MILO_ASSERT(pHeader, 0x83);
        int size = pHeader->size();
        if (size > 0) {
            for (int i = 0; size != 0; i++) {
                int lineLen = LineLength(pBuf, len);
                MILO_ASSERT(lineLen > 0, 0x8c);
                (*pHeader)[i].resize(lineLen + 1);
                strncpy((char *)(*pHeader)[i].c_str(), pBuf, lineLen);
                (*pHeader)[i].erase(lineLen);
                pBuf = GetNextLine(pBuf, &len);
                size--;
            }
        }

        return pBuf;
    }
    unsigned int ParseStatusCode(std::vector<String> const &header) {
        String status_code_str;
        if (StrIStartsWith(header.front(), "HTTP/1.0")
            || StrIStartsWith(header.front(), "HTTP/1.1")) {
            const char *p = header.front().c_str() + 8;
            char c;
            while (true) {
                c = *p;
                if (c >= '0' && c <= '9') {
                    break;
                } else if (c == '\0' || c == '\n') {
                    break;
                }
                p++;
            }
            while (c >= '0' && c <= '9') {
                status_code_str += c;
                c = *++p;
            }
            if (status_code_str.empty()) {
                return 0;
            } else {
                return atoi(status_code_str.c_str());
            }
        }
        return 0;
    }
    int GetContentLength(std::vector<String> const &header) {
        int size = header.size();
        for (int i = 1; i < size; i++) {
            if (StrIStartsWith(header[i], "Content-Length")) {
                const char *str = header[i].c_str() + 0xe;
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
    if (HttpGet::CanRetry()) {
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
    default:
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
    default:
        break;
    }
}

void HttpGet::Poll() {
    if (mState != kS_WaitToSend && mSocket) {
        if (mSocket->Fail()) {
            mFailType = kHttpGetFail_SocketError;
            SetState(kS_FailureRetry);
            return;
        } else {
            switch (mState) {
            case kS_Connecting:
                if (mSocket->CanSend()) {
                    SetState(kS_StartSending);
                } else if (HasTimedOut()) {
                    mFailType = kHttpGetFail_TimeOut;
                    SetState(kS_FailureRetry);
                }
                break;
            case kS_Sending:
                Sending();
                break;
            case kS_WaitToReceive:
                if (mSocket->CanRead()) {
                    if (mIgnoreResponse & 1) {
                        SetState(kS_Done);
                    } else {
                        SetState(kS_Receiving);
                    }
                } else if (HasTimedOut()) {
                    mFailType = kHttpGetFail_TimeOut;
                    SetState(kS_FailureRetry);
                }
                break;
            case kS_Receiving: {
                int numRecv = mSocket->Recv(mRecvBuf + mRecvBufPos, 0x1000 - mRecvBufPos);
                if (HasTimedOut()) {
                    mFailType = kHttpGetFail_TimeOut;
                    SetState(kS_FailureRetry);
                } else {
                    if (numRecv != 0) {
                        mTimer.Restart();
                        mRecvBufPos += numRecv;
                        if (!mFileBuf) {
                            int lastBytePos = 0;
                            int lineCount = 0;
                            if (ValidateHeader(
                                    mRecvBuf, mRecvBufPos, &lastBytePos, &lineCount
                                )) {
                                std::vector<String> header;
                                header.resize(lineCount);
                                ParseHeader(mRecvBuf, lastBytePos, &header);
                                int code = ParseStatusCode(header);
                                mStatusCode = code;
                                if (code != 200) {
                                    bool err400 = code >= 400 && code <= 499;
                                    if (err400) {
                                        mFailType = kHttpGetFail_ClientError;
                                    } else {
                                        bool err500 = code >= 500 && code <= 599;
                                        if (err500) {
                                            mFailType = kHttpGetFail_ServerError;
                                        } else {
                                            mFailType = kHttpGetFail_Unknown;
                                        }
                                    }
                                    SetState(kS_Failure);
                                    return;
                                } else {
                                    if (mIgnoreResponse & 2) {
                                        SetState(kS_Done);
                                        return;
                                    } else {
                                        mFileBufSize = GetContentLength(header);
                                        if (mFileBufSize < 0) {
                                            mFailType = kHttpGetFail_Unknown;
                                            SetState(kS_Failure);
                                            return;
                                        } else if (mFileBufSize == 0) {
                                            SetState(kS_Done);
                                            return;
                                        } else {
                                            mFileBuf = (char *)MemAlloc(
                                                mFileBufSize, __FILE__, 699, "HttpGet"
                                            );
                                            MILO_ASSERT(mFileBuf, 700);
                                            int i2 = lastBytePos + 1;
                                            if (mRecvBufPos > i2) {
                                                int len = mRecvBufPos - i2;
                                                MILO_ASSERT(len <= mFileBufSize, 0x2C8);
                                                memcpy(mFileBuf, mRecvBuf + i2, len);
                                                mRecvBufPos = 0;
                                                mFileBufRecvPos = len;
                                                MILO_ASSERT(mFileBufRecvPos <= mFileBufSize, 0x2CE);
                                                if (mFileBufRecvPos == mFileBufSize) {
                                                    SetState(kS_Done);
                                                }
                                            } else {
                                                mRecvBufPos = 0;
                                                mFileBufRecvPos = 0;
                                            }
                                        }
                                        return;
                                    }
                                }
                            }
                        } else {
                            MILO_ASSERT(mFileBufSize >= mFileBufRecvPos + mRecvBufPos, 0x2E1);
                            memcpy(mFileBuf + mFileBufRecvPos, mRecvBuf, mRecvBufPos);
                            mFileBufRecvPos += mRecvBufPos;
                            mRecvBufPos = 0;
                            if (mFileBufRecvPos == mFileBufSize) {
                                SetState(kS_Done);
                            }
                        }
                    }
                }
                break;
            }
            default:
                MILO_FAIL("Bad State (%d) in HttpGet::Poll\n", mState);
                break;
            }
        }
    }
}
