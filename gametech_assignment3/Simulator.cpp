/*
-----------------------------------------------------------------------------
Filename:    Simulator.cpp
-----------------------------------------------------------------------------
Created by: Benjamin Johnson

// File for running physics simulation using Bullet

*/
#include "Simulator.h"


Simulator::Simulator(void){
  initialize();
}

Simulator::~Simulator(void){
}

// TODO: ADD a boolean return to say if any objects were moved
// Updates the position of all the bullet rigidbodies in the world
// and calls stepSimulation
void Simulator::stepSimulation(double timeSinceLastFrame){
  // Update all rigidbody objects in dynamicsWorld
  callback.resetCollisionLists();
  for (int i = 0; i < dynamicsWorld->getNumCollisionObjects(); ++i){
    btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
    btRigidBody* rigidbody = btRigidBody::upcast(obj);

    dynamicsWorld->contactTest(rigidbody, callback);
  }
  callingCollisions = callback.callingCollisions;
  resultCollisions = callback.resultCollisions;
  // TODO: Make sure that this value is correct!
  dynamicsWorld->stepSimulation(timeSinceLastFrame * 5); //FIXME: LOCAL SPEED OF GAME, CHANGE THIS
}

// Initializes world settings for bullet physics
void Simulator::initialize(){
  collisionConfig = new btDefaultCollisionConfiguration();

  dispatcher = new btCollisionDispatcher(collisionConfig);

  overlappingPairCache = new btDbvtBroadphase();

  solver = new btSequentialImpulseConstraintSolver();

  dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,
    overlappingPairCache, solver, collisionConfig);

  dynamicsWorld->setGravity(btVector3(0,-10,0));
}

// Tears down world variables and physics objects in the scene
void Simulator::cleanup(){

  printf("%s\n", "Terminating simulation...");

  // Delete Rigidbodies
  for (int i=dynamicsWorld->getNumCollisionObjects()-1; i>=0; --i){
    btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
    btRigidBody* body = btRigidBody::upcast(obj);

    if (body && body->getMotionState()){
      delete body->getMotionState();
    }
    dynamicsWorld->removeCollisionObject(obj);
    delete obj;
  }

  //Delete collision shapes
  for (int j=0; j<collisionShapes.size(); j++){
    btCollisionShape* shape = collisionShapes[j];
    collisionShapes[j] = 0;
    delete shape;
  }

  delete dynamicsWorld;
  delete solver;
  delete overlappingPairCache;
  delete dispatcher;

  collisionShapes.clear();

  printf("%s\n", "Simulation terminated.");
}
