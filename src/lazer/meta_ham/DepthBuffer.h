#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIPanel.h"

enum DepthBufferState {
    kDepthBuffer_Normal = 0,
    kDepthBuffer_Tutorial = 1,
    kDepthBufferGameplay_Perform = 10,
    kDepthBufferGameplay_Practice_Learn = 11,
    kDepthBufferGameplay_Practice_Review = 12,
};

class DepthBuffer : public Hmx::Object {
public:
    DepthBuffer();
    virtual ~DepthBuffer() {}
    virtual DataNode Handle(DataArray *, bool);

    void Init(UIPanel *);
    void Poll();

    void SetState(DepthBufferState state) {
        if (state != mState) {
            mState = state;
        }
    }

private:
    UIPanel *mCursorPanel; // 0x2c
    DepthBufferState mState; // 0x30
    bool mWantGameplayStateTransition; // 0x34
};
