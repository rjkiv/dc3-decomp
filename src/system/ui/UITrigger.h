#pragma once
#include "obj/Object.h"
#include "rndobj/EventTrigger.h"
#include "rndobj/Poll.h"
#include "utl/MemMgr.h"

/** "Triggers anims based on UI events (enter, exit, etc.)" */
class UITrigger : public EventTrigger, public RndPollable {
public:
    // Hmx::Object
    OBJ_CLASSNAME(UITrigger);
    OBJ_SET_TYPE(UITrigger);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    // EventTrigger
    virtual void Trigger();
    virtual DataArray *SupportedEvents();
    virtual void CheckAnims();
    // RndPollable
    virtual void Poll();
    virtual void Enter();

    NEW_OBJ(UITrigger)
    OBJ_MEM_OVERLOAD(0x14)

    bool IsDone() const;
    bool IsBlocking() const;
    void StopAnimations();
    void PlayStartOfAnims();
    void PlayEndOfAnims();

protected:
    UITrigger();

    bool mBlockTransition; // 0x11c
    ObjPtr<Hmx::Object> mCallbackObject; // 0x120
    float mStartTime;
    float mEndTime; // 0x138
    bool unk13c; // 0x13c
};

#include "obj/Msg.h"

DECLARE_MESSAGE(UITriggerCompleteMsg, "ui_trigger_complete");
UITriggerCompleteMsg(UITrigger *trig) : Message(Type(), trig) {}
UITrigger *GetTrigger() const { return mData->Obj<UITrigger>(2); }
END_MESSAGE
