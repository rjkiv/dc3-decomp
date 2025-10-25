#pragma once

#include "os/Timer.h"
#include "utl/Str.h"
class HttpPost {
public:
    virtual ~HttpPost();
    virtual void SetContent(char const *);
    virtual void SetContentLength(unsigned int);

    HttpPost(unsigned int, unsigned short, char const *, unsigned char);

protected:
    virtual bool CanRetry();
    virtual void StartSending();
    virtual void Sending();
};

class HttpGet {
public:
    enum State {
        state1,
        state2,
        state3
    };

    virtual ~HttpGet();

    HttpGet(unsigned int, unsigned short, char const *, char const *);
    HttpGet(unsigned int, unsigned short, char const *, unsigned char, char const *);
    bool IsDownloaded();
    bool HasFailed();
    char *DetachBuffer();
    void Send();
    void Poll();
    void SetTimeout(float);
    unsigned int GetBufferSize();

    int unk8;
    String unkc;
    unsigned short unk14;
    int unk18;
    bool unk1c;
    Timer unk20;
    float unk50;
    unsigned int unk54;
    String unk58;
    int unk60;
    int unk64;
    u32 unk68;
    int unk6c;
    int unk70;
    int unk74;
    int unk78;
    int unk7c;
    int unk80;

protected:
    virtual bool CanRetry();
    virtual void StartSending();
    virtual void Sending();

    void StartReceiving();
    void SafeDisconnect();
    void SafeShutdown();
    void StartConnection();
    bool HasTimedOut();
    void SetState(HttpGet::State);

private:
    void AddRequiredHeaders();

    static float const kDefaultTimeoutMs;
};
