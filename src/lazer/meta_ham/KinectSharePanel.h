#pragma once
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "rndobj/Tex.h"
#include "ui/UIPanel.h"
#include "xdk/d3d9i/d3d9types.h"
#include "xdk/xapilibi/xbase.h"

class KinectSharePanel : public UIPanel {
public:
    KinectSharePanel();
    // Hmx::Object
    OBJ_CLASSNAME(KinectSharePanel);
    OBJ_SET_TYPE(KinectSharePanel);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    // UIPanel
    virtual void Poll();

    bool XSocialImagePending() const { return mXOverlapped.hEvent; }

private:
    void ConvertImages();
    void ConvertImagesForLinkPost();

    DataNode OnUpload(DataArray *);
    DataNode OnPostLink(DataArray *);
    DataNode OnCleanup(DataArray *);
    DataNode OnMsg(const RockCentralOpCompleteMsg &);

    ObjPtr<RndTex> mTex; // 0x38
    int unk4c; // 0x4c - upload state
    void *mBuf; // 0x50
    void *mPreviewBuf; // 0x54
    int unk58;
    XOVERLAPPED mXOverlapped; // 0x5c
    int unk78;
    int unk7c;
    int unk80;
    int unk84;
    void *unk88;
    int unk8c;
    int unk90;
    int unk94;
    D3DFORMAT unk98;
    void *unk9c;
    int unka0;
    int unka4;
    int unka8;
    int unkac;
    int unkb0;
    int unkb4;
    int unkb8;
    int unkbc;
    void *unkc0;
    int unkc4;
    int unkc8;
    int unkcc;
    D3DFORMAT unkd0;
    int unkd4;
};
