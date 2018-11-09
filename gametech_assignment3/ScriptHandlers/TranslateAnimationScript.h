#ifndef __TranslateAnimationScript_h_
#define __TranslateAnimationScript_h_

#include <string>
#include <stdlib.h>
#include <OgreMotionState.h>

#include "ScriptHandler.h"
#include "GameObject.h"

class TranslateAnimationScript : public ScriptHandler {

private:
  std::vector<Ogre::Vector3> poses;
  std::vector<float> times;

  bool animating;
  float timePassed;
  int nextPoseIdx;

public:
  TranslateAnimationScript(GameObject* parentObject, std::vector<Ogre::Vector3> poses, std::vector<float> times) :
    ScriptHandler(parentObject),
    poses(poses),
    times(times),
    animating(false)
  {
  }

  void startAnimation() {
    std::cerr << "Start of animation!" << std::endl;
    animating = true;
    timePassed = 0.0f;
    parent->setPosition(poses[0]);
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
          // Get the current translation needed
          Ogre::Vector3 translation = (1.0-t)*poses[nextPoseIdx - 1] + t*poses[nextPoseIdx];
          
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