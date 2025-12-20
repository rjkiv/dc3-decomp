#pragma once
#include "meta_ham/HamPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "stl/_vector.h"
#include "synth/Stream.h"
#include "ui/UIListProvider.h"
#include "ui/UIPanel.h"

class CalibrationOffsetProvider : public UIListProvider, public Hmx::Object {
public:
    // Hmx::Object
    virtual DataNode Handle(DataArray *, bool);

    // UIListProvider
    virtual int NumData() const;
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual void InitData(RndDir *);

    CalibrationOffsetProvider(UIPanel *);
    int GetOffset(int);

    std::vector<int> mOffsets; // 0x30
    UIPanel *unk3c;
};

class CalibrationPanel : HamPanel {
public:
    // Hmx::Object
    virtual ~CalibrationPanel();
    OBJ_CLASSNAME(CalibrationPanel)
    OBJ_SET_TYPE(CalibrationPanel)
    virtual DataNode Handle(DataArray *, bool);

    // UIPanel
    virtual void Enter();
    virtual void Exit();
    virtual void Poll();

    CalibrationPanel();
    void StartAudio();
    void InitializeContent();

    CalibrationOffsetProvider unk3c;
    float unk7c;
    float mVolume; // 0x80
    Stream *mStream; // 0x84
    bool unk88; // 0x88

private:
    float GetAudioTimeMs() const;
    void UpdateStream();
    void UpdateAnimation();
};
