#pragma once
#include "obj/Data.h"
#include "obj/Object.h"
#include "ui/UIComponent.h"
#include "os/Joypad.h"

class LocalUser;

class ScrollSelect {
public:
    ScrollSelect();
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual ~ScrollSelect() {}
    virtual int SelectedAux() const = 0;
    virtual void SetSelectedAux(int) = 0;

    void Store();
    void Reset();
    bool SelectScrollSelect(UIComponent *, LocalUser *);
    bool RevertScrollSelect(UIComponent *, LocalUser *, Hmx::Object *);
    bool IsScrollSelected() const { return mSelectedAux != -1; }

private:
    DataNode SendScrollSelected(UIComponent *, LocalUser *);

protected:
    bool CanScroll() const;

    UIComponent::State DrawState(UIComponent *) const;
    bool CatchNavAction(JoypadAction) const;

    /** "Does [UI item] need to be selected before user can scroll?" */
    bool mSelectToScroll; // 0x4
    int mSelectedAux; // 0x8
};
