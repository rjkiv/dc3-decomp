#pragma once
#include "meta_ham/HamPanel.h"
#include "obj/Data.h"
#include "obj/Object.h"
#include "synth/Stream.h"
#include "ui/UIListProvider.h"
#include "ui/UIPanel.h"

class CalibrationOffsetProvider : public UIListProvider, public Hmx::Object {
public:
    CalibrationOffsetProvider(UIPanel *);
    // Hmx::Object
    virtual DataNode Handle(DataArray *, bool);
    // UIListProvider
    virtual void Text(int, int, UIListLabel *, UILabel *) const;
    virtual int NumData() const { return mOffsets.size(); }
    virtual void InitData(RndDir *);

    int GetOffset(int);
    void ClearOffsets() { mOffsets.clear(); }

private:
    std::vector<int> mOffsets; // 0x30
    UIPanel *unk3c;
};

class CalibrationPanel : public HamPanel {
public:
    CalibrationPanel();
    // Hmx::Object
    virtual ~CalibrationPanel();
    OBJ_CLASSNAME(CalibrationPanel)
    OBJ_SET_TYPE(CalibrationPanel)
    virtual DataNode Handle(DataArray *, bool);

    // UIPanel
    virtual void Enter();
    virtual void Exit();
    virtual void Poll();

    void StartAudio();
    void InitializeContent();

    NEW_OBJ(CalibrationPanel)

private:
    float GetAudioTimeMs() const;
    void UpdateStream();
    void UpdateAnimation();

    CalibrationOffsetProvider mProvider; // 0x3c
    float unk7c;
    float mVolume; // 0x80
    Stream *mStream; // 0x84
    bool unk88; // 0x88
};
