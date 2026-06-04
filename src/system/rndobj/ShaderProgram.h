#pragma once
#include "obj/Object.h"
#include "rndobj/ShaderOptions.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

// A straight up data buffer, meant to store shader info.
class RndShaderBuffer {
public:
    virtual ~RndShaderBuffer() {}
    virtual void *Storage() = 0;
    virtual unsigned int Size() const = 0;

    MEM_OVERLOAD(RndShaderBuffer, 0x11);
};

// An individual node in a RndShaderMgr::ShaderTree.
class RndShaderProgram {
public:
    RndShaderProgram() : mFlags(-1), mLeft(0), mRight(0), mCached(0) {}
    virtual ~RndShaderProgram() {
        delete mLeft;
        delete mRight;
    }
    virtual void Select(bool vertexOnly) = 0;
    virtual void Copy(const RndShaderProgram &src) = 0;
    virtual void EstimatedCost(float &min, float &max) = 0;
    virtual RndShaderBuffer *NewBuffer(unsigned int numBufferBytes) = 0;
    virtual bool
    Compile(ShaderType, const ShaderOptions &, RndShaderBuffer *&, RndShaderBuffer *&) = 0;
    virtual void CreateVertexShader(RndShaderBuffer &vertexBuffer) = 0;
    virtual void CreatePixelShader(RndShaderBuffer &pixelBuffer, ShaderType) = 0;

    /** Load shader info from a stream into a buffer.
     * @param [in] bs The binary stream.
     * @param [in] size The size in bytes of the data to read in.
     * @param [out] buffer The shader buffer.
     */
    void LoadShaderBuffer(BinStream &bs, int size, RndShaderBuffer *&buffer);
    bool Cache(
        ShaderType type,
        const ShaderOptions &options,
        RndShaderBuffer *bufVertex,
        RndShaderBuffer *bufPixel
    );
    bool Cached() const { return mCached; }

    static unsigned long InitModTime();

    u64 mFlags; // 0x8
    RndShaderProgram *mLeft; // 0x10
    RndShaderProgram *mRight; // 0x14
    bool mCached; // 0x18

protected:
    void CopyErrorShader(ShaderType, const ShaderOptions &);
    /** Save the shader buffer data to a file. */
    void SaveShaderBuffer(const char *file, RndShaderBuffer &buffer);
    /** Load shader info from a file into a buffer. */
    void LoadShaderBuffer(const char *file, RndShaderBuffer *&buffer);
};
