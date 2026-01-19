#include "rnddx9/Movie.h"
#include "Tex.h"
#include "os/Debug.h"
#include "os/File.h"
#include "rndobj/Movie.h"
#include "rndobj/Tex.h"
#include "utl/BufStream.h"
#include "utl/FilePath.h"
#include "utl/FileStream.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "rndobj/Utl.h"
#include "xdk/d3d9i/d3d9.h"
#include "xdk/d3d9i/d3d9types.h"

DxMovie::DxMovie() : unk44(0), unk4c(0) {}

DxMovie::~DxMovie() {
    RELEASE(unk4c);
    if (unk44) {
        MemFree(unk44, __FILE__, 0x1F);
        unk44 = nullptr;
    }
}

void DxMovie::StreamReadFinish() {
    while (!unk4c->ReadDone())
        ;
}

void DxMovie::SetFile(const FilePath &file, bool stream) {
    RELEASE(unk4c);
    char *buffer = nullptr;
    if (unk44) {
        MemFree(unk44, __FILE__, 0x2B);
        unk44 = nullptr;
    }
    mVideo.Reset();
    RndMovie::SetFile(file, stream);
    if (!mFile.empty()) {
        if (stream) {
            unk4c = NewFile(CacheResource(mFile.c_str(), this), 2);
            if (!unk4c) {
                MILO_NOTIFY("%s: %s not found", PathName(this), mFile);
                return;
            }
            FileStream fStream(unk4c, true);
            mVideo.Load(fStream, true);
            void *frames =
                MemAlloc(mVideo.FrameSize() * 2, __FILE__, 0x44, "RndMovie frames");
            unk54 = 0;
            unk44 = frames;
            unk58 = frames;
            unk50 = fStream.Tell();
        } else {
            FileLoader *fl = dynamic_cast<FileLoader *>(TheLoadMgr.ForceGetLoader(file));
            int size;
            if (fl) {
                buffer = fl->GetBuffer(&size);
                delete fl;
            }
            if (!buffer)
                return;
            BufStream bStream(buffer, size, true);
            mVideo.Load(bStream, false);
            MemFree(buffer, __FILE__, 0x58);
        }
        unk48 = mVideo.NumFrames();
        SetTex(mTex);
        SetFrame(0, 1);
    }
}

void DxMovie::SetTex(RndTex *tex) {
    RndMovie::SetTex(tex);
    if (mTex) {
        if (mVideo.Width() && mVideo.Height() && mVideo.NumFrames()) {
            mTex->SetBitmap(
                mVideo.Width(),
                mVideo.Height(),
                mVideo.Bpp(),
                RndTex::kMovie,
                false,
                nullptr
            );
        } else {
            mTex->SetBitmap(0x10, 0x10, 0x20, RndTex::kRegular, false, nullptr);
        }
    }
}

void DxMovie::Update() {
    DxTex *tex = static_cast<DxTex *>(mTex.Ptr());
    tex->SwapMovieSurface();
    D3DSurface *surface = tex->GetMovieSurface();
    if (surface) {
        D3DLOCKED_RECT lock;
        int srcPitch = mVideo.Bpp() * mVideo.Width() * 4;
        D3DSurface_LockRect(surface, &lock, nullptr, 0);
        if (srcPitch == lock.Pitch) {
            memcpy(lock.pBits, unk58, mVideo.FrameSize());
        } else {
            MILO_ASSERT(srcPitch < lock.Pitch, 0xB5);
            char *curBits = (char *)lock.pBits;
            char *c58 = (char *)unk58;
            for (int i = mVideo.FrameSize(); i > 0; i -= srcPitch) {
                memcpy(curBits, c58, srcPitch);
                c58 += srcPitch;
                curBits += lock.Pitch;
            }
        }
        D3DSurface_UnlockRect(surface);
        D3DResource_Release(surface);
    }
}

int DxMovie::StreamChunkSize() {
    return Min(mVideo.FrameSize(), unk4c->Size() - unk4c->Tell());
}

void DxMovie::StreamNextBuffer() {
    StreamReadFinish();
    unk54 = unk54 ? 0 : mVideo.FrameSize();
    char *c44 = (char *)unk44;
    int size = StreamChunkSize();
    unk4c->ReadAsync(c44 + unk54, size);
}
