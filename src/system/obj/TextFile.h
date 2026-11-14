#pragma once
#include "utl/TextStream.h"
#include "obj/Object.h"
#include "os/File.h"

class TextFile : public Hmx::Object, public TextStream {
private:
    File *mFile;
    TextFile() : mFile(nullptr) {}

    DataNode OnPrint(DataArray *);
    DataNode OnPrintf(DataArray *);
    DataNode OnReflect(DataArray *);

public:
    virtual ~TextFile() { delete mFile; }
    OBJ_CLASSNAME(File);
    OBJ_SET_TYPE(TextFile);
    virtual DataNode Handle(DataArray *, bool);
    virtual void SetName(const char *, class ObjectDir *);
    virtual void Print(const char *);

    NEW_OBJ(TextFile);
    REGISTER_OBJ(TextFile)
};
