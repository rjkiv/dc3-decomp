#pragma once
#include "rndobj/Cam.h"
#include "rndobj/Dir.h"
#include "ui/UIComponent.h"
#include "utl/MemMgr.h"

class Flow;
class UITrigger;

class PanelDir : public RndDir {
public:
    enum RequestFocus {
        kNoFocus = 0,
        kMaybeFocus = 1,
        kAlwaysFocus = 2,
    };
    PanelDir();
    // Hmx::Object
    virtual ~PanelDir();
    OBJ_CLASSNAME(PanelDir)
    OBJ_SET_TYPE(PanelDir)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // ObjectDir
    virtual void SyncObjects();
    virtual void RemovingObject(Hmx::Object *);
    virtual bool Entering() const;
    virtual bool Exiting() const;
    virtual UIComponent *FocusComponent();
    virtual UIComponent *FindComponent(const char *);
    virtual void SetFocusComponent(UIComponent *, Symbol);
    // RndDrawable
    virtual RndCam *CamOverride();
    virtual void DrawShowing();
    // RndPollable
    virtual void Enter();
    virtual void Exit();

    OBJ_MEM_OVERLOAD(0x19);

    void SetOwnerPanel(UIPanel *panel) { mOwnerPanel = panel; }
    void EnableComponent(UIComponent *, PanelDir::RequestFocus);
    void DisableComponent(UIComponent *, JoypadAction);
    DataNode GetFocusableComponentList();
    void SetShowFocusComponent(bool show);
    void UpdateFocusComponentState();

private:
    UIComponent *GetFirstFocusableComponent();
    UIComponent *ComponentNav(UIComponent *, JoypadAction, JoypadButton, Symbol);
    DataNode OnEnableComponent(DataArray const *);
    bool PanelNav(JoypadAction, JoypadButton, Symbol);
    DataNode OnMsg(ButtonDownMsg const &);
    DataNode OnDisableComponent(DataArray const *);
    void SyncEditModePanels();
    bool
    PropSyncEditModePanels(std::vector<FilePath> &, DataNode &, DataArray *, int, PropOp);

    static bool sAlwaysNeedFocus;

protected:
    void SendTransition(Message const &, Symbol, Symbol);

    /** The currently focused-on component. */
    UIComponent *mFocusComponent; // 0x1fc
    class UIPanel *mOwnerPanel; // 0x200
    /** "Camera to use in game, else standard UI cam" */
    ObjPtr<RndCam> mCam; // 0x204
    /** The list of UITriggers within this PanelDir. */
    std::list<UITrigger *> mTriggers; // 0x218
    std::list<Flow *> mFlows; // 0x220
    /** The list of UIComponents within this PanelDir. */
    std::list<UIComponent *> mComponents; // 0x228
    /** "Trigger postprocs before drawing this panel.
     * If checked, this panel will not be affected by the postprocs." */
    bool mCanEndWorld; // 0x230
    /** "Forces the usage of the 'cam' property to render in milo. This is a milo only
     * feature." */
    bool mUseSpecifiedCam; // 0x231
    /** "Additional panels to display behind this panel." */
    std::vector<RndDir *> mBackPanels; // 0x234
    /** The file paths of the aforementioned back panels. */
    std::vector<FilePath> mBackFilenames; // 0x240
    /** "Additional panels to display in front of this panel." */
    std::vector<RndDir *> mFrontPanels; // 0x24c
    /** The file paths of the aforementioned front panels. */
    std::vector<FilePath> mFrontFilenames; // 0x258
    /** "Whether or no this panel displays its view only panels" */
    bool mShowEditModePanels; // 0x264
    /** Whether or not to show the currently focused component. */
    bool mShowFocusComponent; // 0x265
};
