#pragma once
#include "net/HttpGet.h"
#include "net/XLSPConnection.h"
#include "utl/HxGuid.h"
#include "xdk/xapilibi/xbase.h"

#pragma pack(push, 1)
struct KinectShareHeaderDataStruct {
    unsigned char Flags; // 0x0
    unsigned short MajorVersion; // 0x1
    unsigned short MinorVersion; // 0x3
    u64 SessionStartTime; // 0x5
    u64 UserID0; // 0xd
    u64 UserID1; // 0x15
    u64 UserID2; // 0x1d
    u64 UserID3; // 0x25
    unsigned char CompressionType; // 0x2d
    unsigned int DataSize; // 0x2e
    unsigned char MessageType; // 0x32
    unsigned int CompressedPartitionSizeInBytes; // 0x33
    unsigned int CompressedWindowSizeInBytes; // 0x37
    unsigned short MagicNumber; // 0x3b
    unsigned char Type; // 0x3d
    unsigned short ContentMagicNumber; // 0x3e
    unsigned char ContentType; // 0x40
    int ID[4]; // 0x41
    unsigned int DataChunkSizeInBytes; // 0x51
    unsigned short DataChunkIndex; // 0x55
    unsigned short ChunkCount; // 0x57
    int BatchID[4]; // 0x59
    unsigned int TimeStamp; // 0x69
    unsigned char Reserved; // 0x6d
    unsigned char Reserved2; // 0x6e
    unsigned int DataTotalSizeInBytes; // 0x6f
    u64 CreationTimeStampUtc; // 0x73
    int TitleServerEnumerationID[4]; // 0x7b
    unsigned int Score; // 0x8b
    unsigned int LevelInfo; // 0x8f
    unsigned short Language; // 0x93
    unsigned short Locale; // 0x95
};
#pragma pack(pop)

// size 0x140
class KinectShare : public HttpPost {
public:
    enum EContentType {
        kPhotoContentType = 0x0000,
        kVideoContentType = 0x0001
    };
    KinectShare(unsigned int, int, const char *, int, EContentType, u64, u64, u64, u64);

    void Cancel();

protected:
    virtual bool CanRetry();
    virtual void Sending();

private:
    KinectShareHeaderDataStruct mHeaderData; // 0xa0
    int mHeaderSize; // 0x138
    int mHeaderBytesLeftToSend; // 0x13c
};
#pragma pack(pop)

class KinectShareConnection {
public:
    ~KinectShareConnection();
    void Poll();

    int GetState() const { return mState; }

private:
    XLSPConnection mXLSPConnection; // 0x0
    enum {
        kConnecting = 0x0000,
        kUploading = 0x0001,
        kSuccess = 0x0002,
        kFailed = 0x0003
    } mState; // 0x78
    KinectShare *mKinectShare; // 0x7c
    const char *mPhotoData; // 0x80
    int mPhotoDataLen; // 0x84
    KinectShare::EContentType mContentType; // 0x88
    XUID mXUIDs[4]; // 0x90
};
