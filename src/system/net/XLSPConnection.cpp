#include "net/XLSPConnection.h"
#include "macros.h"
#include "math/Rand.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/ThreadCall.h"
#include "utl/MemMgr.h"
#include "xdk/XAPILIB.h"
#include "xdk/XNET.h"
#include "xdk/XONLINE.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/handleapi.h"
#include "xdk/xapilibi/winerror.h"
#include "xdk/xapilibi/xbase.h"
#include "xdk/xapilibi/xbox.h"
#include "xdk/xnet/winsockx.h"
#include "xdk/xnet/xnetapi.h"
#include "xdk/xonline/xonline.h"
#include <cstring>
#include <utility>

const int XLSPConnection::kTitleServerEnumMaxCount = 8;
std::map<unsigned long, int> XLSPConnection::mXLSPRefCountMap;

XLSPConnection::XLSPConnection()
    : unk4((State)-1), unk8(0), unk14(0), unk18(INVALID_HANDLE_VALUE), unk1c(0),
      unk20(0) {
    memset(&mXOverlapped, 0, sizeof(XOVERLAPPED));
    memset(&unk44, 0, sizeof(IN_ADDR));
    unk48.Reset();
    SetState((State)0);
}

XLSPConnection::~XLSPConnection() { SetState((State)-1); }

int XLSPConnection::ThreadStart() {
    if (unk4 != 5) {
        MILO_FAIL("Unhandled state %d in ThreadStart", unk4);
    } else {
        XCancelOverlapped(&mXOverlapped);
    }
    return 0;
}

void XLSPConnection::ThreadDone(int i1) {
    if (unk4 != 5) {
        MILO_FAIL("Unhandled state %d in ThreadStart", unk4);
    } else {
        memset(&mXOverlapped, 0, sizeof(XOVERLAPPED));
        CloseHandle(unk18);
        unk20 = 0;
        unk18 = INVALID_HANDLE_VALUE;
        if (unk1c) {
            MemFree(unk1c, __FILE__, 299);
            unk1c = nullptr;
        }
        SetState((State)0);
    }
}

void XLSPConnection::Connect(const char *cc, unsigned int ui) {
    unkc = cc;
    unk14 = ui;
    if (unk8 != 3) {
        unk8 = 3;
    }
    if (unk4 == 0) {
        SetState((State)1);
    }
}

void XLSPConnection::Disconnect() {
    if (unk8 != 0) {
        unk8 = 0;
    }
    if (unk4 > 0 && unk4 <= 4) {
        SetState((State)5);
    }
}

void XLSPConnection::StartEnumeration() {
    DWORD res = XTitleServerCreateEnumerator(unkc.c_str(), 8, &unk20, &unk18);
    if (res != ERROR_SUCCESS) {
        MILO_NOTIFY("XTitleServerCreateEnumerator failed with error %d", res);
        SetState((State)4);
    } else {
        unk1c = (XTITLE_SERVER_INFO *)_MemAllocTemp(
            unk20, __FILE__, 0x1CB, "XLSPConnection", 0
        );
        res = XEnumerate(unk18, unk1c, unk20, nullptr, &mXOverlapped);
        if (res != ERROR_IO_PENDING) {
            MILO_NOTIFY("XEnumerate failed with error %d", res);
            SetState((State)4);
        }
    }
}

bool XLSPConnection::SecureDisconnect(in_addr a) {
    bool ret = true;
    auto it = mXLSPRefCountMap.find(a.s_un.s_addr);
    if (it != mXLSPRefCountMap.end()) {
        it->second--;
        if (it->second == 0) {
            mXLSPRefCountMap.erase(it);
            if (XNetGetConnectStatus(a) != 3) {
                XNetUnregisterInAddr(a);
            }
        }
    } else {
        ret = false;
        MILO_NOTIFY("XLSPConnection::SecureDisconnect() - connection not found!");
    }
    return ret;
}

