#pragma once
#include "os/NetworkSocket.h"
#include "os/Timer.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"

enum HttpGetFailType {
    kHttpGetFail_Unknown = 0x0000,
    kHttpGetFail_SocketError = 0x0001,
    kHttpGetFail_TimeOut = 0x0002,
    kHttpGetFail_ClientError = 0x0003,
    kHttpGetFail_ServerError = 0x0004
};

class HttpGet {
public:
    enum State {
        kS_Connecting = 0x0000,
        kS_StartSending = 0x0001,
        kS_Sending = 0x0002,
        kS_WaitToReceive = 0x0003,
        kS_Receiving = 0x0004,
        kS_Done = 0x0005,
        kS_Failure = 0x0006,
        kS_FailureRetry = 0x0007,
        kS_WaitToSend = 0x0008,
        kS_Max = 0x0009,
        kS_Nil = -1
    };

    HttpGet(unsigned int ip, unsigned short port, const char *, const char *);
    HttpGet(
        unsigned int ip, unsigned short port, const char *, unsigned char, const char *
    );
    virtual ~HttpGet();
    virtual void SetContent(const char *content) {}
    virtual void SetContentLength(unsigned int len) {}

    bool IsDownloaded();
    bool HasFailed();
    char *DetachBuffer();
    void Send();
    void Poll();
    unsigned int GetBufferSize();
    void SetTimeout(float);
    HttpGetFailType FailType() const { return mFailType; }
    State PrevState() const { return mFailState; }
    int GetRetryCount() { return mRetryCount; }

    MEM_OVERLOAD(HttpGet, 0x1C);

protected:
    virtual bool CanRetry();
    virtual void StartSending();
    virtual void Sending() {
        MILO_FAIL("HttpGet::Sending() - shouldn't be calling this");
    }

    void StartReceiving();
    void SafeDisconnect();
    void SafeShutdown();
    void StartConnection();
    bool HasTimedOut();
    void SetState(State);

    void SetFailType(HttpGetFailType type) { mFailType = type; }

    NetworkSocket *mSocket; // 0x8
    String mStrRemoteFile; // 0xc
    unsigned short mPort; // 0x14
    State mState; // 0x18
    unsigned char mIgnoreResponse; // 0x1c

private:
    void AddRequiredHeaders();

    static const float kDefaultTimeoutMs;
    static const int kMaxRetries;
    static const int kRecvBufSize;

    Timer mTimer; // 0x20
    float mTimeoutMs; // 0x50
    unsigned int mIP; // 0x54
    String mOptionalHeader; // 0x58
    char *mRecvBuf; // 0x60
    int mRecvBufPos; // 0x64
    unsigned int mStatusCode; // 0x68
    char *mFileBuf; // 0x6c
    int mFileBufSize; // 0x70
    int mFileBufRecvPos; // 0x74
    int mRetryCount; // 0x78
    HttpGetFailType mFailType; // 0x7c
    State mFailState; // 0x80
};

class HttpPost : public HttpGet {
public:
    HttpPost(unsigned int, unsigned short, const char *, unsigned char);
    virtual ~HttpPost();
    virtual void SetContent(const char *content) { mContent = content; }
    virtual void SetContentLength(unsigned int);

protected:
    virtual bool CanRetry();
    virtual void StartSending();
    virtual void Sending();

    const char *mContent; // 0x88
    unsigned int mContentLength; // 0x8c
    int mContentBytesLeftToSend; // 0x90

private:
    String mHeader; // 0x94
    int mHeaderLength;
};
