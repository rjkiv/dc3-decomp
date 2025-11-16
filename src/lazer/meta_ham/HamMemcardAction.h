#pragma once

#include "meta/MemcardAction.h"

class SaveMemcardAction : public MemcardAction {
public:
    SaveMemcardAction(Profile *);
    virtual void PreAction();
};

class LoadMemcardAction : public MemcardAction {
public:
    LoadMemcardAction(Profile *);
    virtual void PostAction();
};
