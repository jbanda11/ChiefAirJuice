/* A motion state for a player that allows the camera to follow you */

#include "OgreMotionState.h"

#ifndef __PlayerMotionState_h_
#define __PlayerMotionState_h_

class PlayerMotionState : public OgreMotionState {
public:
	PlayerMotionState(Ogre::SceneNode* cameraNode, const btTransform &initialPos, Ogre::SceneNode* node) {
		super(initialPos, node);
		mCameraNode = mCameraNode;
	}

	virtual void setWorldTransform(const btTransform &worldTrans) {

		btTransform currentTransform;
		getWorldTransform(currentTransform);
		btVector3 currentPosition = currentTransform.getOrigin();

    	if (mVisibleobj == NULL)
    		return; // silently return before we set a node

	    btQuaternion rot = worldTrans.getRotation();
	    mVisibleobj->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());

	    btVector3 pos = worldTrans.getOrigin();
	    mVisibleobj->setPosition(pos.x(), pos.y(), pos.z());

	    mPos1 = worldTrans;

	    btVector3 difference = pos - currentPosition;
	    node->translate(difference.x(), difference.y(), difference.z());
  	}

private:
	Ogre::SceneNode* mCameraNode;
}

#endif // __PlayerMotionState_h_ 
