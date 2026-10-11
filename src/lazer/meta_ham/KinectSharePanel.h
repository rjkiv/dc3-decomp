#pragma once
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Tex.h"
#include "ui/UIPanel.h"
#include "xdk/XAPILIB.h"
#include "xdk/XSOCIAL.h"

class KinectSharePanel : public UIPanel {
public:
    enum UploadStatus {
        Upload_Idle = 0x0000,
        Upload_InProgress = 0x0001,
        Upload_Success = 0x0002,
        Upload_Failure = 0x0003,
        Upload_Cancelled = 0x004
    };
    KinectSharePanel();
    // Hmx::Object
    OBJ_CLASSNAME(KinectSharePanel);
    OBJ_SET_TYPE(KinectSharePanel);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    // UIPanel
    virtual void Poll();

    NEW_OBJ(KinectSharePanel)

private:
    void ConvertImages();
    void ConvertImagesForLinkPost();
    // why is this private???
    bool XSocialNetworkImagePostPending() const { return mOverlapped.hEvent; }

    DataNode OnUpload(DataArray *);
    DataNode OnPostLink(DataArray *);
    DataNode OnCleanup(DataArray *);
    DataNode OnMsg(const RockCentralOpCompleteMsg &);

    ObjPtr<RndTex> mTex; // 0x38
    UploadStatus mUploadStatus; // 0x4c - upload state
    void *mBuf; // 0x50
    void *mPreviewBuf; // 0x54
    int mBufLen; // 0x58
    XOVERLAPPED mOverlapped; // 0x5c
    XSOCIAL_IMAGEPOSTPARAMS mImagePostParams; // 0x78
    XSOCIAL_LINKPOSTPARAMS mLinkPostParams; // 0xa8
};
