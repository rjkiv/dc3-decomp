#pragma once
#include "os/NetworkSocket.h"
#include "os/Timer.h"
#include "utl/MemMgr.h"
#include "utl/Str.h"

enum HttpGetFailType {
};

class HttpGet {
public:
    enum State {
        kHttpGet_Nil = -1,
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
    State PrevState() const { return mPrevState; }

    MEM_OVERLOAD(HttpGet, 0x1C);

private:
    void AddRequiredHeaders();

    static const float kDefaultTimeoutMs;
    static const int kMaxRetries;
    static const int kRecvBufSize;

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

    NetworkSocket *mSocket; // 0x8
    String unkc; // 0xc
    unsigned short mPort; // 0x14
    int mState; // 0x18
    bool unk1c;
    Timer unk20;
    float mTimeoutMs; // 0x50
    unsigned int mIP; // 0x54
    String unk58;
    void *unk60;
    int mRecvBufPos; // 0x64
    u32 unk68;
    char *mFileBuf; // 0x6c
    int mFileBufSize; // 0x70
    int mFileBufRecvPos; // 0x74
    int unk78;
    HttpGetFailType mFailType; // 0x7c
    State mPrevState; // 0x80
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
    int unk90; // 0x90
    String unk94; // 0x94
    int unk9c;
};
