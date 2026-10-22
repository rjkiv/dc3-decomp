#pragma once
#include "obj/Object.h"
#include "rndobj/Dir.h"
#include "ui/UIListProvider.h"
#include "ui/UIListState.h"
#include "ui/UIListWidget.h"
#include "utl/MemMgr.h"

enum UIListOrientation {
    kUIListVertical,
    kUIListHorizontal
};

/** "Top-level resource object for UILists" */
class UIListDir : public RndDir, public UIListProvider, public UIListStateCallback {
public:
    UIListDir();
    // Hmx::Object
    virtual ~UIListDir();
    OBJ_CLASSNAME(UIListDir)
    OBJ_SET_TYPE(UIListDir)
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, CopyType);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // ObjectDir
    virtual void SyncObjects();
    // RndDrawable
    virtual void DrawShowing();
    // RndPollable
    virtual void Poll();
    // UIListProvider
    virtual int NumData() const;
    virtual float GapSize(int, int, int, int) const;
    virtual bool IsActive(int) const;
    // UIListStateCallback
    virtual void StartScroll(const UIListState &, int, bool);
    virtual void CompleteScroll(const UIListState &);

    UIListOrientation Orientation() const;
    //  public: void SetOrientation(UIListOrientation);
    //  public: int32_t FadeOffset() const;
    //  public: void SetFadeOffset(int32_t);
    float ElementSpacing() const;
    //  public: void SetElementSpacing(float);
    //  public: float ScrollHighlightChange() const;
    //  public: void SetScrollHighlightChange(float);
    //  public: bool TestMode() const;
    //  public: int32_t TestNumDisplay() const;
    //  public: float TestScrollTime() const;
    //  public: UIComponent::State TestListState() const;
    //  public: bool TestDisableElements() const;
    //  public: UIListState* TestState();
    UIList *SubList(int display, std::vector<UIListWidget *> &widgets);
    void DrawWidgets(
        UIListWidgetDrawState &ds,
        const UIListState &s,
        std::vector<UIListWidget *> &widgets,
        const Transform &root_xfm,
        UIComponent::State component_state,
        Box *bounding_box,
        bool drawManuallyControlledObjects
    );
    void PollWidgets(std::vector<UIListWidget *> &widgets);
    void
    FillElement(const UIListState &s, std::vector<UIListWidget *> &widgets, int display);
    void StartScroll(
        const UIListState &s,
        std::vector<UIListWidget *> &widgets,
        int direction,
        bool changing_data
    );
    void CompleteScroll(const UIListState &s, std::vector<UIListWidget *> &widgets);
    void FillElements(const UIListState &s, std::vector<UIListWidget *> &widgets);
    void ListEntered();
    void BuildDrawState(
        UIListWidgetDrawState &ds,
        const UIListState &s,
        UIComponent::State component_state,
        float highlight_offset,
        bool draw_highlight
    ) const;
    void
    CreateElements(UIList *parent, std::vector<UIListWidget *> &widgets, int num_elems);

    NEW_OBJ(UIListDir)
    OBJ_MEM_OVERLOAD(0x1b)

    //  public: void CalcBoundingBox(Box&);

private:
    float SetElementPos(
        Vector3 &pos, float display, int gridSpan, float offset, float offsetAcross
    ) const;
    void Reset();

    /** "scroll direction of list" */
    UIListOrientation mOrientation; // 0x204
    /** "Number of elements to fade from beginning/end of list". Ranges from 0 to 10. */
    int mFadeOffset; // 0x208
    /** "spacing between elements". Ranges from 0 to 1000. */
    float mElementSpacing; // 0x20c
    /** "point during scroll when highlight changes". Ranges from 0 to 1. */
    float mScrollHighlightChange; // 0x210
    /** "draw widgets in preview mode?" */
    bool mTestMode; // 0x214
    UIListState mTestState; // 0x218
    /** "total number of data elements" */
    int mTestNumData; // 0x260
    /** "test gaps between elements". Ranges from 0 to 1000. */
    float mTestGapSize; // 0x264
    UIComponent::State mTestComponentState; // 0x268
    /** "test disable every other element" */
    bool mTestDisableElements; // 0x26c
    std::vector<UIListWidget *> mEditWidgets; // 0x270
    int mDirection; // 0x27c
};
