#include "meta_ham/KinectSharePanel.h"
#include "jpeg/Jpeg.h"
#include "meta_ham/AccomplishmentManager.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/ProfileMgr.h"
#include "net_ham/KinectShareJobs.h"
#include "net_ham/RockCentral.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rnddx9/Rnd.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Tex.h"
#include "rndobj/Utl.h"
#include "ui/UIPanel.h"
#include "utl/Locale.h"
#include "utl/MemMgr.h"
#include "utl/UTF8.h"
#include "xdk/XAPILIB.h"

void SetLocalizedFBText(const Symbol &s, wchar_t *wStr) {
    const char *localized = Localize(s, nullptr, TheLocale);
    if (localized && strlen(localized) != 0) {
        UTF8toWChar_t(wStr, localized);
    }
}

KinectSharePanel::KinectSharePanel()
    : mTex(this), unk4c(0), mBuf(0), mPreviewBuf(0), unk58(0) {
    memset(&mXOverlapped, 0, sizeof(XOVERLAPPED));
}

BEGIN_HANDLERS(KinectSharePanel)
    HANDLE(upload, OnUpload)
    HANDLE(post_link, OnPostLink)
    HANDLE(cleanup, OnCleanup)
    HANDLE_EXPR(xsocial_image_pending, XSocialImagePending())
    HANDLE_MESSAGE(RockCentralOpCompleteMsg)
    HANDLE_SUPERCLASS(UIPanel)
END_HANDLERS

BEGIN_PROPSYNCS(KinectSharePanel)
    SYNC_PROP(texture, mTex)
END_PROPSYNCS

void KinectSharePanel::Poll() {
    UIPanel::Poll();
    switch (unk4c) {
    case 2: {
        MILO_LOG("KinectSharePanel reporting successful upload\n");
        RockCentralOpCompleteMsg msg(true, -1, 0);
        Handle(msg, true);
        HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
        static Symbol acc_photo_share("acc_photo_share");
        TheAccomplishmentMgr->EarnAccomplishmentForProfile(
            profile, acc_photo_share, false
        );
        unk4c = 0;
        break;
    }
    case 3: {
        MILO_LOG("KinectSharePanel reporting failed upload\n");
        RockCentralOpCompleteMsg msg(false, -1, 0);
        Handle(msg, true);
        unk4c = 0;
        break;
    }
    case 4: {
        MILO_LOG("KinectSharePanel reporting cancelled upload\n");
        RockCentralOpCompleteMsg msg(true, -1, 1);
        Handle(msg, true);
        unk4c = 0;
        break;
    }
    default:
        break;
    }
    if (mXOverlapped.hEvent && mXOverlapped.InternalLow != 0x3E5) {
        DWORD dw;
        XGetOverlappedResult(&mXOverlapped, &dw, false);
        MILO_LOG(
            "KinectSharePanel asynch I/O completed 0x%08x\n", mXOverlapped.dwExtendedError
        );
        if (mXOverlapped.dwExtendedError != 0) {
            unk4c = mXOverlapped.dwExtendedError == 0x4C7 ? 4 : 3;
        } else {
            unk4c = 2;
        }
        if (mXOverlapped.hEvent) {
            CloseHandle(mXOverlapped.hEvent);
            mXOverlapped.hEvent = nullptr;
        }
    }
}

