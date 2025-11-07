#include "ui/PanelDir.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Cam.h"
#include "rndobj/Dir.h"
#include "ui/UI.h"
#include "ui/UIComponent.h"
#include "ui/UIPanel.h"
#include "ui/UITrigger.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/Std.h"
#include "utl/Symbol.h"

bool gSendFocusMsg;

void PanelDir::SyncObjects() {
    RndDir::SyncObjects();
    mComponents.clear();
    for (ObjDirItr<UIComponent> it(this, true); it != nullptr; ++it) {
        mComponents.push_back(it);
    }
    mTriggers.clear();
    for (ObjDirItr<UITrigger> it(this, true); it != nullptr; ++it) {
        mTriggers.push_back(it);
        it->CheckAnims();
    }
    if (sAlwaysNeedFocus) {
        UIComponent *comp = GetFirstFocusableComponent();
        if (!mFocusComponent && comp) {
            gSendFocusMsg = false;
            SetFocusComponent(comp, gNullStr);
            gSendFocusMsg = true;
        }
    }
}

PanelDir::~PanelDir() {
    for (std::vector<RndDir *>::iterator it = mBackPanels.begin();
         it != mBackPanels.end();
         ++it) {
        RELEASE(*it);
    }
    for (std::vector<RndDir *>::iterator it = mFrontPanels.begin();
         it != mFrontPanels.end();
         ++it) {
        RELEASE(*it);
    }
}

PanelDir::PanelDir()
    : mFocusComponent(nullptr), mOwnerPanel(nullptr), mCam(this), mCanEndWorld(true),
      mUseSpecifiedCam(false), mShowEditModePanels(false), mShowFocusComponent(true) {
    if (TheLoadMgr.EditMode()) {
        mShowEditModePanels = true;
    }
}

BEGIN_PROPSYNCS(PanelDir)
    SYNC_PROP(cam, mCam)
    SYNC_PROP(postprocs_before_draw, mCanEndWorld)
    SYNC_PROP(use_specified_cam, mUseSpecifiedCam)
    SYNC_PROP(focus_component, mFocusComponent)
    SYNC_PROP(owner_panel, mOwnerPanel) {
        static Symbol _s("front_view_only_panels");
        if (sym == _s) {
            PropSyncEditModePanels(mFrontFilenames, _val, _prop, _i + 1, _op);
            return true;
        }
    }
    {
        static Symbol _s("back_view_only_panels");
        if (sym == _s) {
            PropSyncEditModePanels(mBackFilenames, _val, _prop, _i + 1, _op);
            return true;
        }
    }
    SYNC_PROP_MODIFY(show_view_only_panels, mShowEditModePanels, SyncEditModePanels())
    SYNC_SUPERCLASS(RndDir)
END_PROPSYNCS

BEGIN_SAVES(PanelDir)
END_SAVES

BEGIN_COPYS(PanelDir)
    COPY_SUPERCLASS(RndDir)
    CREATE_COPY(PanelDir)
    BEGIN_COPYING_MEMBERS
        COPY_MEMBER(mCam)
        COPY_MEMBER(mCanEndWorld)
        COPY_MEMBER(mBackFilenames)
        COPY_MEMBER(mFrontFilenames)
        COPY_MEMBER(mShowEditModePanels)
        COPY_MEMBER(mUseSpecifiedCam)
        SyncEditModePanels();
    END_COPYING_MEMBERS
END_COPYS

void PanelDir::PreLoad(BinStream &) {}

void PanelDir::PostLoad(BinStream &bs) {}

void PanelDir::RemovingObject(Hmx::Object *o) {}

bool PanelDir::Entering() const {
    FOREACH (it, mComponents) {
        if ((*it)->Entering())
            return true;
    }
    FOREACH (it, mTriggers) {
        if ((*it)->IsBlocking())
            return true;
    }
    return false;
}

bool PanelDir::Exiting() const {
    FOREACH (it, mComponents) {
        if ((*it)->Exiting())
            return true;
    }
    FOREACH (it, mTriggers) {
        if ((*it)->IsBlocking())
            return true;
    }
    return false;
}

UIComponent *PanelDir::FocusComponent() { return mFocusComponent; }

UIComponent *PanelDir::FindComponent(const char *name) {
    return Find<UIComponent>(name, false);
}

void PanelDir::SetFocusComponent(UIComponent *, Symbol) {}

RndCam *PanelDir::CamOverride() {
    if (TheLoadMgr.EditMode() && !mUseSpecifiedCam)
        return nullptr;
    if (mCam)
        return mCam;
    return TheUI->GetCam();
}

void PanelDir::DrawShowing() {}

void PanelDir::Enter() {
    RndDir::Enter();
    FOREACH (it, mTriggers) {
        (*it)->Enter();
    }

    static Message ui_enter("ui_enter");
    static Symbol ui_enter_forward("ui_enter_forward");
    static Symbol ui_enter_back("ui_enter_back");
    SendTransition(ui_enter, ui_enter_forward, ui_enter_back);
}

void PanelDir::Exit() {
    RndDir::Exit();

    static Message ui_exit("ui_exit");
    static Symbol ui_exit_forward("ui_exit_forward");
    static Symbol ui_exit_back("ui_exit_back");
    SendTransition(ui_exit, ui_exit_forward, ui_exit_back);
}

UIComponent *PanelDir::GetFirstFocusableComponent() {
    UIComponent *ret = nullptr;
    FOREACH (it, mComponents) {
        UIComponent *component = *it;
        MILO_ASSERT(component, 0x214);
        if (component->CanHaveFocus()) {
            ret = component;
            break;
        }
    }
    return ret;
}

