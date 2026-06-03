#include "rndobj/ShaderProgram.h"
#include "ShaderMgr.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/File.h"
#include "os/Memory.h"
#include "os/OSFuncs.h"
#include "os/System.h"
#include "os/Timer.h"
#include "rndobj/Env.h"
#include "rndobj/Mat_NG.h"
#include "rndobj/ShaderOptions.h"
#include "utl/BinStream.h"
#include "utl/DataPointMgr.h"
#include "utl/FileStream.h"
#include "utl/Loader.h"
#include "utl/MemMgr.h"
#include "math/Utl.h"

void RndShaderProgram::SaveShaderBuffer(const char *file, RndShaderBuffer &buffer) {
    FileMkDir(FileGetPath(file));
    File *f = NewFile(file, 0x301);
    f->Write(buffer.Storage(), buffer.Size());
    delete f;
}

void RndShaderProgram::LoadShaderBuffer(
    BinStream &bs, int size, RndShaderBuffer *&buffer
) {
    MemDoTempAllocations tmp;
    buffer = NewBuffer(size);
    bs.Read(buffer->Storage(), size);
}

void RndShaderProgram::LoadShaderBuffer(const char *cc, RndShaderBuffer *&buffer) {
    FileStream stream(cc, FileStream::kReadNoArk, true);
    LoadShaderBuffer(stream, stream.Size(), buffer);
}

unsigned long gModTime;

void ShaderRecurseCB(const char *dir, const char *file) {
    FileStat stat;
    MILO_ASSERT(FileGetStat(MakeString("%s/%s", dir, file), &stat) == 0, 0x1B);
    if (stat.st_mtime > gModTime) {
        gModTime = stat.st_mtime;
    }
}

unsigned long RndShaderProgram::InitModTime() {
    gModTime = 0;
    if (TheShaderMgr.CacheShaders()) {
        FileRecursePattern(
            MakeString("%s/shaders/*.fx", FileSystemRoot()), ShaderRecurseCB, false
        );
    }
    return gModTime;
}

void RndShaderProgram::CopyErrorShader(ShaderType shader, const ShaderOptions &opts) {
    if (!MainThread()) {
        MILO_NOTIFY(
            "missing shader %s_%llx cannot be cached (not used in main thread).",
            ShaderTypeName(shader),
            opts.flags
        );
    }
    MILO_ASSERT(shader != kErrorShader && shader != kPostprocessErrorShader, 0x12F);
    ShaderType errorType = kPostprocessShader ? kPostprocessErrorShader : kErrorShader;
    u64 mask = (errorType == kErrorShader && opts.flags & 0x1000) ? 0x1000 : 0;
    mask |= TheShaderMgr.GetShaderErrorDisplay() << 0x23;
    ShaderOptions newOpts(mask);
    RndShaderProgram &program = TheShaderMgr.FindShader(errorType, newOpts);
    if (!program.Cached()) {
        if (!TheShaderMgr.CacheShaders()) {
            MILO_LOG(
                "FAILURE: Error shader cannot be cached. Unable to handle missing shaders!\n"
            );
            MILO_FAIL(
                "FAILURE: Error shader cannot be cached. Unable to handle missing shaders!\n"
            );
        }
        Cache(errorType, newOpts, nullptr, nullptr);
    }
    Copy(program);
}

bool RndShaderProgram::Cache(
    ShaderType t, const ShaderOptions &opts, RndShaderBuffer *buf1, RndShaderBuffer *buf2
) {
    if (!mCached) {
        mCached = true;
        Platform p = TheLoadMgr.GetPlatform();
        if (p != kPlatformNone && p != kPlatformWii && GetGfxMode() != kOldGfx) {
            PhysMemTypeTracker tracker("D3D(phys):Shader");
            if (buf1 && buf1->Size() != 0 && buf2 && buf2->Size() != 0) {
                CreateVertexShader(*buf1);
                CreatePixelShader(*buf2, t);
                return true;
            } else if (!TheShaderMgr.CacheShaders()) {
                CopyErrorShader(t, opts);
                String str;
                ShaderMakeOptionsString(t, opts, str);
                // this needs to be all wrapped into another MakeString,
                // but our standalone MakeString is always inlined.
                // it needs to not be here
                MILO_NOTIFY(
                    "Missing shader %s_%llx\n(material: %s)\n(environment: %s)\n(compile options: %s)",
                    ShaderTypeName(t),
                    opts.flags,
                    PathName(NgMat::Current()),
                    PathName(RndEnviron::Current()),
                    str.c_str()
                );
                if (UsingCD()) {
                    SendDebugDataPoint(
                        MakeString(
                            "debug/%s/rnd/missing_shaders",
                            SystemConfig("rnd", "title")->Str(1)
                        ),
                        "type",
                        ShaderTypeName(t),
                        "flags",
                        MakeString("%llx", opts.flags),
                        "shader",
                        MakeString("%s_%llx", ShaderTypeName(t), opts.flags),
                        "mat",
                        PathName(NgMat::Current()),
                        "environ",
                        PathName(RndEnviron::Current())
                    );
                }
                return false;
            } else {
                AutoSlowFrame frame("RndShaderProgram::Cache", 5);
                s64 flags = opts.flags;
                char source[256];
                strcpy(source, ShaderSourcePath(ShaderTypeName(t)));
                char vertex[256];
                strcpy(vertex, ShaderCachedPath(source, flags, false));
                char pixel[256];
                strcpy(pixel, ShaderCachedPath(source, flags, true));
                FileStat stat;
                int i14 = 0;
                if (FileGetStat(vertex, &stat) == 0) {
                    i14 = stat.st_mtime;
                }
                if (FileGetStat(pixel, &stat) == 0) {
                    if (stat.st_mtime < i14) {
                        i14 = stat.st_mtime;
                    }
                } else {
                    i14 = 0;
                }
                if (gModTime > i14) {
                    static DataNode &n = DataVariable("shader_compile_print_opts");
                    if (n.Int()) {
                        String str;
                        ShaderMakeOptionsString(t, opts, str);
                        MILO_LOG(
                            "Compiling shader: %s_%llx (%s) (compile options: %s)\n",
                            ShaderTypeName(t),
                            flags,
                            PlatformSymbol(p),
                            str.c_str()
                        );
                    } else {
                        MILO_LOG(
                            "Compiling shader: %s_%llx (%s)\n",
                            ShaderTypeName(t),
                            flags,
                            PlatformSymbol(p)
                        );
                    }
                    if (!MainThread() || !Compile(t, opts, buf1, buf2)) {
                        CopyErrorShader(t, opts);
                        return false;
                    }
                    SaveShaderBuffer(vertex, *buf1);
                    SaveShaderBuffer(pixel, *buf2);
                } else {
                    LoadShaderBuffer(vertex, buf1);
                    LoadShaderBuffer(pixel, buf2);
                }
                CreateVertexShader(*buf1);
                CreatePixelShader(*buf2, t);
                delete buf1;
                delete buf2;
            }
        }
    }
    return true;
}
