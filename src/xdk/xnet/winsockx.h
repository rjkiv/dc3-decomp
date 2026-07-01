#pragma once
#include "xdk/win_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IOCPARAM_MASK 0x7f
#define IOC_VOID 0x20000000
#define IOC_OUT 0x40000000
#define IOC_IN 0x800000000

#define _IOW(x, y, t)                                                                    \
    (IOC_IN | (((long)sizeof(t) & IOCPARAM_MASK) << 16) | ((x) << 8) | (y))
// #define FIONBIO _IOW('f', 126, unsigned long)

#define FIONBIO 0x8004667E

#define AF_INET 2
#define PF_INET AF_INET
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCK_STREAM 1
#define SOCK_DGRAM 2
#define IPPROTO_UDP 17
#define TCP_NODELAY 1
#define IPPROTO_TCP 6

#define SOL_SOCKET 0xffff
#define SO_SNDBUF 0x1001

#define WSABASEERR 10000
#define WSAEINTR WSABASEERR + 4
#define WSAEBADF WSABASEERR + 9
#define WSAEACCES WSABASEERR + 13
#define WSAEFAULT WSABASEERR + 14
#define WSAEINVAL WSABASEERR + 22
#define WSAEMFILE WSABASEERR + 24
#define WSAEWOULDBLOCK WSABASEERR + 35
#define WSAEINPROGRESS WSABASEERR + 36
#define WSAEALREADY WSABASEERR + 37
#define WSAENOTSOCK WSABASEERR + 38
#define WSAEDESTADDRREQ WSABASEERR + 39
#define WSAEMSGSIZE WSABASEERR + 40
#define WSAEPROTOTYPE WSABASEERR + 41
#define WSAENOPROTOOPT WSABASEERR + 42
#define WSAEPROTONOSUPPORT WSABASEERR + 43
#define WSAESOCKTNOSUPPORT WSABASEERR + 44
#define WSAEOPNOTSUPP WSABASEERR + 45
#define WSAEPFNOSUPPORT WSABASEERR + 46
#define WSAEAFNOSUPPORT WSABASEERR + 47
#define WSAEADDRINUSE WSABASEERR + 48
#define WSAEADDRNOTAVAIL WSABASEERR + 49
#define WSAENETDOWN WSABASEERR + 50
#define WSAENETUNREACH WSABASEERR + 51
#define WSAENETRESET WSABASEERR + 52
#define WSAECONNABORTED WSABASEERR + 53
#define WSAECONNRESET WSABASEERR + 54
#define WSAENOBUFS WSABASEERR + 55
#define WSAEISCONN WSABASEERR + 56
#define WSAENOTCONN WSABASEERR + 57
#define WSAESHUTDOWN WSABASEERR + 58
#define WSAETOOMANYREFS WSABASEERR + 59
#define WSAETIMEDOUT WSABASEERR + 60
#define WSAECONNREFUSED WSABASEERR + 61
#define WSAELOOP WSABASEERR + 62
#define WSAENAMETOOLONG WSABASEERR + 63
#define WSAEHOSTDOWN WSABASEERR + 64
#define WSAEHOSTUNREACH WSABASEERR + 65
#define WSAENOTEMPTY WSABASEERR + 66
#define WSAEPROCLIM WSABASEERR + 67
#define WSAEUSERS WSABASEERR + 68
#define WSAEDQUOT WSABASEERR + 69
#define WSAESTALE WSABASEERR + 70
#define WSAEREMOTE WSABASEERR + 71
#define WSASYSNOTREADY WSABASEERR + 91
#define WSAVERNOTSUPPORTED WSABASEERR + 92
#define WSANOTINITIALISED WSABASEERR + 93
#define WSAEDISCON WSABASEERR + 101
#define WSAENOMORE WSABASEERR + 102
#define WSAECANCELLED WSABASEERR + 103
#define WSAEINVALIDPROCTABLE WSABASEERR + 104
#define WSAEINVALIDPROVIDER WSABASEERR + 105
#define WSAEPROVIDERFAILEDINIT WSABASEERR + 106
#define WSASYSCALLFAILURE WSABASEERR + 107
#define WSASERVICE_NOT_FOUND WSABASEERR + 108
#define WSATYPE_NOT_FOUND WSABASEERR + 109
#define WSA_E_NO_MORE WSABASEERR + 110
#define WSA_E_CANCELLED WSABASEERR + 111
#define WSAEREFUSED WSABASEERR + 112

