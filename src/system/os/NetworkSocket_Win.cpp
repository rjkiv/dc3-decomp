#include "os/NetworkSocket_Win.h"
#include "os/Debug.h"
#include "os/NetworkSocket.h"
#include "xdk/xapilibi/handleapi.h"
#include "xdk/xapilibi/synchapi.h"
#include "xdk/xbdm/xbdm.h"
#include "xdk/xnet/winsockx.h"

bool WinSockSocket::sInit = false;

WinSockSocket::WinSockSocket(bool streaming) : mStreaming(streaming), mFail(false) {
    Init();
    if (!mStreaming) {
        mSocket = socket(AF_INET, 2, 0x11);
    } else {
        mSocket = socket(AF_INET, 1, 6);
    }
    MILO_ASSERT(mSocket != INVALID_SOCKET, 0xA8);
    unsigned long ul = 1;
    ioctlsocket(mSocket, 0x8004667E, &ul);
}

WinSockSocket::WinSockSocket(unsigned int s, bool streaming)
    : mSocket((SOCKET)s), mStreaming(streaming), mFail(false) {
    unsigned long ul = 1;
    ioctlsocket(mSocket, 0x8004667E, &ul);
}

WinSockSocket::~WinSockSocket() { Disconnect(); }

bool WinSockSocket::Connect(unsigned int ip, unsigned short port) {
    sockaddr_in addr;
    addr.sin_family = 2;
    addr.sin_port = port;
    addr.sin_addr.s_un.s_addr = ip;
    int res = connect(mSocket, &addr, 0x10);
    if (res == -1 && WSAGetLastError() != 0x2733) {
        mFail = true;
    }
    return res == 0;
}

bool WinSockSocket::Fail() const {
    timeval val;
    fd_set set;
    val.tv_sec = 0;
    val.tv_usec = 0;
    set.fd_count = 1;
    set.fd_array[0] = mSocket;
    switch (select(0, nullptr, nullptr, &set, &val)) {
    case -1:
        MILO_LOG("select returned SOCKET_ERROR %d\n", WSAGetLastError());
        break;
    case 1:
        const_cast<WinSockSocket *>(this)->mFail = true;
        break;
    default:
        break;
    }
    return mFail;
}

void WinSockSocket::Disconnect() {
    if (mSocket != INVALID_SOCKET) {
        shutdown(mSocket, 2);
        closesocket(mSocket);
        mSocket = INVALID_SOCKET;
    }
}

void WinSockSocket::Bind(unsigned short port) {
    int val = 1;
    setsockopt(mSocket, 0xFFFF, 4, (char *)&val, 4);
    sockaddr_in addr;
    addr.sin_family = 2;
    addr.sin_port = port;
    addr.sin_addr.s_un.s_addr = 0;
    int ret = bind(mSocket, &addr, 0x10);
    if (ret == -1) {
        MILO_FAIL(
            "NetworkSocket::Bind(%d) could not bind (error = %d).\nTry rebooting your computer.",
            port,
            WSAGetLastError()
        );
    }
}

bool WinSockSocket::InqBoundPort(unsigned short &port) const {
    sockaddr_in addr;
    int namelen = 16;
    if (getsockname(mSocket, &addr, &namelen) != 0) {
        return false;
    } else {
        port = addr.sin_port;
        return true;
    }
}

void WinSockSocket::Listen() { listen(mSocket, 5); }

NetworkSocket *WinSockSocket::Accept() {
    sockaddr_in addr;
    int addrlen = 16;
    SOCKET s = accept(mSocket, &addr, &addrlen);
    if (s != INVALID_SOCKET) {
        return new WinSockSocket((unsigned int)s, mStreaming);
    } else {
        return nullptr;
    }
}

void WinSockSocket::GetRemoteIP(unsigned int &ip, unsigned short &port) {
    sockaddr_in addr;
    int namelen = 16;
    getpeername(mSocket, &addr, &namelen);
    ip = addr.sin_addr.s_un.s_addr;
    port = addr.sin_port;
}

bool WinSockSocket::CanSend() const {
    fd_set write;
    write.fd_array[0] = mSocket;
    timeval val;
    val.tv_sec = 0;
    val.tv_usec = 0;
    write.fd_count = 1;
    return select(0, nullptr, &write, nullptr, &val) == 1;
}

bool WinSockSocket::CanRead() const {
    fd_set read;
    read.fd_array[0] = mSocket;
    timeval val;
    val.tv_sec = 0;
    val.tv_usec = 0;
    read.fd_count = 1;
    return select(0, &read, nullptr, nullptr, &val) == 1;
}

