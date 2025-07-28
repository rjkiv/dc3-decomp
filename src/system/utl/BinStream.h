#pragma once

#include <types.h>
#include "math/Rand2.h"
#include "os/Platform.h"
#include "utl/Symbol.h"
#include "utl/Str.h"

enum EofType {
    NotEof = 0,
    RealEof = 1,
    TempEof = 2
};

class BinStream {
public:
    /**
     * @brief The three seek types for BinStream::Seek.
     * @see Seek()
     */
    enum SeekType {
        kSeekBegin = 0,
        kSeekCur = 1,
        kSeekEnd = 2,
    };

    BinStream(bool littleEndian);
    virtual ~BinStream();
    /** Completely flush out this BinStream. */
    virtual void Flush() = 0;
    /** Get the current position of the BinStream. */
    virtual int Tell() = 0;
    /** Get the current End Of File type of the BinStream. */
    virtual EofType Eof() = 0;
    /** Get whether or not the BinStream has failed. */
    virtual bool Fail() = 0;
    /** Get this BinStream's name. */
    virtual const char *Name() const;
    /** Get whether or not this BinStream is cached. Interestingly only overwritten in
     * ChunkStream. */
    virtual bool Cached() const { return false; }
    /** Get this BinStream's current platform (Xbox, PS3, etc). Interestingly only
     * overwritten in ChunkStream. */
    virtual Platform GetPlatform() const { return kPlatformNone; }
    /** The specific implementation for reading from byte data.
     *  @param [in] data The data to read from.
     *  @param [in] bytes The number of bytes to read.
     */
    virtual void ReadImpl(void *data, int bytes) = 0;
    /** The specific implementation for writing to byte data.
     *  @param [in] data The data to write to.
     *  @param [in] bytes The number of bytes to read.
     */
    virtual void WriteImpl(const void *data, int bytes) = 0;
    /** The specific implementation for seeking within this BinStream.
     *  @param [in] offset The offset in the data to seek to.
     *  @param [in] type The Seek type.
     */
    virtual void SeekImpl(int offset, SeekType type) = 0;

    void Read(void *, int);
    void ReadEndian(void *, int);

    void Write(const void *, int);
    void WriteEndian(const void *, int);

    void EnableReadEncryption(void);
    void ReadString(char *, int);

    bool AddSharedInlined(const class FilePath &);

    BinStream &operator<<(const char *);
    BinStream &operator<<(const Symbol &);
    BinStream &operator<<(const String &);

    BinStream &operator>>(Symbol &);
    BinStream &operator>>(String &);

#define BS_READ_OP(typename)                                                             \
    BinStream &operator>>(typename &rhs) {                                               \
        ReadEndian(&rhs, sizeof(typename));                                              \
        return *this;                                                                    \
    }
#define BS_WRITE_OP(typename)                                                            \
    BinStream &operator<<(typename rhs) {                                                \
        WriteEndian(&rhs, sizeof(typename));                                             \
        return *this;                                                                    \
    }
    BS_READ_OP(int)
    BS_READ_OP(uint)
    BS_READ_OP(u16)
    BS_READ_OP(u32)
    BS_READ_OP(u64)
    BS_READ_OP(f32)
    BS_READ_OP(f64)

    BS_WRITE_OP(int)
    BS_WRITE_OP(uint)
    BS_WRITE_OP(u16)
    BS_WRITE_OP(u32)
    BS_WRITE_OP(u64)
    BS_WRITE_OP(f32)
    BS_WRITE_OP(f64)

#undef BS_READ_OP
#undef BS_WRITE_OP

private:
    bool mLittleEndian;
    Rand2 *mCrypto;
};
