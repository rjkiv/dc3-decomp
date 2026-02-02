#include "meta_ham/KinectSharePanel.h"
#include "jpeg/Jpeg.h"
#include "meta_ham/AccomplishmentManager.h"
#include "meta_ham/HamProfile.h"
#include "meta_ham/ProfileMgr.h"
#include "net_ham/KinectShareJobs.h"
#include "net_ham/RockCentral.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Msg.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "os/PlatformMgr.h"
#include "rnddx9/Rnd.h"
#include "rndobj/Bitmap.h"
#include "rndobj/Tex.h"
#include "rndobj/Utl.h"
#include "ui/UIPanel.h"
#include "utl/Locale.h"
#include "utl/MemMgr.h"
#include "utl/Symbol.h"
#include "utl/UTF8.h"
#include "xdk/XAPILIB.h"
#include "xdk/win_types.h"
#include "xdk/xapilibi/synchapi.h"
#include "xdk/xapilibi/winerror.h"
#include "xdk/xsocial/xsocial.h"
#include <cstring>
#include <cwchar>

void SetLocalizedFBText(const Symbol &s, wchar_t *wStr) {
    const char *localized = Localize(s, nullptr, TheLocale);
    if (localized && strlen(localized) != 0) {
        UTF8toWChar_t(wStr, localized);
    }
}

