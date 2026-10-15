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
    : mState(kS_Nil), mStateDesired(kS_Disconnected), mServiceId(0),
      mHEnum(INVALID_HANDLE_VALUE), mEnumBuf(0), mEnumBufSize(0) {
    memset(&mEnumXO, 0, sizeof(XOVERLAPPED));
    memset(&mSecureAddr, 0, sizeof(IN_ADDR));
    mThrottleTimer.Reset();
    SetState(kS_Disconnected);
}

XLSPConnection::~XLSPConnection() { SetState(kS_Nil); }

int XLSPConnection::ThreadStart() {
    if (mState != kS_RequestDisconnect) {
        MILO_FAIL("Unhandled state %d in ThreadStart", mState);
    } else {
        XCancelOverlapped(&mEnumXO);
    }
    return 0;
}

void XLSPConnection::ThreadDone(int i1) {
    if (mState != kS_RequestDisconnect) {
        MILO_FAIL("Unhandled state %d in ThreadStart", mState);
    } else {
        memset(&mEnumXO, 0, sizeof(XOVERLAPPED));
        CloseHandle(mHEnum);
        mEnumBufSize = 0;
        mHEnum = INVALID_HANDLE_VALUE;
        if (mEnumBuf) {
            MemFree(mEnumBuf, __FILE__, 299);
            mEnumBuf = nullptr;
        }
        SetState(kS_Disconnected);
    }
}

void XLSPConnection::Connect(const char *cc, unsigned int ui) {
    mStrServerFilter = cc;
    mServiceId = ui;
    if (mStateDesired != kS_Connected) {
        mStateDesired = kS_Connected;
    }
    if (mState == kS_Disconnected) {
        SetState(kS_Enumerating);
    }
}

void XLSPConnection::Disconnect() {
    if (mStateDesired != kS_Disconnected) {
        mStateDesired = kS_Disconnected;
    }
    if (mState > kS_Disconnected && mState <= kS_Failure) {
        SetState(kS_RequestDisconnect);
    }
}

void XLSPConnection::StartEnumeration() {
    DWORD res =
        XTitleServerCreateEnumerator(mStrServerFilter.c_str(), 8, &mEnumBufSize, &mHEnum);
    if (res != ERROR_SUCCESS) {
        MILO_NOTIFY("XTitleServerCreateEnumerator failed with error %d", res);
        SetState(kS_Failure);
    } else {
        mEnumBuf = (XTITLE_SERVER_INFO *)_MemAllocTemp(
            mEnumBufSize, __FILE__, 0x1CB, "XLSPConnection", 0
        );
        res = XEnumerate(mHEnum, mEnumBuf, mEnumBufSize, nullptr, &mEnumXO);
        if (res != ERROR_IO_PENDING) {
            MILO_NOTIFY("XEnumerate failed with error %d", res);
            SetState(kS_Failure);
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
    switch (mState) {
    case kS_Disconnected:
        if (mStateDesired == kS_Connected)
            SetState(kS_Enumerating);
        break;
    case kS_Enumerating: {
        if (mHEnum != INVALID_HANDLE_VALUE && mEnumXO.InternalLow != ERROR_IO_PENDING) {
            DWORD word = 0;
            DWORD overlapResult = XGetOverlappedResult(&mEnumXO, &word, false);
            if (overlapResult != 0) {
                XGetOverlappedExtendedError(&mEnumXO);
            } else {
                memset(&mEnumXO, 0, sizeof(XOVERLAPPED));
                if (word != 0) {
                    DWORD random = RandomInt(0, word);
                    if ((unsigned int)XNetServerToInAddr(
                            mEnumBuf[random].inaServer, mServiceId, &mSecureAddr
                        )
                        == 0) {
                        CloseHandle(mHEnum);
                        mHEnum = INVALID_HANDLE_VALUE;
                        if (mEnumBuf) {
                            MemFree(mEnumBuf, __FILE__, 0xaa);
                            mEnumBuf = nullptr;
                        }
                        SetState(kS_ConnectingToGateway);
                        return;
                    }
                }
            }
            SetState(kS_Failure);
        } else if (mStateDesired == kS_Disconnected) {
            SetState(kS_RequestDisconnect);
        }
        break;
    }
    case kS_Connected: {
        if (mStateDesired != kS_Disconnected) {
            DWORD connectStatus = XNetGetConnectStatus(mSecureAddr);
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
            SetState(kS_Failure);
        } else {
            SetState(kS_RequestDisconnect);
        }
    } break;
    case kS_ConnectingToGateway: {
        if (mStateDesired != 0) {
            DWORD connectStatus = XNetGetConnectStatus(mSecureAddr);
            switch (connectStatus) {
            case 0:
                break;
            case 1:
                return;
            case 2:
                SetState(kS_Connected);
                return;
            case 3:
                break;
            default:
                MILO_NOTIFY("XNetGetConnectStatus() unhandled return: %d", connectStatus);
                return;
            }
            SetState(kS_Failure);
        } else {
            SetState(kS_RequestDisconnect);
        }
    } break;

    case kS_Failure:
        if (mStateDesired == kS_Disconnected)
            SetState(kS_RequestDisconnect);
        break;
    }
}

void XLSPConnection::SetState(State s) {
    while (mState != s) {
        switch (mState) {
        case 1: {
            bool b = false;
            if (mHEnum != INVALID_HANDLE_VALUE) {
                if (mEnumXO.InternalLow == ERROR_IO_PENDING) {
                    if (s == kS_RequestDisconnect) {
                        b = true;
                    } else {
                        XCancelOverlapped(&mEnumXO);
                    }
                }
                if (!b) {
                    memset(&mEnumXO, 0, sizeof(XOVERLAPPED));
                    CloseHandle(mHEnum);
                    mHEnum = INVALID_HANDLE_VALUE;
                }
            }
            if (!b) {
                mEnumBufSize = 0;
                if (mEnumBuf) {
                    MemFree(mEnumBuf, __FILE__, 0x167);
                    mEnumBuf = nullptr;
                }
            }
        } break;
        case 2:
            if (s != kS_Connected) {
                SecureDisconnect(mSecureAddr);
            }
            break;
        case 3:
            SecureDisconnect(mSecureAddr);
            break;
        }

        mState = s;
        switch (s) {
        case 1:
            StartEnumeration();
            return;
        case 2:
            if (StartGatewayConnection(mSecureAddr) == 0) {
                return;
            }
            s = kS_Failure;
            break;
        case 4:
            mThrottleTimer.Restart();
            return;
        case 5: {
            if (mHEnum != INVALID_HANDLE_VALUE) {
                ThreadCall(this);
                return;
            }
            s = kS_Disconnected;
        } break;
        default:
            return;
        }
    }
}