#define WSAHOST_NOT_FOUND WSABASEERR + 1001
#define WSATRY_AGAIN WSABASEERR + 1002
#define WSANO_RECOVERY WSABASEERR + 1003
#define WSANO_DATA WSABASEERR + 1004

typedef HANDLE WSAEVENT;

typedef struct sockaddr {
    unsigned short sa_family;
    char sa_data[14];
} sockaddr;

typedef unsigned int SOCKET;

typedef struct fd_set {
    unsigned int fd_count;
    SOCKET fd_array[64];
} fd_set;

#define FD_SETSIZE 64
#define FD_ZERO(set) (((fd_set *)(set))->fd_count = 0)
#define FD_ISSET(fd, set) __WSAFDIsSet((SOCKET)(fd), (fd_set *)(set))

#define FD_CLR(fd, set)                                                                  \
    do {                                                                                 \
        unsigned int __i;                                                                \
        for (__i = 0; __i < ((fd_set *)(set))->fd_count; __i++) {                        \
            if (((fd_set *)(set))->fd_array[__i] == fd) {                                \
                while (__i < ((fd_set *)(set))->fd_count - 1) {                          \
                    ((fd_set *)(set))->fd_array[__i] =                                   \
                        ((fd_set *)(set))->fd_array[__i + 1];                            \
                    __i++;                                                               \
                }                                                                        \
                ((fd_set *)(set))->fd_count--;                                           \
                break;                                                                   \
            }                                                                            \
        }                                                                                \
    } while (0)

#define FD_SET(fd, set)                                                                  \
    do {                                                                                 \
        unsigned int __i;                                                                \
        for (__i = 0; __i < ((fd_set *)(set))->fd_count; __i++) {                        \
            if (((fd_set *)(set))->fd_array[__i] == (fd)) {                              \
                break;                                                                   \
            }                                                                            \
        }                                                                                \
        if (__i == ((fd_set *)(set))->fd_count) {                                        \
            if (((fd_set *)(set))->fd_count < FD_SETSIZE) {                              \
                ((fd_set *)(set))->fd_array[__i] = (fd);                                 \
                ((fd_set *)(set))->fd_count++;                                           \
            }                                                                            \
        }                                                                                \
    } while (0)

struct timeval {
    long tv_sec;
    long tv_usec;
};

struct addrinfo {
    int ai_flags;
    int ai_family;
    int ai_socktype;
    int ai_protocol;
    unsigned long ai_addrlen;
    char *ai_canonname;
    struct sockaddr *ai_addr;
    struct addrinfo *ai_next;
};

// size 0x4
typedef struct in_addr {
    union {
        struct {
            unsigned char s_b1, s_b2, s_b3, s_b4;
        } s_un_b;
        struct {
            unsigned short s_w1, s_w2;
        } s_un_w;
        unsigned long s_addr;
    } s_un;
} IN_ADDR;

typedef struct sockaddr_in {
    short sin_family;
    unsigned short sin_port;
    IN_ADDR sin_addr;
    char sin_zero[8];
} sockaddr_in;

#define _SS_MAXSIZE 128
#define _SS_ALIGNSIZE (sizeof(long long))
#define _SS_PAD1SIZE (_SS_ALIGNSIZE - sizeof(short))
#define _SS_PAD2SIZE (_SS_MAXSIZE - sizeof(short) + _SS_PAD1SIZE + _SS_ALIGNSIZE)
struct sockaddr_storage {
    short ss_family;
    char __ss_pad1[_SS_PAD1SIZE];
    long long __ss_align;
    char __ss_pad2[_SS_PAD2SIZE];
};

typedef struct hostent {
    char *h_name;
    char **h_aliases;
    short h_addrtype;
    short h_length;
    char **h_addr_list;
} HOSTENT, *PHOSTEND, *LPHOSTENT;

#define WSADESCRIPTION_LEN 256
#define WSASYS_STATUS_LEN 128

typedef struct WSADATA {
    WORD wVersion;
    WORD wHighVersion;
    char szDescription[WSADESCRIPTION_LEN + 1];
    char szSystemStatus[WSASYS_STATUS_LEN + 1];
    unsigned short iMaxSockets;
    unsigned short iMaxUdpDg;
    char *lpVendorInfo;
} WSADATA, *LPWSADATA;

