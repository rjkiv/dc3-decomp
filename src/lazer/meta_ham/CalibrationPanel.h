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
    void Cleanup() { mOffsets.clear(); }

private:
    std::vector<int> mOffsets; // 0x30
    UIPanel *mCalibrationPanel; // 0x3c
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

    CalibrationOffsetProvider mOffsetProvider; // 0x3c
    float mCycleTime; // 0x7c
    float mVolDb; // 0x80
    Stream *mStream; // 0x84
    bool mStartASAP; // 0x88
};
