#pragma once

class Profile;

class MemcardAction {
    MemcardAction(Profile *);
    virtual ~MemcardAction() {}

    int unk4;
    int unk8;
    int unkc;
    Profile *unk10;
};
