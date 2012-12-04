#ifndef NODE_OPENNI_JOINTS_H
#define NODE_OPENNI_JOINTS_H

#include <v8.h>

#define NODE_OPENNI_JOINT_COUNT 24

using namespace v8;

namespace nodeopenni {

    const XnSkeletonJoint joints [NODE_OPENNI_JOINT_COUNT] = {
      XN_SKEL_HEAD,
      XN_SKEL_NECK,
      XN_SKEL_TORSO,
      XN_SKEL_WAIST,
      XN_SKEL_LEFT_COLLAR,
      XN_SKEL_LEFT_SHOULDER,
      XN_SKEL_LEFT_ELBOW,
      XN_SKEL_LEFT_WRIST,
      XN_SKEL_LEFT_HAND,
      XN_SKEL_LEFT_FINGERTIP,
      XN_SKEL_RIGHT_COLLAR,
      XN_SKEL_RIGHT_SHOULDER,
      XN_SKEL_RIGHT_ELBOW,
      XN_SKEL_RIGHT_WRIST,
      XN_SKEL_RIGHT_HAND,
      XN_SKEL_RIGHT_FINGERTIP,
      XN_SKEL_LEFT_HIP,
      XN_SKEL_LEFT_KNEE,
      XN_SKEL_LEFT_ANKLE,
      XN_SKEL_LEFT_FOOT,
      XN_SKEL_RIGHT_HIP,
      XN_SKEL_RIGHT_KNEE,
      XN_SKEL_RIGHT_ANKLE,
      XN_SKEL_RIGHT_FOOT
    };

    static const char * jointNames [NODE_OPENNI_JOINT_COUNT] = {
      "head",
      "neck",
      "torso",
      "waist",
      "left_collar",
      "left_shoulder",
      "left_elbow",
      "left_wrist",
      "left_hand",
      "left_fingertip",
      "right_collar",
      "right_shoulder",
      "right_elbow",
      "right_wrist",
      "right_hand",
      "right_fingertip",
      "left_hip",
      "left_knee",
      "left_ankle",
      "left_foot",
      "right_hip",
      "right_knee",
      "right_ankle",
      "right_foot"
    };
}

#endif