struct XNKEY { /* Size=0x10 */
    /* 0x0000 */ BYTE ab[16];
};

struct XNADDR { /* Size=0x24 */
    /* 0x0000 */ IN_ADDR ina;
    /* 0x0004 */ IN_ADDR inaOnline;
    /* 0x0008 */ WORD wPortOnline;
    /* 0x000a */ BYTE abEnet[6];
    /* 0x0010 */ BYTE abOnline[20];
};

struct XNKID { /* Size=0x8 */
    /* 0x0000 */ BYTE ab[8];
};

typedef struct _XSESSION_INFO { /* Size=0x3c */
    /* 0x0000 */ struct XNKID sessionID;
    /* 0x0008 */ struct XNADDR hostAddress;
    /* 0x002c */ struct XNKEY keyExchangeKey;
} XSESSION_INFO;

struct XNetStartupParams { /* Size=0xd */
    /* 0x0000 */ unsigned char cfgSizeOfStruct;
    /* 0x0001 */ unsigned char cfgFlags;
    /* 0x0002 */ unsigned char cfgSockMaxDgramSockets;
    /* 0x0003 */ unsigned char cfgSockMaxStreamSockets;
    /* 0x0004 */ unsigned char cfgSockDefaultRecvBufsizeInK;
    /* 0x0005 */ unsigned char cfgSockDefaultSendBufsizeInK;
    /* 0x0006 */ unsigned char cfgKeyRegMax;
    /* 0x0007 */ unsigned char cfgSecRegMax;
    /* 0x0008 */ unsigned char cfgQosDataLimitDiv4;
    /* 0x0009 */ unsigned char cfgQosProbeTimeoutInSeconds;
    /* 0x000a */ unsigned char cfgQosProbeRetries;
    /* 0x000b */ unsigned char cfgQosSrvMaxSimultaneousResponses;
    /* 0x000c */ unsigned char cfgQosPairWaitTimeInSeconds;
};

typedef struct XNDNS { /* Size=0x28 */
    /* 0x0000 */ INT iStatus;
    /* 0x0004 */ UINT cina;
    /* 0x0008 */ IN_ADDR aina[8];
} XNDNS;

INT XNetStartup(const struct XNetStartupParams *pxnsp);
INT XNetInAddrToString(const IN_ADDR ina, char *pchBuf, INT cchBuf);
INT XNetDnsLookup(const char *pszHost, WSAEVENT hEvent, XNDNS **ppxndns);
INT XNetDnsRelease(XNDNS *pxndns);

int WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData);
int WSACleanup(void);
WSAEVENT WSACreateEvent();

SOCKET socket(int af, int type, int protocol);
int ioctlsocket(SOCKET s, long cmd, unsigned long *argp);
int connect(SOCKET s, const sockaddr *name, int namelen);
int WSAGetLastError();
int select(
    int nfds,
    struct fd_set *readfds,
    struct fd_set *writefds,
    struct fd_set *exceptfds,
    const struct timeval *timeout
);
int shutdown(SOCKET s, int how);
int closesocket(SOCKET s);
int setsockopt(SOCKET s, int level, int optname, const char *optval, int optlen);
int bind(SOCKET s, const sockaddr *name, int namelen);
int getsockname(SOCKET s, sockaddr *name, int *namelen);
int listen(SOCKET s, int backlog);
SOCKET accept(SOCKET s, sockaddr *addr, int *addrlen);
int getpeername(SOCKET s, sockaddr *name, int *namelen);
int send(SOCKET s, const char *buf, int len, int flags);
int recv(SOCKET s, char *buf, int len, int flags);
int sendto(SOCKET s, const char *buf, int len, int flags, const sockaddr *to, int tolen);
int recvfrom(SOCKET s, char *buf, int len, int flags, sockaddr *from, int *fromlen);
unsigned long inet_addr(const char *cp);

unsigned short htons(unsigned short us) { return us; }
unsigned short ntohs(unsigned short us) { return us; }

unsigned long htonl(unsigned long ul) { return ul; }
unsigned long ntohl(unsigned long ul) { return ul; }

#ifdef __cplusplus
}
#endif
