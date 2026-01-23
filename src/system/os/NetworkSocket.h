#pragma once
#include "utl/Str.h"

class NetAddress {
public:
    NetAddress(unsigned int ip, unsigned short port) : mIP(ip), mPort(port) {}
    NetAddress() : mIP(0), mPort(0) {}
    unsigned int mIP;
    unsigned short mPort;
};

class NetworkSocket {
public:
    NetworkSocket() {}
    virtual ~NetworkSocket() {}
    virtual bool Connect(unsigned int ip, unsigned short port) = 0;
    virtual bool Fail() const = 0;
    virtual void Disconnect() = 0;
    virtual void Bind(unsigned short port) = 0;
    virtual int InqBoundPort(unsigned short &) const = 0;
    virtual void Listen() = 0;
    virtual NetworkSocket *Accept() = 0;
    virtual void GetRemoteIP(unsigned int &ip, unsigned short &port) = 0;
    virtual bool CanSend() const = 0;
    virtual bool CanRead() const = 0;
    virtual int Send(const void *data, unsigned int len) = 0;
    virtual int Recv(void *data, unsigned int len) = 0;
    virtual int
    SendTo(const void *data, unsigned int len, unsigned int ip, unsigned short port) = 0;
    virtual int BroadcastTo(const void *data, unsigned int len, unsigned short port) = 0;
    virtual int
    RecvFrom(void *data, unsigned int maxLen, unsigned int &ip, unsigned short &port) = 0;
    virtual bool SetNoDelay(bool enabled) = 0;

    static String GetHostName();
    static NetworkSocket *Create(bool streaming);
    static NetAddress SetIPPortFromHostPort(
        const char *host_port, const char *domain, unsigned short default_port
    );
    static unsigned int IPStringToInt(const String &ip);
    static String IPIntToString(unsigned int ip);
    static unsigned int ResolveHostName(String name);
};
