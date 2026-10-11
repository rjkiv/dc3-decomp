#include "meta_ham/DepthBuffer.h"
#include "meta_ham/HamUI.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UIPanel.h"

DepthBuffer::DepthBuffer()
    : mCursorPanel(0), mState(kDepthBuffer_Normal), mWantGameplayStateTransition(0) {}

BEGIN_HANDLERS(DepthBuffer)
    HANDLE_ACTION(set_state, SetState((DepthBufferState)_msg->Int(2)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void DepthBuffer::Init(UIPanel *panel) {
    SetName("depth_buffer", ObjectDir::Main());
    mCursorPanel = panel;
}

void DepthBuffer::Poll() {
    static bool sIsGameplayPanel = false;
    TheHamUI.GetShellInput()->HasSkeleton();
    bool ispanel = TheHamUI.GetShellInput()->IsGameplayPanel();
    if (ispanel) {
        MILO_ASSERT(mState == kDepthBuffer_Normal, 0x3A);
    }
    if (ispanel && (!sIsGameplayPanel || mWantGameplayStateTransition)) {
        mWantGameplayStateTransition = false;
    }
    sIsGameplayPanel = ispanel;
}
