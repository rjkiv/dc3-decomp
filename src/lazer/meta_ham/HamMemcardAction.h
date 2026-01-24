#pragma once

#include "meta/MemcardAction.h"

class SaveMemcardAction : public MemcardAction {
public:
    SaveMemcardAction(Profile *);
    virtual void PreAction();
    virtual void PostAction();
};

class LoadMemcardAction : public MemcardAction {
public:
    LoadMemcardAction(Profile *);
    virtual void PreAction();
    virtual void PostAction();
};
