#ifndef __GameManager_h_
#define __GameManager_h_

#include <string>
#include <sstream>

#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreRoot.h>
#include <OgreFrameListener.h>

#include <GameObject.h>
#include <Simulator.h>


class GameManager {

private:
  static const float DEFAULT_CUBE_DIMENSION = 100.0f;
  static const float DEFAULT_SPHERE_RADIUS = 100.0f;

  Ogre::SceneManager* scene;
  Simulator* physicsEngine;

  Ogre::SceneNode* camNode;

  int goCount;

public:
  // Added to allow for networking of gameobjects creation and deletion
  bool isServer;
  
  bool soundOn;
  std::vector<GameObject*> allGameObjects;

  // Give the GameManager an initial scene, camera, and physics engine
	GameManager(Ogre::SceneManager* scnMgr, Ogre::Camera* ogreCam, Simulator* simulator);

  GameObject* createBoxGameObject(std::string name, Ogre::Vector3 dimensions);
  GameObject* createSphereGameObject(std::string name, float radius);

	// TODO Way to attach no material?
	GameObject* createGameObject(std::string name, std::string meshName, std::string materialName);
	GameObject* findGameObject(std::string name);
  GameObject* findGameObjectID(uint id);
  GameObject* removeGameObject(std::string name);

  //Delete this object from the GameManager, Ogre, and Bullet
  void destroyObject(GameObject* object);
  void destroyRigidBody(GameObject* object);
  void destroySceneNode(Ogre::SceneNode* node);

  // Prints a list of GameObjects tracked by this manager
  void printGameObjects(void);

  Ogre::SceneNode* getCamera();

	///// TODO: ERROR if name is non-unique
	// Create a top level GO
	// Create a GO with a parent

	// Get GO by name

};

#endif // #ifndef __GameManager_h_
