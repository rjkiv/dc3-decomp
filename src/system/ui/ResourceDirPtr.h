#pragma once
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "obj/PropSync.h"
#include "os/File.h"
#include "utl/BinStream.h"
#include "utl/FilePath.h"
#include "utl/Loader.h"
#include "utl/Symbol.h"

class ResourceDirBase {
public:
    static bool MakeResourcePath(FilePath &, Symbol, Symbol, const char *);
    static DataNode GetFileList(Symbol, Symbol);

    DataNode GetFileList(Symbol s) { return GetFileList(mOwner->ClassName(), s); }

protected:
    ResourceDirBase(Hmx::Object *owner) : mOwner(owner) {}

    static const char *GetResourcesPath(Symbol, Symbol);

    Hmx::Object *mOwner; // 0x0
};

template <class T>
class ResourceDirPtr : public ObjDirPtr<T>, public ResourceDirBase {
public:
    // i have no idea if this is right
    ResourceDirPtr(Hmx::Object *owner) : ResourceDirBase(owner) {}
    ResourceDirPtr &operator=(const ResourceDirPtr &other) {
        ObjDirPtr<T>::operator=((T *)other);
        mOwner = other.mOwner;
        return *this;
    }

    const char *GetName() const { return FileGetBase(GetFile().c_str()); }
    void SetName(const char *name, bool b2) {
        FilePath path;
        // maybe classname is accessed via a ResourceDirBase helper?
        // something going on where the ResourceDirBase is addi'ed
        if (MakeResourcePath(path, mOwner->ClassName(), T::StaticClassName(), name)) {
            LoadFile(path, b2, true, kLoadFront, false);
        } else {
            ObjDirPtr<T>::operator=(nullptr);
        }
    }
};

template <class T>
BinStream &operator>>(BinStream &bs, ResourceDirPtr<T> &ptr) {
    FilePath path;
    bs >> path;
    ptr.LoadFile(path, true, true, kLoadFront, false);
    return bs;
}

template <class T>
bool PropSync(ResourceDirPtr<T> &ptr, DataNode &node, DataArray *prop, int i, PropOp op) {
    if (i < prop->Size()) {
        static Symbol list("list");
        static Symbol file_path("file_path");
        Symbol sym = prop->Sym(prop->Size() - 1);
        if (sym == list) {
            node = ptr.GetFileList(T::StaticClassName());
            return true;
        } else if (sym == file_path) {
            node = ptr.GetFile();
            return true;
        }
    }
    if (op == kPropGet) {
        node = ptr.GetName();
    } else {
        ptr.SetName(node.Str(), false);
    }
    return true;
}