KinectSharePanel::KinectSharePanel()
    : mTex(this), unk4c(0), mBuf(0), mPreviewBuf(0), unk58(0) {
    memset(&mOverlapped, 0, sizeof(XOVERLAPPED));
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
    if (mOverlapped.hEvent && mOverlapped.InternalLow != 0x3E5) {
        DWORD dw;
        XGetOverlappedResult(&mOverlapped, &dw, false);
        MILO_LOG(
            "KinectSharePanel asynch I/O completed 0x%08x\n", mOverlapped.dwExtendedError
        );
        if (mOverlapped.dwExtendedError != 0) {
            unk4c = mOverlapped.dwExtendedError == 0x4C7 ? 4 : 3;
        } else {
            unk4c = 2;
        }
        if (mOverlapped.hEvent) {
            CloseHandle(mOverlapped.hEvent);
            mOverlapped.hEvent = nullptr;
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
        mImagePostParams.FullImageByteCount = iref;
        mImagePostParams.pFullImage = (const BYTE *)mBuf;
    }
    {
        static int _x = MemFindHeap("physical");
        MemHeapTracker mem(_x);
        mImagePostParams.PreviewImage.Pitch = bitmapa0.RowBytes();
        mImagePostParams.PreviewImage.Width = bitmapa0.Width();
        mImagePostParams.PreviewImage.Height = bitmapa0.Height();
        mImagePostParams.PreviewImage.Format = TheDxRnd.D3DFormatForBitmap(bitmapa0);
        if (mImagePostParams.PreviewImage.Format == D3DFMT_A8R8G8B8) {
            mImagePostParams.PreviewImage.Format = D3DFMT_LIN_A8R8G8B8;
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
        mImagePostParams.PreviewImage.pBytes = (BYTE *)mPreviewBuf;
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
        mLinkPostParams.PreviewImage.Pitch = bitmap90.RowBytes();
        mLinkPostParams.PreviewImage.Width = bitmap90.Width();
        mLinkPostParams.PreviewImage.Height = bitmap90.Height();
        mLinkPostParams.PreviewImage.Format = TheDxRnd.D3DFormatForBitmap(bitmap90);
        if (mLinkPostParams.PreviewImage.Format == D3DFMT_A8R8G8B8) {
            mLinkPostParams.PreviewImage.Format = D3DFMT_LIN_A8R8G8B8;
        }
        int mult = bitmap90.Height() * bitmap90.RowBytes();
        mPreviewBuf = MemAlloc(mult, __FILE__, 0xA8, "FB_Preview");
        MILO_ASSERT(mPreviewBuf != NULL, 0xA9);
        if (mPreviewBuf) {
            memcpy(mPreviewBuf, bitmap90.Pixels(), mult);
        }
        mLinkPostParams.PreviewImage.pBytes = (BYTE *)mPreviewBuf;
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

DataNode KinectSharePanel::OnPostLink(DataArray *a) {
    MILO_ASSERT(!mBuf && ! mPreviewBuf, 0x163);
    MILO_LOG(
        "KinectSharePanel::OnUpload() - mTex:[%d %d %d %d]\n",
        mTex->Width(),
        mTex->Height(),
        mTex->Bpp(),
        mTex->PowerOf2()
    );
    ConvertImagesForLinkPost();
    mOverlapped.InternalContext = 0;
    mOverlapped.InternalHigh = 0;
    mOverlapped.InternalLow = 0;
    mOverlapped.hEvent = CreateEventA(nullptr, true, false, "SocialNetworkLinkPost");
    if (!mOverlapped.hEvent) {
        MILO_LOG("KinectSharePanel: mOverlapped.hEvent is null");
    } else {
        mOverlapped.dwCompletionContext = (DWORD_PTR)this;
        mOverlapped.pCompletionRoutine = nullptr;
        mOverlapped.dwExtendedError = 0;
        WCHAR caption[256];
        WCHAR description[256];
        WCHAR linkUrl[256];
        WCHAR pictureUrl[256];
        WCHAR title[256];
        memcpy(title, L"Title Text", 0x16);
        memset(&title[0x16 / 2], 0, sizeof(title) - 0x16);
        memcpy(linkUrl, L"http://www.dancecentral.com/", 0x3A);
        memset(&linkUrl[0x3A / 2], 0, sizeof(linkUrl) - 0x3A);
        memcpy(caption, L"Picture Caption", 0x20);
        memset(&caption[0x20 / 2], 0, sizeof(caption) - 0x20);
        memcpy(description, L"Picture Description", 0x28);
        memset(&description[0x28 / 2], 0, sizeof(description) - 0x28);
        memcpy(
            pictureUrl,
            L"http://www.dancecentral.com/content-assets/2012/06/2012E3LogoBox_tn.jpg",
            0x90
        );
        memset(&pictureUrl[0x90 / 2], 0, sizeof(pictureUrl) - 0x90);
        static Symbol fb_link_title_text("fb_link_title_text");
        static Symbol fb_link_title_url("fb_link_title_url");
        static Symbol fb_link_picture_caption("fb_link_picture_caption");
        static Symbol fb_link_picture_description("fb_link_picture_description");
        static Symbol fb_link_picture_url("fb_link_picture_url");
        SetLocalizedFBText(fb_link_title_text, title);
        SetLocalizedFBText(fb_link_title_url, linkUrl);
        SetLocalizedFBText(fb_link_picture_caption, caption);
        SetLocalizedFBText(fb_link_picture_description, description);
        SetLocalizedFBText(fb_link_picture_url, pictureUrl);
        mLinkPostParams.Size = 0x30;
        mLinkPostParams.TitleText = title;
        mLinkPostParams.TitleURL = linkUrl;
        mLinkPostParams.PictureCaption = caption;
        mLinkPostParams.PictureDescription = description;
        mLinkPostParams.PictureURL = pictureUrl;
        mLinkPostParams.Flags = 4;
        int padnum = 0;
        HamProfile *profile = TheProfileMgr.GetActiveProfile(true);
        if (profile) {
            padnum = profile->GetPadNum();
        }
        DWORD dw;
        DWORD res;
        if (PlatformMgr::sXShowCallback(dw)) {
            res = XShowNuiSocialNetworkLinkPostUI(
                dw, padnum, &mLinkPostParams, &mOverlapped
            );
            MILO_LOG(
                "KinectSharePanel: XShowNuiSocialNetworkLinkPostUI returns %x\n", res
            );
        } else {
            res = XShowSocialNetworkLinkPostUI(padnum, &mLinkPostParams, &mOverlapped);
        }
        if (res == ERROR_IO_PENDING) {
            unk4c = 1;
        } else if (res == ERROR_SUCCESS) {
            unk4c = 2;
        } else {
            unk4c = 3;
        }
    }
    return DataNode(kDataInt, 0);
}

DataNode KinectSharePanel::OnCleanup(DataArray *a) {
    MILO_LOG("KinectSharePanel: cleaning up\n");
    RELEASE(mTex);
    if (mBuf) {
        MemFree(mBuf, __FILE__, 0x1CA);
        mBuf = nullptr;
    }
    if (mPreviewBuf) {
        MemFree(mPreviewBuf, __FILE__, 0x1CB);
        mPreviewBuf = nullptr;
    }
    return 0;
}

DataNode KinectSharePanel::OnUpload(DataArray *arr) {
    MILO_ASSERT(!mBuf && ! mPreviewBuf, 0x115);
    MILO_LOG(
        "KinectSharePanel::OnUpload() - mTex:[%d %d %d %d]\n",
        mTex->Width(),
        mTex->Height(),
        mTex->Bpp(),
        mTex->PowerOf2()
    );
    ConvertImages();
    mOverlapped.InternalContext = (DWORD_PTR)this;
    mOverlapped.InternalHigh = 0;
    mOverlapped.InternalLow = 0;
    mOverlapped.hEvent = CreateEventA(0, 1, 0, "SocialNetworkImagePost");
    if (!mOverlapped.hEvent) {
        MILO_LOG("KinectSharePanel: mOverlapped.hEvent is null");
    } else {
        mOverlapped.dwCompletionContext = 0;
        mOverlapped.pCompletionRoutine = nullptr;
        mOverlapped.dwExtendedError = 0;

        WCHAR TitleText[256];
        WCHAR PictureCaption[256];
        WCHAR PictureDescription[256];

        memcpy(TitleText, L"Title Text", 0x16);
        memset(&TitleText, 0, 0x1ea);
        memcpy(PictureCaption, L"Picture Caption", 0x20);
        memset(&PictureCaption, 0, 0x1e0);
        memcpy(PictureDescription, L"Picture Description", 0x28);
        memset(&PictureDescription, 0, 0x1d8);

        static Symbol fb_photo_title_text("fb_photo_title_text");
        static Symbol fb_photo_picture_caption("fb_photo_picture_caption");
        static Symbol fb_photo_picture_description("fb_photo_picture_description");

        SetLocalizedFBText(fb_photo_title_text, TitleText);
        SetLocalizedFBText(fb_photo_picture_caption, PictureCaption);
        SetLocalizedFBText(fb_photo_picture_description, PictureDescription);

        mImagePostParams.Size = 0x30;
        mImagePostParams.TitleText = TitleText;
        mImagePostParams.PictureCaption = PictureCaption;
        mImagePostParams.PictureDescription = PictureDescription;
        mImagePostParams.Flags = 6;

        int padNum = 0;
        HamProfile *pActiveProfile = TheProfileMgr.GetActiveProfile(true);
        if (pActiveProfile) {
            padNum = pActiveProfile->GetPadNum();
        }
        DWORD dw;
        DWORD res;
        if (PlatformMgr::sXShowCallback(dw)) {
            res = XShowNuiSocialNetworkImagePostUI(
                dw, padNum, &mImagePostParams, &mOverlapped
            );
            MILO_LOG(
                "KinectSharePanel: XShowNuiSocialNetworkImagePostUI returns %x\n", res
            );
        } else {
            res = XShowSocialNetworkImagePostUI(padNum, &mImagePostParams, &mOverlapped);
        }

        if (res == ERROR_IO_PENDING) {
            unk4c = 1;
        } else if (res == ERROR_SUCCESS) {
            unk4c = 2;
        } else {
            unk4c = 3;
        }
    }
    return DataNode(kDataInt, 0);
}
