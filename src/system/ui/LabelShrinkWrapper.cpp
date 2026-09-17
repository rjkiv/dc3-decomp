#include "ui/LabelShrinkWrapper.h"
#include "UIComponent.h"
#include "macros.h"
#include "math/Geo.h"
#include "obj/Data.h"
#include "obj/Dir.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/Mesh.h"
#include "ui/UILabel.h"
#include "ui/UIPanel.h"
#include "utl/BinStream.h"
#include "utl/Loader.h"
#include "utl/Symbol.h"

LabelShrinkWrapper::LabelShrinkWrapper()
    : mResourceDir(this), m_pLabel(this), m_pShow(0), mLeftBorder(0), mRightBorder(0),
      mTopBorder(0), mBottomBorder(0), m_pTopLeftBone(0), m_pTopRightBone(0),
      m_pBottomLeftBone(0), m_pBottomRightBone(0) {}

LabelShrinkWrapper::~LabelShrinkWrapper() {}

BEGIN_HANDLERS(LabelShrinkWrapper)
    HANDLE_SUPERCLASS(UIComponent)
END_HANDLERS

BEGIN_PROPSYNCS(LabelShrinkWrapper)
    SYNC_PROP_MODIFY(resource, mResourceDir, Update())
    SYNC_PROP_SET(label, Label(), m_pLabel = _val.Obj<UILabel>())
    SYNC_PROP_SET(show, m_pShow, m_pShow = _val.Int())
    SYNC_PROP_MODIFY(left_border, mLeftBorder, Update())
    SYNC_PROP_MODIFY(right_border, mRightBorder, Update())
    SYNC_PROP_MODIFY(top_border, mTopBorder, Update())
    SYNC_PROP_MODIFY(bottom_border, mBottomBorder, Update())
    SYNC_SUPERCLASS(UIComponent)
END_PROPSYNCS

BEGIN_SAVES(LabelShrinkWrapper)
    SAVE_REVS(2, 0)
    bs << m_pLabel << m_pShow;
    bs << mResourceDir;
    bs << mLeftBorder;
    bs << mRightBorder;
    bs << mTopBorder;
    bs << mBottomBorder;
    SAVE_SUPERCLASS(UIComponent)
END_SAVES

BEGIN_COPYS(LabelShrinkWrapper)
    COPY_SUPERCLASS(UIComponent)
    CREATE_COPY_AS(LabelShrinkWrapper, c)
    BEGIN_COPYING_MEMBERS_FROM(c)
        COPY_MEMBER(m_pLabel)
        COPY_MEMBER(m_pShow)
        COPY_MEMBER(mLeftBorder)
        COPY_MEMBER(mRightBorder)
        COPY_MEMBER(mTopBorder)
        COPY_MEMBER(mBottomBorder)
        COPY_MEMBER(mResourceDir)
    END_COPYING_MEMBERS
    Update();
END_COPYS

BEGIN_LOADS(LabelShrinkWrapper)
    PreLoad(bs);
    PostLoad(bs);
END_LOADS

void LabelShrinkWrapper::SetTypeDef(DataArray *d) {
    Hmx::Object::SetTypeDef(d);
    Update();
}

INIT_REVS(2, 0)

void LabelShrinkWrapper::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    d.stream >> m_pLabel;
    d.stream >> m_pShow;
    if (d.rev >= 1)
        d >> mResourceDir;
    if (d.rev >= 2) {
        d >> mLeftBorder;
        d >> mRightBorder;
        d >> mTopBorder;
        d >> mBottomBorder;
    }
    UIComponent::PreLoad(d.stream);
    d.PushRev(this);
}

void LabelShrinkWrapper::PostLoad(BinStream &bs) {
    bs.PopRev(this);
    mResourceDir.PostLoad(nullptr);
    UIComponent::PostLoad(bs);
    Update();
}

void LabelShrinkWrapper::DrawShowing() {
    if (m_pLabel && m_pShow) {
        MILO_ASSERT(mResourceDir, 0xa7);
        UpdateAndDrawWrapper();
        mResourceDir->SetWorldXfm(WorldXfm());
        mResourceDir->Draw();
    }
}

void LabelShrinkWrapper::Enter() { UIComponent::Enter(); }

void LabelShrinkWrapper::Update() {
    const DataArray *pTypeDef = TypeDef();
    if (pTypeDef && mResourceDir) {
        static Symbol topleft_bone("topleft_bone");
        static Symbol topright_bone("topright_bone");
        static Symbol bottomleft_bone("bottomleft_bone");
        static Symbol bottomright_bone("bottomright_bone");
        m_pTopLeftBone =
            mResourceDir->Find<RndMesh>(pTypeDef->FindStr(topleft_bone), true);
        MILO_ASSERT(m_pTopLeftBone, 0xc5);
        m_pTopRightBone =
            mResourceDir->Find<RndMesh>(pTypeDef->FindStr(topright_bone), true);
        MILO_ASSERT(m_pTopRightBone, 0xc7);
        m_pBottomLeftBone =
            mResourceDir->Find<RndMesh>(pTypeDef->FindStr(bottomleft_bone), true);
        MILO_ASSERT(m_pBottomLeftBone, 0xc9);
        m_pBottomRightBone =
            mResourceDir->Find<RndMesh>(pTypeDef->FindStr(bottomright_bone), true);
        MILO_ASSERT(m_pBottomRightBone, 0xcb);
    } else {
        m_pBottomRightBone = nullptr;
        m_pBottomLeftBone = nullptr;
        m_pTopRightBone = nullptr;
        m_pTopLeftBone = nullptr;
    }
}

void LabelShrinkWrapper::Init() { REGISTER_OBJ_FACTORY(LabelShrinkWrapper) }

void LabelShrinkWrapper::UpdateAndDrawWrapper() {
    MILO_ASSERT(m_pLabel, 0x86);
    const Hmx::Rect &r = m_pLabel->DrawRect();
    float left = r.x - mLeftBorder;
    float bottom = r.y - mBottomBorder;
    float right = mRightBorder + r.w + r.x;
    float top = mTopBorder + r.h + r.y;
    SetWorldXfm(m_pLabel->WorldXfm());
    Vector3 v1(left, 0, top);
    Vector3 v2(right, 0, top);
    Vector3 v3(left, 0, bottom);
    Vector3 v4(right, 0, bottom);
    m_pTopLeftBone->SetLocalPos(v1);
    m_pTopRightBone->SetLocalPos(v2);
    m_pBottomLeftBone->SetLocalPos(v3);
    m_pBottomRightBone->SetLocalPos(v4);
}
