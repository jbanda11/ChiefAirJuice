#include <OgreSceneManager.h>
#include <Ogre.h>
#include "btBulletDynamicsCommon.h"

#ifndef __OgreMotionState_h_
#define __OgreMotionState_h_

// TODO Credit
class OgreMotionState : public btMotionState {
protected:
  Ogre::SceneNode* mVisibleobj;
  btTransform mPos1;
public:
  OgreMotionState(const btTransform &initialpos, Ogre::SceneNode* node) {
    mVisibleobj = node;
    mPos1 = initialpos;
  }

  virtual ~OgreMotionState() {}
  //Provides flexibility in terms of object visibility
  void setNode(Ogre::SceneNode* node) {
    mVisibleobj = node;
  }

  virtual void getWorldTransform(btTransform &worldTrans) const {
    worldTrans = mPos1;
  }

  virtual void setWorldTransform(const btTransform &worldTrans) {
    if (mVisibleobj == NULL)
    return; // silently return before we set a node

    btQuaternion rot = worldTrans.getRotation();
    mVisibleobj->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());
    btVector3 pos = worldTrans.getOrigin();
    mVisibleobj->setPosition(pos.x(), pos.y(), pos.z());

    mPos1 = worldTrans;
  }
};

#endif // #ifndef __OgreMotionState_h_
