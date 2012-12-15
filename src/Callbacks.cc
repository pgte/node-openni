#define BUILDING_NODE_EXTENSION

#ifndef NODE_OPENNI_CALLBACKS_H
#define NODE_OPENNI_CALLBACKS_H

#include "Context.h"

#define POSE_TO_USE "Psi"

namespace nodeopenni {

  void XN_CALLBACK_TYPE
  User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
  {
    printf("New User: %d\n", nId);
    Context * context = (Context *) pCookie;
    assert(context);
    context->AddUser(nId);
    context->UserEventAsync("newuser", nId);
    context->userGenerator_.GetPoseDetectionCap().StartPoseDetection(POSE_TO_USE, nId);
  }

  void XN_CALLBACK_TYPE
  User_LostUser(xn::UserGenerator& generator, XnUserID nId,
                void* pCookie)
  {
    Context * context = (Context *) pCookie;
    assert(context);
    context->RemoveUser(nId);
    printf("Lost user %d\n", nId);
    context->UserEventAsync("lostuser", nId);
  }
  
  void XN_CALLBACK_TYPE
  User_UserExit(xn::UserGenerator& generator, XnUserID nId,
                void* pCookie)
  {
    Context * context = (Context *) pCookie;
    assert(context);
    context->UserEventAsync("userexit", nId);
  }

  void XN_CALLBACK_TYPE
  Pose_Detected(xn::PoseDetectionCapability& pose, const XnChar* strPose,
                XnUserID nId, void* pCookie)
  {
    printf("Pose %s for user %d\n", strPose, nId);
    Context * context = (Context *) pCookie;
    assert(context);
    context->UserEventAsync("posedetected", nId);
    context->userGenerator_.GetPoseDetectionCap().StopPoseDetection(nId);
    context->userGenerator_.GetSkeletonCap().RequestCalibration(nId, TRUE);
  }

  void XN_CALLBACK_TYPE
  Calibration_Start(xn::SkeletonCapability& capability, XnUserID nId,
                    void* pCookie)
  {
    Context * context = (Context *) pCookie;
    assert(context);
    context->UserEventAsync("calibrationstart", nId);
    printf("Starting calibration for user %d\n", nId);
  }

  void XN_CALLBACK_TYPE
  Calibration_End(xn::SkeletonCapability& capability, XnUserID nId,
                  XnBool bSuccess, void* pCookie)
  {
    Context * context = (Context *) pCookie;
    assert(context);
    if (bSuccess)
    {
      context->UserEventAsync("calibrationsuccess", nId);
      printf("User calibrated\n");
      context->userGenerator_.GetSkeletonCap().StartTracking(nId);
    }
    else
    {
      printf("Failed to calibrate user %d\n", nId);
      context->UserEventAsync("calibrationfail", nId);
      context->userGenerator_.GetPoseDetectionCap().StartPoseDetection(
        POSE_TO_USE,
        nId);
    }
  }

}

#endif