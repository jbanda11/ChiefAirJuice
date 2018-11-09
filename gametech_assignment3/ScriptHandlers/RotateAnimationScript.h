#ifndef __RotateAnimationScript_h_
#define __RotateAnimationScript_h_

#include <string>
#include <stdlib.h>
#include <OgreMotionState.h>

#include "ScriptHandler.h"
#include "GameObject.h"

class RotateAnimationScript : public ScriptHandler {

private:
  std::vector<Ogre::Quaternion> poses;
  std::vector<float> times;

  bool animating;
  float timePassed;
  int nextPoseIdx;

public:
  RotateAnimationScript(GameObject* parentObject, std::vector<Ogre::Quaternion> poses, std::vector<float> times) :
    ScriptHandler(parentObject),
    poses(poses),
    times(times),
    animating(false)
  {
  }

  void startAnimation() {
    animating = true;
    timePassed = 0.0f;
    parent->setOrientation(poses[0]);
    nextPoseIdx = 1;
  }

  void OnFrame(const Ogre::FrameEvent& fe) {

    // Error
    if(poses.size() != times.size()) {
          std::cerr << "Trying to animate with #poses not matching #times!" << std::endl;
          return;
    }

    // Actually animate
    if(animating) {

      if(nextPoseIdx <= poses.size()) {
        timePassed += fe.timeSinceLastFrame;
        float t = (timePassed / (times[nextPoseIdx] - times[nextPoseIdx - 1])) - times[nextPoseIdx - 1];
        if(t < 1.0f) {
          // SLERP
          parent->setOrientation(Ogre::Quaternion::Slerp(t, poses[nextPoseIdx - 1], poses[nextPoseIdx]));
        } else {
          // Next pose time
          nextPoseIdx++;
        }
      } else {
        // Done animating
        animating = false;
      }
    }
    return;
  }
};

#endif