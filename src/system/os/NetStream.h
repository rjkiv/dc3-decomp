#pragma once
#include "os/NetworkSocket.h"
#include "utl/BinStream.h"

/**
 * @brief BinStream implementation that handles networking.
 * Useful for loading rawfiles from a network or similar.
 */
class NetStream : public BinStream {
public:
    NetStream();
    virtual ~NetStream();
    virtual int Tell() { return 0; }
    virtual void Flush() {}
    virtual EofType Eof();
    virtual bool Fail() { return mFail; }
    virtual int ReadAsync(void *, int);

    void ClientConnect(const NetAddress &);
    NetworkSocket *Socket() const { return mSocket; }

    static NetworkSocket *Create(bool);
    static unsigned int IPStringToInt(const String &);
    static String IPIntToString(unsigned int);
    static String GetHostName();
    static unsigned int ResolveHostName(String);

private:
    virtual void ReadImpl(void *, int);
    virtual void WriteImpl(const void *, int);
    virtual void SeekImpl(int, SeekType);

    NetworkSocket *mSocket; // 0x10
    bool mFail; // 0x14
    float mReadTimeoutMs; // 0x18
    int mBytesRead; // 0x1c
    int mBytesWritten; // 0x20
};
