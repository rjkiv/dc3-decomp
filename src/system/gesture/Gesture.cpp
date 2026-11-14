#include "gesture/Gesture.h"
#include "Skeleton.h"
#include "StreamRecorder.h"
#include "gesture/CameraTilt.h"
#include "gesture/DepthBuffer3D.h"
#include "gesture/FitnessFilter.h"
#include "gesture/GestureMgr.h"
#include "gesture/HandRaisedGestureFilter.h"
#include "gesture/HandsUpGestureFilter.h"
#include "gesture/HighFiveGestureFilter.h"
#include "gesture/NavigationSkeletonDir.h"
#include "gesture/SkeletonClip.h"
#include "gesture/SkeletonDir.h"
#include "gesture/SkeletonViz.h"
#include "gesture/StreamRenderer.h"
#include "gesture/WaveToTurnOnLight.h"
#include "obj/Object.h"

void GestureTerminate() { StreamRenderer::Terminate(); }

void GestureInit() {
    DepthBuffer3D::Init();
    FitnessFilterObj::Register();
    HandRaisedGestureFilter::Register();
    HandsUpGestureFilter::Register();
    HighFiveGestureFilter::Register();
    StreamRenderer::Init();
    StreamRecorder::Register();
    SkeletonClip::Init();
    SkeletonDir::Register();
    SkeletonFrame::Init();
    SkeletonViz::Register();
    NavigationSkeletonDir::Register();
    GestureMgr::Init();
    CameraTilt::Init();
    WaveToTurnOnLight::Init();
}
