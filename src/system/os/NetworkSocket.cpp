#include "os/NetworkSocket.h"
#include "utl/Str.h"

NetAddress NetworkSocket::SetIPPortFromHostPort(
    const char *host_port, const char *domain, unsigned short default_port
) {
    NetAddress addr;
    String str(host_port);
    unsigned int idx = str.find(':');
    if (idx != FixedString::npos) {
        String str2 = str.substr(idx + 1);
        str = str.substr(0, idx);
        addr.mPort = strtol(str2.c_str(), 0, 0);
    } else {
        addr.mPort = default_port;
    }
    unsigned int ip = IPStringToInt(str);
    if (ip != -1) {
        addr.mIP = ip;
    }
    if (addr.mIP == 0) {
        if (domain) {
            str += ".";
            str += domain;
        }
        addr.mIP = ResolveHostName(str);
    }
    return addr;
}
