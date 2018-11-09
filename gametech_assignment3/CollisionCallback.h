#ifndef _CollisionCallback_H_
#define _CollisionCallback_H_

#include "BulletCollision/CollisionDispatch/btCollisionWorld.h"

class MyContactResultCallback : public btCollisionWorld::ContactResultCallback {

public:
    // These are lists keeping track of collision pairs where calling is colObj0Wrap
    // and result is colObj1Wrap, index into both of these to find a pair
    // These must be reset each frame to only show collisions for current frame
    std::vector<const btCollisionObject*> callingCollisions;
    std::vector<const btCollisionObject*> resultCollisions;

    // Call this every frame before testing for object collisions
    void resetCollisionLists(){
      callingCollisions.clear();
      resultCollisions.clear();
    }

private:
    btScalar addSingleResult(btManifoldPoint& cp,
        const btCollisionObjectWrapper* colObj0Wrap,
        int partId0,
        int index0,
        const btCollisionObjectWrapper* colObj1Wrap,
        int partId1,
        int index1)
    {
        if (colObj0Wrap != NULL && colObj1Wrap != NULL){
          callingCollisions.push_back(colObj0Wrap->getCollisionObject());
          resultCollisions.push_back(colObj0Wrap->getCollisionObject());
        }
    }
};

#endif
