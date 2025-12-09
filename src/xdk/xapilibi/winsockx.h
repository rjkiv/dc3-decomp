#pragma once

#define IOCPARAM_MASK 0x7f
#define IOC_VOID 0x20000000
#define IOC_OUT 0x40000000
#define IOC_IN 0x800000000

#define _IOW(x, y, t)                                                                    \
    (IOC_IN | (((long)sizeof(t) & IOCPARAM_MASK) << 16) | ((x) << 8) | (y))
#define FIONBIO _IOW('f', 126, unsigned long)

#define AF_INET 2
#define PF_INET AF_INET
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCK_STREAM 1
#define SOCK_DGRAM 2
#define IPPROTO_UDP 17

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
struct sockaddr {
    unsigned short sa_family;
    char sa_data[14];
};

typedef unsigned int *SOCKET;

typedef struct fd_set {
    unsigned int fd_count;
    SOCKET fd_array[64];
} fd_set;

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

struct in_addr {
    union {
        struct {
            unsigned char s_b1, s_b2, s_b3, s_b4;
        } s_un_b;
        struct {
            unsigned short s_w1, s_w2;
        } s_un_w;
        unsigned long s_addr;
    } s_un;
};

struct sockaddr_in {
    short sin_family;
    unsigned short sin_port;
    struct in_addr sin_addr;
    char sin_zero[0];
};

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
#define WSASYS_STATUS_LEN
typedef unsigned short WORD;
typedef struct WSADATA {
    WORD wVersion;
    WORD wHighVersion;
    char szDescription[WSADESCRIPTION_LEN + 1];
    char szSystemStatus[WSASYS_STATUS_LEN + 1];
    unsigned short iMaxSockets;
    unsigned short iMaxUdpDg;
    char *lpVendorInfo;
} WSADATA, *LPWSADATA;
