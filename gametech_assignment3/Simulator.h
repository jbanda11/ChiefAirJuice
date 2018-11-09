/*
-----------------------------------------------------------------------------
Filename:    Simulator.h
-----------------------------------------------------------------------------
Created by: Benjamin Johnson

// File for running physics simulation using Bullet

*/

#ifndef _Simulator_H_
#define _Simulator_H_

#include "btBulletDynamicsCommon.h"
#include <OgreRoot.h>
#include "CollisionCallback.h"

class Simulator {

  btSequentialImpulseConstraintSolver* solver;
  btBroadphaseInterface* overlappingPairCache;
  btCollisionDispatcher* dispatcher;
  btDefaultCollisionConfiguration* collisionConfig;
  MyContactResultCallback callback;

  public:
    btDiscreteDynamicsWorld* dynamicsWorld;
    // Simulation Objects
    //keep track of the shapes, release memory at shut down.
  	btAlignedObjectArray<btCollisionShape*> collisionShapes;

    // Current collision pairs, updated every physics step! Be careful with these!
    // Index into each of these to find two objects which are colliding
    std::vector<const btCollisionObject*> callingCollisions;
    std::vector<const btCollisionObject*> resultCollisions;

    Simulator(void);
    virtual ~Simulator(void);
    // void runSimulation();
    void stepSimulation(const double timeSinceFrame);
    void initialize();

  private:
    void cleanup();

};

#endif
