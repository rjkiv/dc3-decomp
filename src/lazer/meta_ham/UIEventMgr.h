#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "obj/Msg.h"
#include "utl/Str.h"

class UIEventMgr : public Hmx::Object {
public:
    enum EventType {
    };
    class BandEvent {
    public:
        BandEvent(EventType, DataArray *, DataArray *);
        void CacheDestination();

        EventType mType; // 0x0
        DataArray *unk4; // 0x4
        DataArrayPtr unk8; // 0x8
        String mDestScreen; // 0xc
        bool mActive; // 0x14
    };
    UIEventMgr();
    virtual ~UIEventMgr();
    virtual DataNode Handle(DataArray *, bool);

    void DismissEvent(Symbol);
    bool HasActiveTransitionEvent() const;
    bool HasActiveDialogEvent() const;
    Symbol CurrentEvent() const;
    bool IsTransitionEventStarted() const;
    bool IsTransitionEventFinished() const;
    void TriggerEvent(Symbol, DataArray *);

    static void Init();
    static void Terminate();

private:
    void ActivateFirstEvent();

    DataNode OnTriggerEvent(DataArray *);

    std::vector<BandEvent *> mEventQueue; // 0x2c
};

extern UIEventMgr *TheUIEventMgr;