UIComponent *PanelDir::ComponentNav(
    UIComponent *comp, JoypadAction act, JoypadButton btn, Symbol controller_type
) {
    return nullptr;
}

void PanelDir::EnableComponent(UIComponent *c, PanelDir::RequestFocus focusable) {
    if (c->GetState() == UIComponent::kDisabled)
        c->SetState(UIComponent::kNormal);
    if (c->CanHaveFocus()
        && (focusable == kAlwaysFocus
            || (focusable == kMaybeFocus && !mFocusComponent))) {
        SetFocusComponent(c, gNullStr);
    }
}

DataNode PanelDir::OnEnableComponent(DataArray const *da) {
    UIComponent *c = da->Obj<UIComponent>(2);
    if (da->Size() == 4) {
        EnableComponent(c, (RequestFocus)da->Int(3));
    } else if (da->Size() == 3) {
        EnableComponent(c, kNoFocus);
    } else
        MILO_NOTIFY("wrong number of args to PanelDir enable");
    return 0;
}

void PanelDir::SendTransition(Message const &msg, Symbol forward, Symbol back) {
    static Message dirMsg = Message("");
    dirMsg.SetType(TheUI->WentBack() ? back : forward);
    RndDir::Handle(msg, false);
    RndDir::Handle(dirMsg, false);
}

bool PanelDir::PanelNav(JoypadAction act, JoypadButton btn, Symbol controller_type) {
    return false;
}

DataNode PanelDir::OnMsg(ButtonDownMsg const &msg) {
    DataNode node(kDataUnhandled, 0);
    if (mFocusComponent) {
        node = mFocusComponent->Handle(msg, false);
    }
    if (node.Type() == kDataUnhandled) {
        if (PanelNav(
                msg.GetAction(),
                msg.GetButton(),
                JoypadControllerTypePadNum(msg.GetPadNum())
            )) {
            return 0;
        }
    }
    return node;
}

void PanelDir::DisableComponent(UIComponent *c, JoypadAction nav_action) {}

DataNode PanelDir::OnDisableComponent(DataArray const *) { return NULL_OBJ; }

DataNode PanelDir::GetFocusableComponentList() {
    std::vector<UIComponent *> components;
    FOREACH (it, mComponents) {
        UIComponent *component = *it;
        MILO_ASSERT(component, 0x1f4);
        if (component->CanHaveFocus()) {
            components.push_back(component);
        }
    }
    DataArrayPtr ptr(new DataArray(components.size()));
    int i = 0;
    FOREACH (it, components) {
        ptr->Node(i) = *it;
    }
    return ptr;
}

void PanelDir::SyncEditModePanels() {
    if (TheLoadMgr.EditMode()) {
        FOREACH (it, mBackPanels) {
            RELEASE(*it);
        }
        FOREACH (it, mFrontPanels) {
            RELEASE(*it);
        }
        if (mShowEditModePanels) {
            FOREACH (it, mBackFilenames) {
                FilePath fp3c(*it);
                if (fp3c.length() != 0) {
                    RndDir *curDir =
                        dynamic_cast<RndDir *>(DirLoader::LoadObjects(fp3c, 0, 0));
                    if (curDir) {
                        mBackPanels.push_back(curDir);
                        curDir->Enter();
                    }
                }
            }
            FOREACH (it, mFrontFilenames) {
                FilePath fp48(*it);
                if (fp48.length() != 0) {
                    RndDir *curDir =
                        dynamic_cast<RndDir *>(DirLoader::LoadObjects(fp48, 0, 0));
                    if (curDir) {
                        mFrontPanels.push_back(curDir);
                        curDir->Enter();
                    }
                }
            }
        }
    }
}

bool PanelDir::PropSyncEditModePanels(
    std::vector<FilePath> &paths, DataNode &val, DataArray *prop, int i, PropOp op
) {
    if (op == kPropSize) {
        MILO_ASSERT(i == prop->Size(), 0x2c6);
        val = (int)paths.size();
        return true;
    } else {
        MILO_ASSERT(i == prop->Size() - 1, 0x2cb);
        std::vector<FilePath>::iterator it = paths.begin() + prop->Int(i);
        switch (op) {
        case kPropGet:
            val = *it;
            break;
        case kPropSet:
            it->SetRoot(val.Str());
            SyncEditModePanels();
            break;
        case kPropRemove:
            paths.erase(it);
            SyncEditModePanels();
            break;
        case kPropInsert:
            paths.insert(it, val.Str());
            SyncEditModePanels();
            break;
        default:
            return false;
        }
        return true;
    }
}

void PanelDir::SetShowFocusComponent(bool b) {
    mShowFocusComponent = b;
    UpdateFocusComponentState();
}

void PanelDir::UpdateFocusComponentState() {
    if (!mFocusComponent)
        return;
    if (mShowFocusComponent)
        mFocusComponent->SetState(UIComponent::kFocused);
    else
        mFocusComponent->SetState(UIComponent::kNormal);
}

BEGIN_HANDLERS(PanelDir)
    HANDLE(enable, OnEnableComponent)
    HANDLE(disable, OnDisableComponent)
    HANDLE_ACTION(set_focus, SetFocusComponent(_msg->Obj<UIComponent>(2), gNullStr))
    HANDLE_EXPR(focus_name, mFocusComponent ? mFocusComponent->Name() : "")
    HANDLE_EXPR(get_focusable_components, GetFocusableComponentList())
    HANDLE_ACTION(set_show_focus_component, SetShowFocusComponent(_msg->Int(2)))
    HANDLE_SUPERCLASS(RndDir)
    HANDLE_MESSAGE(ButtonDownMsg)
    if (sym != "button_down")
        HANDLE_MEMBER_PTR(mFocusComponent)
END_HANDLERS
