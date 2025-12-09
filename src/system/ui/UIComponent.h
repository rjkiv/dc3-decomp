#pragma once
#include "obj/Object.h"
#include "os/User.h"
#include "rndobj/Draw.h"
#include "rndobj/Trans.h"
#include "rndobj/Poll.h"
#include "utl/MemMgr.h"
#include "ui/UIScreen.h"

/**
 * @brief A base implementation of a UI object.
 * Original _objects description:
 * "Base class of all UI components,
 * defines navigation and component state"
 */
class UIComponent : public RndPollable,
                    public virtual RndDrawable,
                    public virtual RndTransformable {
public:
    enum State {
        kNormal = 0,
        kFocused = 1,
        kDisabled = 2,
        kSelecting = 3,
        kSelected = 4,
        kNumStates = 5,
    };
    // Hmx::Object
    virtual ~UIComponent() {}
    OBJ_CLASSNAME(UIComponent)
    OBJ_SET_TYPE(UIComponent)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void Load(BinStream &);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // RndDrawable
    virtual void Highlight() { RndDrawable::Highlight(); }
    // RndPollable
    virtual void Poll();
    virtual void Enter();
    virtual void Exit();
    virtual void SetState(UIComponent::State);
    virtual Symbol StateSym() const;
    virtual bool Entering() const { return false; }
    virtual bool Exiting() const { return mState == kSelecting; }
    virtual bool CanHaveFocus() { return true; }

    OBJ_MEM_OVERLOAD(0x19);
    NEW_OBJ(UIComponent)

    State GetState() { return mState; }

    static void Init();

protected:
    virtual void OldResourcePreload(BinStream &);
    UIComponent();
    void SendSelect(LocalUser *);

    static int sSelectFrames;

    State mState; // 0x8
    ObjPtr<UIComponent> mNavRight; // 0xc
    ObjPtr<UIComponent> mNavDown; // 0x20
    LocalUser *mSelectingUser; // 0x34
    UIScreen *mSelectScreen; // 0x38
    int mSelected; // 0x3c
    bool unk40; // 0x40

private:
    void FinishSelecting();
};

#include "obj/Msg.h"

// DECLARE_MESSAGE(UIComponentScrollMsg, "component_scroll");
// UIComponentScrollMsg(UIComponent *comp, LocalUser *user) : Message(Type(), comp, user)
// {} UIComponent *GetUIComponent() const { return mData->Obj<UIComponent>(2); } LocalUser
// *GetUser() const { return mData->Obj<LocalUser>(3); } END_MESSAGE

DECLARE_MESSAGE(UIComponentSelectMsg, "component_select");
UIComponentSelectMsg(UIComponent *comp, LocalUser *user) : Message(Type(), comp, user) {}
UIComponent *GetComponent() const { return mData->Obj<UIComponent>(2); }
LocalUser *GetUser() const { return mData->Obj<LocalUser>(3); }
END_MESSAGE

DECLARE_MESSAGE(UIComponentSelectDoneMsg, "component_select_done");
UIComponentSelectDoneMsg(UIComponent *comp, LocalUser *user)
    : Message(Type(), comp, user) {}
UIComponent *GetComponent() const { return mData->Obj<UIComponent>(2); }
LocalUser *GetUser() const { return mData->Obj<LocalUser>(3); }
END_MESSAGE

DECLARE_MESSAGE(UIComponentScrollSelectMsg, "component_scroll_select");
UIComponentScrollSelectMsg(UIComponent *comp, LocalUser *user, bool selected)
    : Message(Type(), comp, user, selected) {}
UIComponent *GetComponent() const { return mData->Obj<UIComponent>(2); }
LocalUser *GetUser() const { return mData->Obj<LocalUser>(3); }
bool GetSelected() const { return mData->Int(4); }
END_MESSAGE

// // #define FOCUS_MSG (component_focus ($new_focus $old_focus $panel_dir $nav_type))
// DECLARE_MESSAGE(UIComponentFocusChangeMsg, "component_focus");
// UIComponentFocusChangeMsg(UIComponent *comp1, UIComponent *comp2, PanelDir *dir, Symbol
// s)
//     : Message(Type(), comp1, comp2, dir, s) {}
// UIComponent *GetNewFocus() const { return mData->Obj<UIComponent>(2); }
// UIComponent *GetOldFocus() const { return mData->Obj<UIComponent>(3); }
// PanelDir *GetDir() const { return mData->Obj<PanelDir>(4); }
// Symbol GetNavType() const { return mData->Sym(5); }
// END_MESSAGE

// DECLARE_MESSAGE(UIComponentScrollStartMsg, "component_scroll_start");
// UIComponentScrollStartMsg(UIComponent *comp, LocalUser *user)
//     : Message(Type(), comp, user) {}
// UIComponent *GetComponent() const { return mData->Obj<UIComponent>(2); }
// LocalUser *GetUser() const { return mData->Obj<LocalUser>(3); }
// END_MESSAGE