int XLSPConnection::StartGatewayConnection(in_addr a) {
    int ret;
    auto it = mXLSPRefCountMap.find(a.s_un.s_addr);
    if (it != mXLSPRefCountMap.end()) {
        ret = 0;
        it->second++;
    } else {
        ret = XNetConnect(a);
        if (ret == 0) {
            mXLSPRefCountMap.insert(std::make_pair(a.s_un.s_addr, 1));
        } else {
            MILO_NOTIFY(
                "XNetConnect(%d.%d.%d.%d) failed with %d",
                a.s_un.s_un_b.s_b1,
                a.s_un.s_un_b.s_b2,
                a.s_un.s_un_b.s_b3,
                a.s_un.s_un_b.s_b4,
                ret
            );
        }
    }
    return ret;
}

void XLSPConnection::Poll() {
    switch (unk4) {
    case 0:
        if (unk8 == 3)
            SetState((State)1);
        break;
    case 1: {
        if (unk18 != INVALID_HANDLE_VALUE
            && mXOverlapped.InternalLow != ERROR_IO_PENDING) {
            DWORD word = 0;
            DWORD overlapResult = XGetOverlappedResult(&mXOverlapped, &word, false);
            if (overlapResult != 0) {
                XGetOverlappedExtendedError(&mXOverlapped);
            } else {
                memset(&mXOverlapped, 0, sizeof(XOVERLAPPED));
                if (word != 0) {
                    DWORD random = RandomInt(0, word);
                    if ((unsigned int)
                            XNetServerToInAddr(unk1c[random].inaServer, unk14, &unk44)
                        == 0) {
                        CloseHandle(unk18);
                        unk18 = INVALID_HANDLE_VALUE;
                        if (unk1c) {
                            MemFree(unk1c, __FILE__, 0xaa);
                            unk1c = nullptr;
                        }
                        SetState((State)2);
                        return;
                    }
                }
            }
            SetState((State)4);
        } else if (unk8 == 0) {
            SetState((State)5);
        }
        break;
    }
    case 3: {
        if (unk8 != 0) {
            DWORD connectStatus = XNetGetConnectStatus(unk44);
            switch (connectStatus) {
            case 0:
            case 1:
                MILO_NOTIFY("XLSPConnection: Idle/establishing status while connected?");
                break;
            case 2:
                return;
            case 3:
                break;
            default:
                MILO_NOTIFY("XNetGetConnectStatus() unhandled return: %d", connectStatus);
                return;
            }
            SetState((State)4);
        } else {
            SetState((State)5);
        }
    } break;
    case 2: {
        if (unk8 != 0) {
            DWORD connectStatus = XNetGetConnectStatus(unk44);
            switch (connectStatus) {
            case 0:
                break;
            case 1:
                return;
            case 2:
                SetState((State)3);
                return;
            case 3:
                break;
            default:
                MILO_NOTIFY("XNetGetConnectStatus() unhandled return: %d", connectStatus);
                return;
            }
            SetState((State)4);
        } else {
            SetState((State)5);
        }
    } break;

    case 4:
        if (unk8 == 0)
            SetState((State)5);
        break;
    }
}

void XLSPConnection::SetState(State s) {
    while (unk4 != s) {
        switch (unk4) {
        case 1: {
            bool b = false;
            if (unk18 != INVALID_HANDLE_VALUE) {
                if (mXOverlapped.InternalLow == ERROR_IO_PENDING) {
                    if (s == (State)5) {
                        b = true;
                    } else {
                        XCancelOverlapped(&mXOverlapped);
                    }
                }
                if (!b) {
                    memset(&mXOverlapped, 0, sizeof(XOVERLAPPED));
                    CloseHandle(unk18);
                    unk18 = INVALID_HANDLE_VALUE;
                }
            }
            if (!b) {
                unk20 = 0;
                if (unk1c) {
                    MemFree(unk1c, __FILE__, 0x167);
                    unk1c = nullptr;
                }
            }
        } break;
        case 2:
            if (s != (State)3) {
                SecureDisconnect(unk44);
            }
            break;
        case 3:
            SecureDisconnect(unk44);
            break;
        }

        unk4 = s;
        switch (s) {
        case 1:
            StartEnumeration();
            return;
        case 2:
            if (StartGatewayConnection(unk44) == 0) {
                return;
            }
            s = (State)4;
            break;
        case 4:
            unk48.Restart();
            return;
        case 5: {
            if (unk18 != INVALID_HANDLE_VALUE) {
                ThreadCall(this);
                return;
            }
            s = (State)0;
        } break;
        default:
            return;
        }
    }
}
