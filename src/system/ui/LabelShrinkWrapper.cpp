#include "ui/LabelShrinkWrapper.h"
#include "UIComponent.h"
#include "macros.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "utl/BinStream.h"

LabelShrinkWrapper::LabelShrinkWrapper() : m_pLabel(this) {}

LabelShrinkWrapper::~LabelShrinkWrapper() {}

BEGIN_PROPSYNCS(LabelShrinkWrapper)
END_PROPSYNCS

BEGIN_SAVES(LabelShrinkWrapper)
END_SAVES

BEGIN_COPYS(LabelShrinkWrapper)
END_COPYS

void LabelShrinkWrapper::SetTypeDef(DataArray *) {}

void LabelShrinkWrapper::PreLoad(BinStream &) {}

void LabelShrinkWrapper::PostLoad(BinStream &) {}

void LabelShrinkWrapper::DrawShowing() {}

void LabelShrinkWrapper::Poll() {}

void LabelShrinkWrapper::Enter() {}

void LabelShrinkWrapper::OldResourcePreload(BinStream &) {}

BEGIN_HANDLERS(LabelShrinkWrapper)
    HANDLE_SUPERCLASS(UIComponent)
END_HANDLERS
