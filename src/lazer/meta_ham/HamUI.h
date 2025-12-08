#pragma once

#include "lazer/meta_ham/HelpBarPanel.h"
#include "lazer/meta_ham/ShellInput.h"
#include "meta/ConnectionStatusPanel.h"
#include "os/ContentMgr.h"
#include "os/JoypadMsgs.h"
#include "os/PlatformMgr.h"
#include "rndobj/Mesh.h"
#include "ui/UI.h"

class HamUI : public UIManager {
public:
    HamUI();
    virtual ~HamUI();
    virtual DataNode Handle(DataArray *, bool);
    virtual void Init();
    virtual void Terminate();

    void ForceLetterboxOff();
    void ForceLetterboxOffImmediate();

    HelpBarPanel *unk_0xD8;
    u32 unk_0xDC; // LetterboxPanel*
    u32 unk_0xE0;
    u32 unk_0xE4;
    u32 unk_0xE8;
    u32 unk_0xEC;
    u32 unk_0xF0;
    u32 unk_0xF4;
    Hmx::Object *unk_0xF8; // TODO figure type
    u8 unk_0xFC;
    u8 unk_0xFD;
    u32 unk_0x100;
    ShellInput *unk_0x104;
    s32 unk_0x108;
    u32 unk_0x10C;
    bool mFullScreenDrawActive;
    float mSkelRot;
    bool unk_0x118;
    u32 unk_0x11C;

protected:
    DataNode OnMsg(const UITransitionCompleteMsg &);
    DataNode OnMsg(const ContentReadFailureMsg &);
    DataNode OnMsg(const ConnectionStatusChangedMsg &);
    DataNode OnMsg(const DiskErrorMsg &);
    DataNode OnMsg(const ButtonDownMsg &);
    // DataNode OnMsg(const KinectGuideGestureMsg &);

private:
    bool SetFullScreenDraw(bool);
    Symbol DisplayNextCameraOutput();
    float NextSkeletonDrawRot();
    void ResetSnapshots();
    int TakeSnapshot();
    int NumSnapshots();
    void ApplySnapshotToMesh(int, RndMesh *);
    void InitTextureStore(int);
    void ClearTextureStore();
    int NumStoredTextures() const;
    int StoreTexture(RndTex *);
    void StoreTextureAt(RndTex *, int);
    void StoreTextureClipAt(float, float, float, float, int, int);
    RndTex *GetStoredTexture(int) const;
    void ApplyTextureClip(RndMat *, int) const;
    void StoreColorBufferAt(int);
    void StoreColorBufferClipAt(float, float, float, float, int);
    void StoreDepthBufferAt(int);
    void StoreDepthBufferClipAt(float, float, float, float, int);
    void ReloadStrings();
};

extern HamUI TheHamUI;
