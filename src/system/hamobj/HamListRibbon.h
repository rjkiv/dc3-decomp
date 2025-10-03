#pragma once
#include "flow/Flow.h"
#include "hamobj/HamLabel.h"
#include "math/DoubleExponentialSmoother.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "rndobj/Anim.h"
#include "rndobj/Dir.h"
#include "synth/Sound.h"
#include "utl/BinStream.h"
#include "utl/MemMgr.h"

struct HamListRibbonDrawState {
    DoubleExponentialSmoother unk0;
    bool unk14;
    int unk18;
    bool unk1c;
    float unk20;
    bool unk24;
};

/** "Top-level resource object for UILists" */
class HamListRibbon : public RndDir {
public:
    enum RibbonMode {
        kRibbonSwell = 0,
        kRibbonSlide = 1,
        kRibbonSelect = 2,
        kRibbonDisengaged = 3
    };
    class ScrollAnims {
    public:
        ScrollAnims(Hmx::Object *owner)
            : mScrollAnim(owner), mScrollActive(owner), mScrollFade(owner),
              mScrollFaded(owner) {}

        void SetScrollFrame(float);
        void SetAnims(int);
        void Save(BinStream &) const;
        void Load(BinStreamRev &);

        ObjPtr<RndAnimatable> mScrollAnim; // 0x0
        ObjPtr<RndAnimatable> mScrollActive; // 0x14
        ObjPtr<RndAnimatable> mScrollFade; // 0x28
        ObjPtr<RndAnimatable> mScrollFaded; // 0x3c
    };
    HamListRibbon();
    // Hmx::Object
    virtual ~HamListRibbon() {}
    OBJ_CLASSNAME(HamListRibbon);
    OBJ_SET_TYPE(HamListRibbon);
    virtual DataNode Handle(DataArray *, bool);
    virtual bool SyncProperty(DataNode &, DataArray *, int, PropOp);
    virtual void Save(BinStream &);
    virtual void Copy(const Hmx::Object *, Hmx::Object::CopyType);
    virtual void PreLoad(BinStream &);
    virtual void PostLoad(BinStream &);
    // ObjectDir
    virtual void SyncObjects() { RndDir::SyncObjects(); }
    // RndDrawable
    virtual void DrawShowing();
    // RndAnimatable
    virtual void SetFrame(float frame, float blend) {
        RndAnimatable::SetFrame(frame, blend);
    }
    virtual float StartFrame();
    virtual float EndFrame();

    OBJ_MEM_OVERLOAD(0x2E)
    NEW_OBJ(HamListRibbon)
    static const int sNumListSelectable;

    void HandleEnter();
    void OnSelectDone();
    void PlayHighlightSound(int);
    void PlaySelectSound(int);
    Sound *SlideSound() const { return mSlideSound; }

private:
    void ResetAnims(bool);

    DataNode OnEnterBlacklightMode(const DataArray *);
    DataNode OnExitBlacklightMode(const DataArray *);

protected:
    ScrollAnims mScrollAnims; // 0x1fc
    /** "(Milo only) Draw as a test list?" */
    bool mTestMode; // 0x24c
    /** "(Milo only) If test_mode is on, how many to draw" */
    int mTestNumDisplay; // 0x250
    /** "(Milo only) If test_mode is on, which element is highlighted" */
    int mTestSelectedIndex; // 0x254
    /** "How far apart elements should be spaced" */
    float mSpacing; // 0x258
    /** "Mode for animations" */
    RibbonMode mMode; // 0x25c
    /** "(Milo only) Test enter anim?" */
    bool mTestEntering; // 0x260
    /** "Minimum number of ribbons to show" */
    int mPaddedSize; // 0x264
    /** "Spacing between padded ribbons" */
    float mPaddedSpacing; // 0x268
    bool unk26c; // 0x26c
    ObjPtr<RndAnimatable> mSwellAnim; // 0x270
    ObjPtr<RndAnimatable> mSlideAnim; // 0x284
    ObjPtr<RndAnimatable> mSelectAnim; // 0x298
    ObjPtr<RndAnimatable> mSelectToggleAnim; // 0x2ac
    ObjPtr<RndAnimatable> mSelectInactiveAnim; // 0x2c0
    ObjPtr<RndAnimatable> mSelectAllAnim; // 0x2d4
    ObjPtr<RndAnimatable> mDisengageAnim; // 0x2e8
    ObjPtr<RndAnimatable> mEnterAnim; // 0x2fc
    /** "Where the label goes" */
    ObjPtr<HamLabel> mLabelPlaceholder; // 0x310
    ObjPtrVec<Flow> mHighlightSounds; // 0x324
    ObjPtrVec<Flow> mSelectSounds; // 0x340
    /** "Flow to play on enter" */
    ObjPtr<Flow> mEnterFlow; // 0x35c
    ObjPtr<Sound> mSlideSound; // 0x370
    ObjPtr<RndAnimatable> mSlideSoundAnim; // 0x384
    ObjPtr<Sound> mScrollSound; // 0x398
    ObjPtr<RndAnimatable> mScrollSoundAnim; // 0x3ac
};