void KinectSharePanel::ConvertImages() {
    MILO_ASSERT(mTex.Ptr(), 0x2B);
    RndBitmap bitmapa0;
    RndBitmap bitmap80;
    mTex->LockBitmap(bitmapa0, true);
    EndianSwapBitmap(bitmapa0);
    bitmap80.Create(bitmapa0, 24, 1, nullptr);
    int w = bitmap80.Width();
    int h = bitmap80.Height();
    int bpp = bitmap80.Bpp() >> 3;
    int pixelBytes = bitmap80.PixelBytes();
    char *pixels = (char *)bitmap80.Pixels();
    mBuf = MemAlloc(pixelBytes, __FILE__, 0x3B, "JpegWriter");
    MILO_ASSERT(mBuf != NULL, 0x3C);
    if (mBuf) {
        int iref;
        LoadBitmapIntoJpeg(pixels, w, h, bpp, mBuf, iref);
        unk58 = iref;
        unka0 = iref;
        unk9c = mBuf;
    }
    {
        static int _x = MemFindHeap("physical");
        MemHeapTracker mem(_x);
        unk8c = bitmapa0.RowBytes();
        unk90 = bitmapa0.Width();
        unk94 = bitmapa0.Height();
        unk98 = TheDxRnd.D3DFormatForBitmap(bitmapa0);
        if (unk98 == D3DFMT_A8R8G8B8) {
            unk98 = D3DFMT_LIN_A8R8G8B8;
        }
        int mult = bitmapa0.Height() * bitmapa0.RowBytes();
        mPreviewBuf = MemAlloc(mult, __FILE__, 0x56, "FB_Preview");
        MILO_ASSERT(mPreviewBuf != NULL, 0x57);
        if (mPreviewBuf) {
            EndianSwapBitmap(bitmapa0);
            memcpy(mPreviewBuf, bitmapa0.Pixels(), mult);
            unsigned char *previewPtr = (unsigned char *)mPreviewBuf;
            int area = bitmapa0.Height() * bitmapa0.Width();
            for (int i = 0; i < area; i++) {
                previewPtr[i * 4] = -1;
            }
        }
        unk88 = mPreviewBuf;
        MILO_LOG(
            "KinectSharePanel: preview bitmap = 0x%08x %d %d %d 0x%08x\n",
            mPreviewBuf,
            bitmapa0.Width(),
            bitmapa0.Height(),
            bitmapa0.RowBytes(),
            TheDxRnd.D3DFormatForBitmap(bitmapa0)
        );
    }
    mTex->UnlockBitmap();
}

void KinectSharePanel::ConvertImagesForLinkPost() {
    ObjectDir *dataDir = DataDir();
    if (dataDir) {
        RndTex *pTex = dataDir->Find<RndTex>("game_logo.tex");
        MILO_ASSERT(pTex, 0x85);
        if (pTex) {
            mTex = pTex;
        }
    }
    MILO_ASSERT(mTex.Ptr(), 0x8C);
    RndBitmap bitmap70;
    RndBitmap bitmap90;
    mTex->LockBitmap(bitmap70, 3);
    bitmap90.Create(bitmap70, 32, 0, nullptr);
    EndianSwapBitmap(bitmap90);
    bitmap90.PixelBytes();
    {
        static int _x = MemFindHeap("physical");
        MemHeapTracker mem(_x);
        unkc4 = bitmap90.RowBytes();
        unkc8 = bitmap90.Width();
        unkcc = bitmap90.Height();
        unkd0 = TheDxRnd.D3DFormatForBitmap(bitmap90);
        if (unkd0 == D3DFMT_A8R8G8B8) {
            unkd0 = D3DFMT_LIN_A8R8G8B8;
        }
        int mult = bitmap90.RowBytes() * bitmap90.Height();
        mPreviewBuf = MemAlloc(mult, __FILE__, 0xA8, "FB_Preview");
        MILO_ASSERT(mPreviewBuf != NULL, 0xA9);
        if (mPreviewBuf) {
            memcpy(mPreviewBuf, bitmap90.Pixels(), mult);
        }
        unkc0 = mPreviewBuf;
        MILO_LOG(
            "KinectSharePanel: preview bitmap = 0x%08x %d %d %d 0x%08x\n",
            mPreviewBuf,
            bitmap90.Width(),
            bitmap90.Height(),
            bitmap90.RowBytes(),
            TheDxRnd.D3DFormatForBitmap(bitmap90)
        );
    }
    mTex->UnlockBitmap();
}

DataNode KinectSharePanel::OnMsg(const RockCentralOpCompleteMsg &msg) {
    if (msg.Success()) {
        if (msg.Arg2().Int() == 0) {
            Message msg("upload_complete", "endgame_kinectshare_complete");
            HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
            static Symbol acc_facebook("acc_facebook");
            TheAccomplishmentMgr->EarnAccomplishmentForProfile(
                profile, acc_facebook, false
            );
            HandleType(msg);
            TheRockCentral.ManageJob(new KinectShareJob(nullptr));
        } else {
            Message msg("upload_cancelled", "endgame_kinectshare_complete");
            HandleType(msg);
        }
    } else {
        Message msg("upload_complete", "leaderboard_no_net");
        HandleType(msg);
    }
    return 1;
}
