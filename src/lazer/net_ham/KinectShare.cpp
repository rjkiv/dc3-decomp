#include "net_ham/KinectShare.h"
#include "net/HttpGet.h"
#include "os/Debug.h"
#include "os/System.h"
#include "utl/HxGuid.h"
#include "utl/MemMgr.h"
#include "xdk/xapilibi/sysinfoapi.h"
#include "xdk/xapilibi/xbox.h"
#include <cstring>

KinectShare::KinectShare(
    unsigned int ip,
    int svrPort,
    const char *data,
    int len,
    EContentType contentType,
    u64 xuid0,
    u64 xuid1,
    u64 xuid2,
    u64 xuid3
)
    : HttpPost(ip, svrPort, "/lspfrontdoorprocessor/default.aspx", 0) {
    SetTimeout(10000.0f);
    mContentLength = len;
    mHeaderSize = 0x97;
    mHeaderBytesLeftToSend = 0x97;
    unk90 = len;
    mHeaderData.Flags = 0x80;
    mHeaderData.MajorVersion = 0;
    mHeaderData.MinorVersion = 7;

    FILETIME fileTime;
    GetSystemTimeAsFileTime(&fileTime);
    mHeaderData.SessionStartTime =
        (fileTime.dwHighDateTime) * 0x10000 + fileTime.dwLowDateTime;
    mHeaderData.MagicNumber = 0xF00D;
    mHeaderData.ContentMagicNumber = 0xF00D;
    mHeaderData.CompressionType = 0;
    mHeaderData.UserID0 = xuid0;
    mHeaderData.UserID1 = xuid1;
    mHeaderData.UserID2 = xuid2;
    mHeaderData.UserID3 = xuid3;
    mHeaderData.MessageType = 1;
    mHeaderData.CompressedPartitionSizeInBytes = 0;
    mHeaderData.CompressedWindowSizeInBytes = 0;
    mHeaderData.Type = 3;
    mHeaderData.ContentType = contentType;
    mHeaderData.DataSize = mContentLength + mHeaderSize;

    HxGuid tmpGuid;
    tmpGuid.Generate();
    memcpy(mHeaderData.ID, tmpGuid.Data(), sizeof(HxGuid));

    mHeaderData.DataChunkIndex = 0;
    mHeaderData.ChunkCount = 1;
    mHeaderData.DataChunkSizeInBytes = mContentLength + mHeaderSize;

    tmpGuid.Generate();
    memcpy(mHeaderData.BatchID, tmpGuid.Data(), sizeof(HxGuid));
    mHeaderData.TimeStamp = 0;
    mHeaderData.Reserved = 0;
    mHeaderData.Reserved2 = 0;
    mHeaderData.DataTotalSizeInBytes = mContentLength;
    mHeaderData.CreationTimeStampUtc = mHeaderData.SessionStartTime;

    tmpGuid.Generate();
    memcpy(mHeaderData.TitleServerEnumerationID, tmpGuid.Data(), sizeof(HxGuid));
    mHeaderData.Score = 0;
    mHeaderData.LevelInfo = 0;
    mHeaderData.Language = ULSystemLanguage();
    mHeaderData.Locale = ULSystemLocale();
}

bool KinectShare::CanRetry() {
    if ((mHeaderBytesLeftToSend || unk90) && HttpPost::CanRetry()) {
        mHeaderBytesLeftToSend = mHeaderSize;
        return true;
    } else {
        return false;
    }
}

void KinectShare::Sending() {
    MILO_ASSERT(mSocket, 0x87);
    if (mHeaderBytesLeftToSend > 0) {
        int ret = mSocket->Send(
            &mHeaderData.Flags + (mHeaderSize - mHeaderBytesLeftToSend),
            mHeaderBytesLeftToSend
        );
        if (ret == -1) {
            mFailType = (HttpGetFailType)1;
            SetState((State)7);
        } else if (ret != mHeaderBytesLeftToSend) {
            mHeaderBytesLeftToSend -= ret;
        } else {
            mHeaderBytesLeftToSend = 0;
        }
    } else {
        HttpPost::Sending();
    }
}

KinectShareConnection::~KinectShareConnection() {
    RELEASE(mKinectShare);
    mXLSPConnection.Disconnect();
}

void KinectShareConnection::Poll() {
    switch (mState) {
    case kConnecting: {
        MILO_ASSERT(!mKinectShare, 0xC6);
        mXLSPConnection.Poll();
        int connectionState = mXLSPConnection.GetState();
        if (connectionState == 4) {
            MILO_LOG(
                "KinectShareConnection::Poll: XLSP connection failed while connecting\n"
            );
            mXLSPConnection.Disconnect();
            mState = kFailed;
        } else if (connectionState == 3) {
            mKinectShare = new KinectShare(
                mXLSPConnection.GetServiceIP(),
                1000,
                mPhotoData,
                mPhotoDataLen,
                mContentType,
                mXUIDs[0],
                mXUIDs[1],
                mXUIDs[2],
                mXUIDs[3]
            );
            mKinectShare->Send();
            mState = kUploading;
        } else {
            return;
        }
        break;
    }
    case kUploading: {
        MILO_ASSERT(mKinectShare, 0xDA);
        mXLSPConnection.Poll();
        mKinectShare->Poll();
        if (mKinectShare->HasFailed()) {
            MILO_LOG(
                "KinectShare::Poll: Upload failed, fail type: %d, prev state: %d\n",
                mKinectShare->FailType(),
                mKinectShare->PrevState()
            );
            mXLSPConnection.Disconnect();
            RELEASE(mKinectShare);
            mState = kFailed;
        } else if (mKinectShare->IsDownloaded()) {
            if (mKinectShare->GetBufferSize() == 5) {
                char *response = mKinectShare->DetachBuffer();
                MILO_ASSERT(response, 0xEA);
                if (response[4] == 1) {
                    mState = kSuccess;
                } else {
                    MILO_LOG(
                        "KinectShare::Poll: Upload failed, response code = %d\n",
                        response[4]
                    );
                    mState = kFailed;
                }
                MemFree(response, __FILE__, 0xF4);
            } else {
                MILO_LOG("KinectShare::Poll: Upload failed, invalid response data\n");
                mState = kFailed;
            }
            mXLSPConnection.Disconnect();
            RELEASE(mKinectShare);
        }
        break;
    }
    default:
        break;
    }
}
