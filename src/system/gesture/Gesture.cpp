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
    REGISTER_OBJ_FACTORY(FitnessFilterObj)
    REGISTER_OBJ_FACTORY(HandRaisedGestureFilter)
    REGISTER_OBJ_FACTORY(HandsUpGestureFilter)
    REGISTER_OBJ_FACTORY(HighFiveGestureFilter)
    StreamRenderer::Init();
    REGISTER_OBJ_FACTORY(StreamRecorder)
    SkeletonClip::Init();
    REGISTER_OBJ_FACTORY(SkeletonDir)
    SkeletonFrame::Init();
    REGISTER_OBJ_FACTORY(SkeletonViz)
    REGISTER_OBJ_FACTORY(NavigationSkeletonDir)
    GestureMgr::Init();
    CameraTilt::Init();
    WaveToTurnOnLight::Init();
}
