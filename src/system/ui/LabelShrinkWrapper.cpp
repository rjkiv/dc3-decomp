#include "ui/LabelShrinkWrapper.h"
#include "UIComponent.h"
#include "macros.h"
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

BEGIN_PROPSYNCS(LabelShrinkWrapper)
    SYNC_PROP_MODIFY(resource, mResourceDir, Update())
    SYNC_PROP_SET(
        label, m_pLabel.Ptr(), m_pLabel = dynamic_cast<UILabel *>(_val.GetObj())
    ) // somethings wrong with this line for some reason
    SYNC_PROP_SET(show, m_pShow, m_pShow = _val.Int())
    SYNC_PROP(left_border, mLeftBorder)
    SYNC_PROP(right_border, mRightBorder)
    SYNC_PROP(top_border, mTopBorder)
    SYNC_PROP(bottom_border, mBottomBorder)
    SYNC_SUPERCLASS(UIComponent)
END_PROPSYNCS

BEGIN_SAVES(LabelShrinkWrapper)
    SAVE_REVS(2, 0)
    bs << m_pLabel;
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

void LabelShrinkWrapper::PreLoad(BinStream &bs) {
    LOAD_REVS(bs)
    ASSERT_REVS(2, 0)
    bs >> m_pLabel;
    bs >> m_pShow;
    if (d.rev >= 1)
        bs >> mResourceDir;
    if (2 <= d.rev) {
        bs >> mLeftBorder;
        bs >> mRightBorder;
        bs >> mTopBorder;
        bs >> mBottomBorder;
    }
    UIComponent::PreLoad(bs);
    bs.PushRev(packRevs(d.altRev, d.rev), this);
}

void LabelShrinkWrapper::PostLoad(BinStream &bs) {
    bs.PopRev(this);
    // mResourceDir->PostLoad(bs);  fix this line later ig
    UIComponent::PostLoad(bs);
    Update();
}

void LabelShrinkWrapper::Enter() { UIComponent::Enter(); }

void LabelShrinkWrapper::SetTypeDef(DataArray *d) {
    Hmx::Object::SetTypeDef(d);
    Update();
}

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

void LabelShrinkWrapper::DrawShowing() {
    if (m_pLabel && m_pShow) {
        MILO_ASSERT(mResourceDir, 0xa7);
        UpdateAndDrawWrapper();
    }
}

BEGIN_HANDLERS(LabelShrinkWrapper)
    HANDLE_SUPERCLASS(UIComponent)
END_HANDLERS
