#include "os/HolmesClient.h"
#include "os/NetStream.h"
#include "os/NetworkSocket.h"
#include "os/System.h"
#include "utl/BinStream.h"
#include "utl/Option.h"
#include <cstdio>

String HolmesClient::PlatformGetHostName() { return NetworkSocket::GetHostName(); }

NetAddress HolmesClient::PlatformResolveIP() {
    String name = HolmesFileHostName();
    NetAddress addr =
        NetworkSocket::SetIPPortFromHostPort(name.c_str(), "harmonixmusic.com", 4544);
    if (addr.mIP == 0 && !gHostLogging) {
        MILO_FAIL("Couldn't resolve holmes_host: %s", name);
    }
    return addr;
}

BinStream *HolmesClient::PlatformCreateServerStream(bool b1, const char *cc2) {
    std::vector<String> names;
    String curName = HolmesFileHostName();
    if (!curName.empty()) {
        names.push_back(curName);
    }
    while (true) {
        curName = OptionStr("holmes_host", "");
        if (curName.empty())
            break;
        names.push_back(curName);
    }
    while (true) {
        curName = OptionStr("xb_host", "");
        if (curName.empty())
            break;
        names.push_back(curName);
    }
    if (names.size() == 0) {
        if (b1) {
            return nullptr;
        } else {
            MILO_FAIL(
                "NO HOSTNAME PROVIDED, ADD \"-holmes_host <hostname>\" to your args"
            );
        }
    }
    printf("HolmesClientInit(host={%s", names[0].c_str());
    for (int i = 1; i < names.size(); i++) {
        printf(", %s", names[i].c_str());
    }
    printf("})\n");
    int i4 = -1;
    NetStream *ret = nullptr;
    while (true) {
        i4++;
        if (i4 >= names.size()) {
            if (b1) {
                return nullptr;
            }
            i4 = 0;
            Timer::Sleep(1000);
        }
        HolmesSetFileShare(names[i4].c_str(), cc2);
        NetAddress addr = HolmesResolveIP();
        if (addr.mIP == 0) {
            if (!b1) {
                printf(
                    "\n\nCOULD NOT RESOLVE HOST ADDRESS '%s'\n\n", HolmesFileHostName()
                );
            }
        } else {
            ret = new NetStream();
            ret->Socket()->SetNoDelay(true);
            ret->ClientConnect(addr);
            if (ret->Fail()) {
                delete ret;
                ret = nullptr;
                if (!b1) {
                    printf(
                        "\n\nCOULD NOT CONNECT TO HOLMES ADDRESS '%s'\n\n",
                        HolmesFileHostName()
                    );
                }
            }
        }
        if (ret) {
            return ret;
        }
    }
}
