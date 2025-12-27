#pragma once
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Tex.h"
#include "ui/UIPanel.h"
#include "xdk/XAPILIB.h"
#include "xdk/XSOCIAL.h"
#include "xdk/xsocial/xsocial.h"

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

    NEW_OBJ(KinectSharePanel)

    bool XSocialImagePending() const { return mOverlapped.hEvent; }

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
    int unk58; // 0x58
    XOVERLAPPED mOverlapped; // 0x5c
    XSOCIAL_IMAGEPOSTPARAMS mImagePostParams; // 0x78
    XSOCIAL_LINKPOSTPARAMS mLinkPostParams; // 0xa8
};