int WinSockSocket::Send(const void *data, unsigned int len) {
    if (!mFail) {
        int ret = send(mSocket, (const char *)data, len, 0);
        if (ret != -1) {
            return ret;
        }
        int err = WSAGetLastError();
        switch (err) {
        case 0x2733:
            break;
        case 0x2746:
        case 0x2749:
            mFail = true;
            break;
        default:
            MILO_FAIL("error in Send: %i", err);
            return -1;
            break;
        }
    }
    return 0;
}

int WinSockSocket::Recv(void *data, unsigned int len) {
    if (!mFail && CanRead()) {
        int ret = recv(mSocket, (char *)data, len, 0);
        if (ret > 0) {
            return ret;
        }
        mFail = true;
    }
    return 0;
}

int WinSockSocket::SendTo(
    const void *data, unsigned int len, unsigned int ip, unsigned short port
) {
    sockaddr_in addr;
    addr.sin_family = 2;
    addr.sin_port = port;
    addr.sin_addr.s_un.s_addr = ip;
    int res = sendto(mSocket, (const char *)data, len, 0, &addr, 16);
    if (res == -1) {
        int err = WSAGetLastError();
        if (err != 0x2733) {
            if (err != 0x2751) {
                MILO_FAIL("error in Send: %i", err);
                return -1;
            }
            if (mStreaming) {
                mFail = true;
            }
        }
        return 0;
    } else {
        return res;
    }
}

int WinSockSocket::BroadcastTo(const void *data, unsigned int len, unsigned short port) {
    int val = 1;
    setsockopt(mSocket, 0xFFFF, 32, (const char *)&val, 4);
    return SendTo(data, len, -1, port);
}

int WinSockSocket::RecvFrom(
    void *data, unsigned int maxLen, unsigned int &ip, unsigned short &port
) {
    sockaddr_in addr;
    int fromlen = 16;
    int ret = recvfrom(mSocket, (char *)data, maxLen, 0, &addr, &fromlen);
    if (ret == -1) {
        int err = WSAGetLastError();
        if (err == 0x2733) {
            return 0;
        } else {
            MILO_FAIL("error in RecvFrom: %i", err);
            port = -1;
            ip = 0;
            return 0;
        }
    } else {
        ip = addr.sin_addr.s_un.s_addr;
        port = addr.sin_port;
    }
    return ret;
}

bool WinSockSocket::SetNoDelay(bool enabled) {
    int val = enabled ? 1 : 0;
    return setsockopt(mSocket, 6, 1, (const char *)&val, 4) == 0;
}

void WinSockSocket::Init() {
    if (!sInit) {
        sInit = true;
        XNetStartupParams params;
        memset(&params, 0, sizeof(XNetStartupParams));
        params.cfgSizeOfStruct = 0xD;
        params.cfgFlags = 1;
        XNetStartup(&params);
        WSADATA data;
        WSAStartup(0x202, &data);
    }
}

NetworkSocket *NetworkSocket::Create(bool streaming) {
    return new WinSockSocket(streaming);
}

unsigned int NetworkSocket::IPStringToInt(const String &ip) {
    WinSockSocket::Init();
    return inet_addr(ip.c_str());
}

String NetworkSocket::IPIntToString(unsigned int ip) {
    WinSockSocket::Init();
    char buffer[32];
    buffer[0] = 0;
    memset(&buffer[1], 0, 31);
    in_addr addr;
    addr.s_un.s_addr = ip;
    XNetInAddrToString(addr, buffer, 32);
    return buffer;
}

String NetworkSocket::GetHostName() {
    WinSockSocket::Init();
    char buf[0x100];
    buf[0xff] = 0;
    DWORD buf_sz = 0x100;
    DmGetXboxName(buf, &buf_sz);
    MILO_ASSERT(buf[buf_sz-1] == '\0', 0x43);
    return buf;
}

unsigned int NetworkSocket::ResolveHostName(String name) {
    WinSockSocket::Init();
    WSAEVENT event = WSACreateEvent();
    unsigned int ret = 0;
    XNDNS *dns = nullptr;
    int dnsRet = XNetDnsLookup(name.c_str(), event, &dns);
    if (dnsRet == 0 && dns) {
        WaitForSingleObject(event, 10000);
        if (dns->iStatus == 0x2AF9) {
            MILO_LOG("Host %s not found.", name.c_str());
        } else if (dns->iStatus == 0x274C) {
            MILO_LOG("Host %s lookup timed out.", name.c_str());
        } else if (dns->iStatus == 0) {
            ret = dns->aina[0].s_un.s_addr;
        }
        if (XNetDnsRelease(dns) != 0) {
            MILO_LOG("could not release XNDNS");
        }
        CloseHandle(event);
        return ret;
    } else {
        MILO_LOG("XNetDnsLookup returned %d %x for %s\n", dnsRet, dns, name.c_str());
        return 0;
    }
}
