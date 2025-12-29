#include "meta_ham/DepthBuffer.h"
#include "meta_ham/HamUI.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "ui/UIPanel.h"

DepthBuffer::DepthBuffer() : unk2c(0), mState(kDepthBuffer_Normal), unk34(0) {}

BEGIN_HANDLERS(DepthBuffer)
    HANDLE_ACTION(set_state, SetState(_msg->Int(2)))
    HANDLE_SUPERCLASS(Hmx::Object)
END_HANDLERS

void DepthBuffer::Init(UIPanel *panel) {
    SetName("depth_buffer", ObjectDir::Main());
    unk2c = panel;
}
