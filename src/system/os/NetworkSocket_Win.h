#pragma once
#include "os/NetworkSocket.h"
#include "xdk/XNET.h"

class WinSockSocket : public NetworkSocket {
public:
    WinSockSocket(bool);
    virtual ~WinSockSocket();
    virtual bool Connect(unsigned int ip, unsigned short port);
    virtual bool Fail() const;
    virtual void Disconnect();
    virtual void Bind(unsigned short port);
    virtual bool InqBoundPort(unsigned short &) const;
    virtual void Listen();
    virtual NetworkSocket *Accept();
    virtual void GetRemoteIP(unsigned int &ip, unsigned short &port);
    virtual bool CanSend() const;
    virtual bool CanRead() const;
    virtual int Send(const void *data, unsigned int len);
    virtual int Recv(void *data, unsigned int len);
    virtual int
    SendTo(const void *data, unsigned int len, unsigned int ip, unsigned short port);
    virtual int BroadcastTo(const void *data, unsigned int len, unsigned short port);
    virtual int
    RecvFrom(void *data, unsigned int maxLen, unsigned int &ip, unsigned short &port);
    virtual bool SetNoDelay(bool enabled);

    static void Init();

private:
    WinSockSocket(unsigned int, bool);

protected:
    SOCKET mSocket; // 0x4
    bool mStreaming; // 0x8
    bool mFail; // 0x9

    static bool sInit;
};
