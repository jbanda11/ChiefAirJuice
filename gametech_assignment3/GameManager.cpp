#include "GameManager.h"

GameManager::GameManager(Ogre::SceneManager* scnMgr, Ogre::Camera* ogreCam, Simulator* simulator) :
scene(scnMgr),
physicsEngine(simulator)
{
	// Count tracking total number of GOs, used in naming
	goCount = 0;

	// Initalize FPS camera
	camNode = scene->getRootSceneNode()->createChildSceneNode();
	camNode->attachObject(ogreCam);
	camNode->setPosition(Ogre::Vector3(0, 0, 0));

	// Initialize basic lights and shadows
	// Add ambient light and enable shadows
	scene->setAmbientLight(Ogre::ColourValue(.25, .25, .25));
	scene->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_MODULATIVE);

	// Create point light
	Ogre::Light* pointLight = scene->createLight("PointLight");
	pointLight->setType(Ogre::Light::LT_POINT);
	pointLight->setPosition(25, 25, 25);
	pointLight->setDiffuseColour(Ogre::ColourValue::White);
	pointLight->setSpecularColour(Ogre::ColourValue::White);
}

// Create a box GO of specified dimensions
GameObject* GameManager::createBoxGameObject(std::string name, Ogre::Vector3 dimensions) {
	GameObject* box = createGameObject(name, "cube.mesh", "");
	box->sceneNode->setScale(dimensions/DEFAULT_CUBE_DIMENSION);

	return box;
}

// Create a box GO of specified dimensions
GameObject* GameManager::createBoxGameObject(std::string name, Ogre::Vector3 dimensions, std::string materialName) {
	GameObject* box = createGameObject(name, "cube.mesh", materialName);
	box->sceneNode->setScale(dimensions/DEFAULT_CUBE_DIMENSION);

	return box;
}

// Create a sphere GO of a specified radius
GameObject* GameManager::createSphereGameObject(std::string name, float radius) {
	GameObject* ball = createGameObject(name, "sphere.mesh", "");
	float newRadius = radius/DEFAULT_SPHERE_RADIUS;
	ball->sceneNode->setScale(Ogre::Vector3(newRadius, newRadius, newRadius));

	return ball;
}

/** Creates a GO. All methods in the scene should call this. All methods creating
  * special GO types should call this. Names are automatically numbered based on a global counter of GameObjects.
  */
GameObject* GameManager::createGameObject(std::string name, std::string meshName, std::string materialName) {
	// Create GameObject
	std::ostringstream goName;
	goName << ++goCount << "_" << name;
	GameObject* gameObject = new GameObject(scene, this, physicsEngine, goName.str(), meshName, materialName);
	// Track GameObject
	allGameObjects.push_back(gameObject);

	return gameObject;
}

// Remove a game object from the manager by name
GameObject* GameManager::removeGameObject(std::string name){
	std::vector <GameObject*> newObjList;
	GameObject* objToRemove = NULL;

	for (int i = 0; i < allGameObjects.size(); ++i){
		if (allGameObjects[i]->name.compare(name) == 0){
			objToRemove = allGameObjects[i];
		} else{
			newObjList.push_back(allGameObjects[i]);
		}
	}
	allGameObjects = newObjList;
	return objToRemove;
}

// TODO Doesn't work well rn cus we number GOs. Change if needed.
GameObject* GameManager::findGameObject(std::string name) {
	for(int i = 0; i < allGameObjects.size(); i++) {
		if(allGameObjects[i]->getName() == name) {
			return allGameObjects[i];
		}
	}

	return NULL;
}

// TODO This needs to be maintained for client and server id's to be
// in sync, also it is confusing to initialize by name and ID
// Find an object by ID
GameObject* GameManager::findGameObjectID(uint id) {
	for(int i = 0; i < allGameObjects.size(); i++) {
		if(allGameObjects[i]->object_id == id) {
			return allGameObjects[i];
		}
	}
	return NULL;
}

void GameManager::printGameObjects(void) {
  std::cout << "Printing " << allGameObjects.size() << " GameObjects:\n" << std::endl;
  for(int i = 0; i < allGameObjects.size(); i++) {
      std::cout << allGameObjects[i]->getName() << std::endl;
  }
}

// Removes the object and its children from Ogre
// Recursive function to kill a SceneNode and all its children
// Code from https://forums.ogre3d.org/viewtopic.php?f=2&t=53647&start=0
void GameManager::destroySceneNode( Ogre::SceneNode* node )
{
	if(!node) return;

	// Destroy all the attached objects
	Ogre::SceneNode::ObjectIterator itObject = node->getAttachedObjectIterator();

	while ( itObject.hasMoreElements() )
		node->getCreator()->destroyMovableObject(itObject.getNext());

	// Recurse to child SceneNodes
	Ogre::SceneNode::ChildNodeIterator itChild = node->getChildIterator();

	while ( itChild.hasMoreElements() )
	{
		Ogre::SceneNode* pChildNode = static_cast<Ogre::SceneNode*>(itChild.getNext());
		destroySceneNode( pChildNode );
	}

	// Remove the node from the SceneManager
	scene->destroySceneNode(node);
}

// Removes the object from Bullet physics
void GameManager::destroyRigidBody(GameObject* object)
{
	if(object->rigidbody != NULL) {
		physicsEngine->dynamicsWorld->removeCollisionObject(object->rigidbody);
	}
}

// Completely removes this game object from the game, Bullet, and Ogre
// A wrapper around the other destory methods
void GameManager::destroyObject(GameObject* object) {
	// Remove from Ogre
	destroySceneNode(object->sceneNode);
	// Remove from Bullet
	destroyRigidBody(object);
	//Remove from the manager
	removeGameObject(object->name);
}

Ogre::SceneNode* GameManager::getCamera(void)
{
	return camNode;
}
