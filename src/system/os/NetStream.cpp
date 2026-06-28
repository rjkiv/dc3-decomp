#include "os/NetStream.h"
#include "NetStream.h"
#include "NetworkSocket.h"
#include "os/Timer.h"
#include "utl/BinStream.h"

NetStream::NetStream()
    : BinStream(true), mSocket(nullptr), mFail(false), mReadTimeoutMs(0), mBytesRead(0),
      mBytesWritten(0) {
    mSocket = NetworkSocket::Create(true);
}

NetStream::~NetStream() {
    if (mSocket) {
        mSocket->Disconnect();
        delete mSocket;
    }
}

EofType NetStream::Eof() { return (EofType)!mSocket->CanRead(); }

int NetStream::ReadAsync(void *v, int i) { return mSocket->Recv(v, i); }

void NetStream::ReadImpl(void *v, int i) {
    int orig_i = i;
    void *orig_v = v;
    Timer t;
    t.Start();
    while (i > 0) {
        int bytes = mSocket->Recv(v, i);
        if (mSocket->Fail() || (mReadTimeoutMs && t.SplitMs() > mReadTimeoutMs)) {
            mFail = true;
            break;
        }

        v = (void *)((uint)v + bytes);
        i -= bytes;
    }
    if (!mFail) {
        mBytesRead += orig_i;
    }
    if (mFail)
        memset(orig_v, 0xEA, orig_i);
}

void NetStream::WriteImpl(const void *v, int i) {
    int orig_i = i;
    Timer t;
    t.Start();
    while (i > 0) {
        int bytes = mSocket->Send(v, i);
        if (mSocket->Fail() || (mReadTimeoutMs && t.SplitMs() > mReadTimeoutMs)) {
            mFail = true;
            break;
        }

        v = (void *)((uint)v + bytes);
        i -= bytes;
    }
    if (!mFail) {
        mBytesWritten += orig_i;
    }
}

void NetStream::SeekImpl(int i, SeekType ty) { MILO_ASSERT(false, 0x7A); }

void NetStream::ClientConnect(const NetAddress &addr) {
    MILO_ASSERT(mSocket, 0x38);
    Timer timer;
    timer.Restart();
    mSocket->Connect(addr.GetIP(), addr.GetPort());
    if (mSocket->Fail()) {
        mFail = true;
    } else {
        while (!mSocket->CanSend()) {
            if (mSocket->Fail() || timer.SplitMs() > 1000) {
                mFail = true;
                break;
            }
        }
    }
    if (mSocket->Fail()) {
        mFail = true;
    }
}
