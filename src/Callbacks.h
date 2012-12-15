#ifndef NODE_OPENNI_CALLBACKS_H
#define NODE_OPENNI_CALLBACKS_H

namespace nodeopenni {
  
  /// OpenNI Callbacks
  void XN_CALLBACK_TYPE
  User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie);

  void XN_CALLBACK_TYPE
  User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie);
  
  void XN_CALLBACK_TYPE
  User_UserExit(xn::UserGenerator& generator, XnUserID nId, void* pCookie);

  void XN_CALLBACK_TYPE
  Pose_Detected(xn::PoseDetectionCapability& pose, const XnChar* strPose,
    XnUserID nId, void* pCookie);


  void XN_CALLBACK_TYPE
  Calibration_Start(xn::SkeletonCapability& capability, XnUserID nId,
                    void* pCookie);

  void XN_CALLBACK_TYPE
  Calibration_End(xn::SkeletonCapability& capability, XnUserID nId,
                  XnBool bSuccess, void* pCookie);

  void XN_CALLBACK_TYPE
  Joint_Configuration_Change(xn::ProductionNode &node, void* pCookie);

  // LibUV Callbacks
  static void async_joint_change_callback_(uv_async_t *handle, int notUsed);

}

#endif