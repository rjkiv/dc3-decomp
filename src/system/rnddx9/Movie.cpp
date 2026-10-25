#include "rnddx9/Movie.h"
#include "Tex.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/Timer.h"
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

DxMovie::DxMovie() : mBuffer(0), mStreamFile(0) {}

DxMovie::~DxMovie() {
    RELEASE(mStreamFile);
    if (mBuffer) {
        MemFree(mBuffer, __FILE__, 0x1F);
        mBuffer = nullptr;
    }
}

void DxMovie::StreamReadFinish() {
    // pdb has a `bytes` local but idk what it'd. be
    while (!mStreamFile->ReadDone())
        ;
}

void DxMovie::SetFile(const FilePath &file, bool stream) {
    RELEASE(mStreamFile);
    if (mBuffer) {
        MemFree(mBuffer, __FILE__, 0x2B);
        mBuffer = nullptr;
    }
    mVideo.Reset();
    RndMovie::SetFile(file, stream);
    if (!mFile.empty()) {
        if (stream) {
            mStreamFile = NewFile(CacheResource(mFile.c_str(), this), 2);
            if (!mStreamFile) {
                MILO_NOTIFY("%s: %s not found", PathName(this), mFile);
                return;
            }
            FileStream fStream(mStreamFile, true);
            mVideo.Load(fStream, true);
            void *frames =
                MemAlloc(mVideo.FrameSize() * 2, __FILE__, 0x44, "RndMovie frames");
            mStreamOffset = 0;
            mBuffer = static_cast<char *>(frames);
            mStreamAddr = static_cast<char *>(frames);
            mStreamOrigin = fStream.Tell();
        } else {
            FileLoader *fl = dynamic_cast<FileLoader *>(TheLoadMgr.ForceGetLoader(file));
            int size;
            char *buffer = fl ? fl->GetBuffer(&size) : nullptr;
            delete fl;
            if (!buffer)
                return;
            BufStream bStream(buffer, size, true);
            mVideo.Load(bStream, false);
            MemFree(buffer, __FILE__, 0x58);
        }
        mMovieFrame = mVideo.NumFrames();
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
        int srcPitch = (mVideo.Bpp() * mVideo.Width() * 4) >> 3;
        surface->LockRect(&lock, nullptr, 0);
        if (srcPitch == lock.Pitch) {
            memcpy(lock.pBits, mStreamAddr, mVideo.FrameSize());
        } else {
            MILO_ASSERT(srcPitch < lock.Pitch, 0xB5);
            char *curBits = (char *)lock.pBits;
            char *addr_it = mStreamAddr;
            for (int i = mVideo.FrameSize(); i > 0; i -= srcPitch) {
                memcpy(curBits, addr_it, srcPitch);
                addr_it += srcPitch;
                curBits += lock.Pitch;
            }
        }
        surface->UnlockRect();
        surface->Release();
    }
}

int DxMovie::StreamChunkSize() {
    return Min(mVideo.FrameSize(), mStreamFile->Size() - mStreamFile->Tell());
}

void DxMovie::StreamNextBuffer() {
    StreamReadFinish();
    mStreamOffset = mStreamOffset ? 0 : mVideo.FrameSize();
    mStreamFile->ReadAsync(mBuffer + mStreamOffset, StreamChunkSize());
}

void DxMovie::StreamRestart(int f) {
    StreamReadFinish();
    mStreamFile->Seek(mStreamOrigin + f * mVideo.FrameSize(), FILE_SEEK_SET);
    mStreamFile->Read(mBuffer, StreamChunkSize());
    mStreamOffset = 0;
    mStreamAddr = mBuffer;
    StreamNextBuffer();
}

void DxMovie::SetFrame(float frame, float blend) {
    START_AUTO_TIMER("movie");
    RndAnimatable::SetFrame(frame, blend);
    if ((mStream == false || mBuffer != nullptr) && mTex != nullptr
        && mVideo.NumFrames() != 0) {
        int i4 = static_cast<int>(frame) % mVideo.NumFrames(); // in theory it's here
        int frame_i = i4 - mMovieFrame;
        if (i4 - mMovieFrame == 0) {
            return;
        } else {
            mMovieFrame = i4;
            if (mStream) {
                if (frame_i >= 0 && frame_i <= 1) {
                    StreamReadFinish();
                    mStreamAddr = mBuffer + mStreamOffset;
                    StreamNextBuffer();
                } else {
                    StreamRestart(i4);
                }
            } else {
                mStreamAddr = mVideo.Frame(i4);
            }
            Update();
            return;
        }
    }
}
