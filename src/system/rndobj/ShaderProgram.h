#pragma once
#include "obj/Object.h"
#include "rndobj/ShaderOptions.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

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
    virtual void Select(bool) = 0;
    virtual void Copy(const RndShaderProgram &) = 0;
    virtual void EstimatedCost(float &, float &) = 0;
    virtual RndShaderBuffer *NewBuffer(unsigned int) = 0;
    virtual bool
    Compile(ShaderType, const ShaderOptions &, RndShaderBuffer *&, RndShaderBuffer *&) = 0;
    virtual void CreateVertexShader(RndShaderBuffer &) = 0;
    virtual void CreatePixelShader(RndShaderBuffer &, ShaderType) = 0;

    void LoadShaderBuffer(BinStream &, int, RndShaderBuffer *&);
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
    void SaveShaderBuffer(const char *, RndShaderBuffer &);
    void LoadShaderBuffer(const char *, RndShaderBuffer *&);
};
