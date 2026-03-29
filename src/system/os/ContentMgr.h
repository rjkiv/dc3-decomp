#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/MemMgr.h"
#include "utl/FilePath.h"

enum ContentLocT {
    kLocationRemovableMem,
    kLocationHDD,
    kLocationRoot
};

class Content {
public:
    enum State {
        kUnmounted = 0,
        kNeedsMounting = 1,
        kMounting = 2,
        kUnmounting = 3,
        kMounted = 4,
        kAlwaysMounted = 5,
        kNeedsBackup = 6,
        kBackingUp = 7,
        kContentDeleting = 8,
        kDeleted = 9,
        kFailed = 10
    };

    Content() {}
    virtual ~Content() {}
    virtual const char *Root() = 0;
    virtual bool OnMemcard() = 0;
    virtual ContentLocT Location() = 0;
    virtual unsigned long LicenseBits() { return 0; }
    virtual bool HasValidLicenseBits() { return true; }
    virtual bool IsCorrupt() { return false; }
    virtual State GetState() = 0;
    virtual void Poll() {}
    virtual void Mount() {}
    virtual void Unmount() {}
    virtual void Delete() {}
    virtual Symbol FileName() = 0;
    virtual const char *DisplayName() = 0;
    virtual unsigned int GetLRM() { return 0; }

    bool Contains(const char *);

    MEM_OVERLOAD(Content, 0x24);
};

class RootContent : public Content {
public:
    RootContent(const char *str) : mRoot(str) {}
    virtual const char *Root() { return mRoot.c_str(); }
    virtual bool OnMemcard() { return false; }
    virtual ContentLocT Location() { return kLocationRoot; }
    virtual unsigned long LicenseBits() { return 0; }
    virtual bool HasValidLicenseBits() { return true; }
    virtual bool IsCorrupt() { return false; }
    virtual State GetState() { return kAlwaysMounted; }
    virtual void Poll() {}
    virtual void Mount() {}
    virtual void Unmount() {}
    virtual void Delete() {}
    virtual Symbol FileName() { return mRoot.c_str(); }
    virtual const char *DisplayName() { return mRoot.c_str(); }
    virtual unsigned int GetLRM() { return 0; }

private:
    String mRoot; // 0x4
};

class ContentMgr : public Hmx::Object {
public:
    class Callback {
    public:
        Callback() {}
        virtual ~Callback() {}
        virtual void ContentStarted() {}
        virtual bool ContentDiscovered(Symbol contentName) { return true; }
        virtual bool ContentTitleDiscovered(unsigned int, Symbol) { return true; }
        virtual void ContentMountBegun(int) {}
        virtual void ContentAllMounted() {}
        virtual void ContentMounted(const char *contentName, const char *) {}
        virtual void ContentUnmounted(const char *contentName) {}
        virtual void ContentFailed(const char *contentName) {}
        virtual void ContentLoaded(class Loader *, ContentLocT location, Symbol) {}
        virtual void ContentDone() {}
        virtual const char *ContentPattern() { return ""; }
        virtual const char *ContentDir() { return "."; }
        virtual std::vector<String> *ContentAltDirs() { return nullptr; }
        virtual bool HasContentAltDirs() { return false; }
    };

    struct CallbackFile {
        CallbackFile(const char *cc1, Callback *cb, ContentLocT t, const char *cc2);
        ~CallbackFile() {}

        FilePath mFile; // 0x0
        Callback *mCallback; // 0x8
        ContentLocT mLocation; // 0xc
        String mName; // 0x10
    };

    ContentMgr() {}
    virtual DataNode Handle(DataArray *, bool);
    virtual void PreInit() {}
    virtual void Init();
    virtual void Terminate() {}
    virtual void StartRefresh() {}
    virtual void PollRefresh();
    virtual const char *TitleContentPath() { return nullptr; }
    virtual const char *ContentPath(int) { return 0; }
    virtual bool MountContent(Symbol) { return true; }
    virtual bool IsMounted(Symbol) { return true; }
    virtual bool IsCorrupt(Symbol, const char *&) { return false; }
    virtual bool DeleteContent(Symbol) { return true; }
    virtual bool IsDeleteDone(Symbol) { return true; }
    virtual bool GetLicenseBits(Symbol, unsigned long &ul) {
        ul = 0;
        return true;
    }
    virtual unsigned int GetCreationDate(Symbol) { return 0; }

    bool NeverRefreshed() const { return mState == kDone; }
    bool RefreshDone() const;
    bool RefreshInProgress();
    Hmx::Object *SetReadFailureHandler(Hmx::Object *);
    void RefreshSynchronously();
    void OnReadFailure(bool, const char *);
    bool Contains(const char *, String &);
    void RegisterCallback(Callback *, bool);
    void UnregisterCallback(Callback *, bool);

private:
    DataNode OnAddContent(DataArray *);
    DataNode OnRemoveContent(DataArray *);

protected:
    virtual void NotifyMounted(Content *) {}
    virtual void NotifyUnmounted(Content *) {}
    virtual void NotifyDeleted(Content *) {}
    virtual void NotifyFailed(Content *) {}

    void AddCallbackFile(const char *, const char *);
    static void RecurseCallback(const char *, const char *);

    enum {
        kDone = 0,
        kDiscoveryEnumerating = 1,
        kDiscoveryMounting = 2,
        kDiscoveryLoading = 3,
        kDiscoveryCheckIfDone = 4,
        kMounting = 5,
        kContentMgrState6 = 6
    } mState; // 0x2c
    std::list<Callback *> mCallbacks; // 0x30
    std::list<Content *> mContents; // 0x38
    std::list<String> mExtraContents; // 0x40
    bool mDirty; // 0x48
    Loader *mLoader; // 0x4c
    Callback *mCallback; // 0x50
    ContentLocT mLocation; // 0x54
    String mName; // 0x58
    int mRootLoaded; // 0x60
    std::list<CallbackFile> mCallbackFiles; // 0x64
    Hmx::Object *mReadFailureHandler; // 0x6c
};

extern ContentMgr &TheContentMgr;

#include "obj/Msg.h"

DECLARE_MESSAGE(ContentReadFailureMsg, "content_read_failure");
ContentReadFailureMsg(bool b, const char *cc) : Message(Type(), b, cc) {}
// TODO: rename these methods once you actually know what the bool and const char*
// represent
bool GetBool() const { return mData->Int(2); }
const char *GetStr() const { return mData->Str(3); }
END_MESSAGE

DECLARE_MESSAGE(ContentInstalledMsg, "content_installed")
ContentInstalledMsg() : Message(Type()) {}
END_MESSAGE
