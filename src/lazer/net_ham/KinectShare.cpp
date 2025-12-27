#include "net_ham/KinectShare.h"
#include "net/HttpGet.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/HxGuid.h"
#include "utl/MemMgr.h"
#include "xdk/xapilibi/sysinfoapi.h"

KinectShare::KinectShare(
    unsigned int ip,
    int port,
    const char *cc,
    int len,
    EContentType etype,
    u64 i5,
    u64 i6,
    u64 i7,
    u64 i8
)
    : HttpPost(ip, port, "/lspfrontdoorprocessor/default.aspx", 0) {
    SetTimeout(10000);
    mContentLength = len;
    unk138 = 0x97;
    unk13c = 0x97;
    unk90 = len;
    unka0 = 0x80;
    unka1 = 0;
    unka3 = 7;
    FILETIME lpSystemTimeAsFileTime;
    GetSystemTimeAsFileTime(&lpSystemTimeAsFileTime);
    unkdb = 0xF00D;
    unkde = 0xF00D;
    unkcd = false;
    unkad = i5;
    unkb5 = i6;
    unkbd = i7;
    unkc5 = i8;
    unkd2 = true;
    unkd3 = 0;
    unkd7 = 0;
    unkdd = 3;
    unke0 = etype;
    unkce = mContentLength + unk138;
    unka5 = (lpSystemTimeAsFileTime.dwLowDateTime & 0xFFFF) * 0x10000
            + lpSystemTimeAsFileTime.dwHighDateTime
        & 0xFFFFFFFF;
    HxGuid hx60;
    hx60.Generate();
    memcpy(unke1, hx60.Data(), sizeof(HxGuid));
    unkf5 = 0;
    unkf7 = 1;
    unkf1 = mContentLength + unk138;
    hx60.Generate();
    memcpy(unkf9, hx60.Data(), sizeof(HxGuid));
    unk109 = 0;
    unk10d = false;
    unk10e = false;
    unk10f = mContentLength;
    unk113 = unka5;
    hx60.Generate();
    memcpy(unk11b, hx60.Data(), sizeof(HxGuid));
    unk12b = 0;
    unk12f = 0;
    unk133 = ULSystemLanguage();
    unk135 = ULSystemLocale();
}

bool KinectShare::CanRetry() {
    if ((unk13c || unk90) && HttpPost::CanRetry()) {
        unk13c = unk138;
        return true;
    } else {
        return false;
    }
}

void KinectShare::Sending() {
    MILO_ASSERT(mSocket, 0x87);
    if (unk13c > 0) {
        int ret = mSocket->Send(&unka0, (unk138 - unk13c) + unka0);
        if (ret == -1) {
            mFailType = (HttpGetFailType)1;
            SetState((State)7);
        } else if (ret != unk13c) {
            unk13c -= ret;
        } else {
            unk13c = 0;
        }
    } else {
        HttpPost::Sending();
    }
}

KinectShareConnection::~KinectShareConnection() {
    RELEASE(mKinectShare);
    mConnection.Disconnect();
}

void KinectShareConnection::Poll() {
    switch (unk78) {
    case 0: {
        MILO_ASSERT(!mKinectShare, 0xC6);
        mConnection.Poll();
        int connectionState = mConnection.GetState();
        if (connectionState == 4) {
            MILO_LOG(
                "KinectShareConnection::Poll: XLSP connection failed while connecting\n"
            );
            mConnection.Disconnect();
            unk78 = 3;
        } else if (connectionState == 3) {
            mKinectShare = new KinectShare(
                mConnection.GetServiceIP(),
                1000,
                unk80,
                unk84,
                unk88,
                unk90,
                unk98,
                unka0,
                unka8
            );
            mKinectShare->Send();
            unk78 = 1;
        } else {
            return;
        }
        break;
    }
    case 1: {
        MILO_ASSERT(mKinectShare, 0xDA);
        mConnection.Poll();
        mKinectShare->Poll();
        if (mKinectShare->HasFailed()) {
            MILO_LOG(
                "KinectShare::Poll: Upload failed, fail type: %d, prev state: %d\n",
                mKinectShare->FailType(),
                mKinectShare->PrevState()
            );
            mConnection.Disconnect();
            RELEASE(mKinectShare);
            unk78 = 3;
        } else if (mKinectShare->IsDownloaded()) {
            if (mKinectShare->GetBufferSize() == 5) {
                char *response = mKinectShare->DetachBuffer();
                MILO_ASSERT(response, 0xEA);
                if (response[4] == 1) {
                    unk78 = 2;
                } else {
                    MILO_LOG(
                        "KinectShare::Poll: Upload failed, response code = %d\n",
                        response[4]
                    );
                    unk78 = 3;
                }
                MemFree(response, __FILE__, 0xF4);
            } else {
                MILO_LOG("KinectShare::Poll: Upload failed, invalid response data\n");
                unk78 = 3;
            }
            mConnection.Disconnect();
            RELEASE(mKinectShare);
        }
        break;
    }
    default:
        break;
    }
}
