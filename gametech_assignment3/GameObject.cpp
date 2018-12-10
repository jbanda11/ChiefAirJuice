#include "GameObject.h"

/*
	The representation of everything in our game.
*/
// Initialize GameObject count
uint GameObject::objectCount = 0;

GameObject::GameObject(
	Ogre::SceneManager* scene,
	GameManager* manager,
	Simulator* physicsEngine,
	std::string name,
	std::string meshName,
	std::string materialName,
	OgreMotionState* motionState
) :
	scene(scene),
	game_manager(manager),
	physicsEngine(physicsEngine),
	name(name),
	meshName(meshName),
	materialName(materialName),
	motionState(motionState),
{
	shouldCollide = false;
	label = LABEL_DEFAULT;
	object_id = objectCount++;

	// TODO Figure out how to pull into a static config file
	DEFAULT_MATERIAL_NAME = "Wall";

	entity = scene->createEntity(meshName);

	physicsType = NONE;

	// TODO Make optional
	if(!materialName.empty()) {
		entity->setMaterialName(materialName);
	} else {
		entity->setMaterialName(DEFAULT_MATERIAL_NAME);
	}

	sceneNode = scene->getRootSceneNode()->createChildSceneNode(name);
}

void GameObject::setParent(GameObject* parent) {
	// Remove this scene node from old parent's children
	sceneNode->getParentSceneNode()->removeChild(sceneNode);
	// Add this scene node as new parent's child
	parent->sceneNode->addChild(sceneNode);
}

void GameObject::setParent(Ogre::SceneNode* parent) {
	// Remove this scene node from old parent's children
	sceneNode->getParentSceneNode()->removeChild(sceneNode);
	// Add this scene node as new parent's child
	parent->addChild(sceneNode);
}

void GameObject::initialize() {
	sceneNode->attachObject(entity);
}

void GameObject::setPosition(Ogre::Vector3 position) {

	if(physicsType == NONE) {

		sceneNode->setPosition(position);

	} else if(physicsType == KINEMATIC) {
		btTransform newTransform;
		rigidbody->getMotionState()->getWorldTransform(newTransform);
		btVector3 newPos(position.x, position.y, position.z);
		newTransform.setOrigin(newPos);
		rigidbody->getMotionState()->setWorldTransform(newTransform);
	}
}

void GameObject::translate(Ogre::Vector3 position) {

	if(physicsType == NONE) {

		sceneNode->translate(position);

	} else {
		btTransform newTransform;
		rigidbody->getMotionState()->getWorldTransform(newTransform);
		btVector3 oldPos = newTransform.getOrigin();
		btVector3 addPos(position.x, position.y, position.z);
		btVector3 newPos = oldPos + addPos;
		newTransform.setOrigin(newPos);
		rigidbody->getMotionState()->setWorldTransform(newTransform);
	}
}

/* Takes in a vector that uses Degrees. */
void GameObject::rotate(Ogre::Vector3 yawPitchRoll) {
	Ogre::Radian yawRad(Ogre::Degree(yawPitchRoll.x));
	Ogre::Radian pitchRad(Ogre::Degree(yawPitchRoll.y));
	Ogre::Radian rollRad(Ogre::Degree(yawPitchRoll.z));

	if(physicsType == NONE) {
		sceneNode->yaw(yawRad);
		sceneNode->pitch(pitchRad);
		sceneNode->roll(rollRad);
	} else {
		// Load the old transform
		btTransform newTransform;
		btTransform oldTransform;
		rigidbody->getMotionState()->getWorldTransform(oldTransform);

		// Construct a new quat
		btQuaternion oldQuat = newTransform.getRotation();
		btQuaternion addQuat(yawRad.valueRadians(), pitchRad.valueRadians(), rollRad.valueRadians());
		btQuaternion newQuat = oldQuat * addQuat;

		// Set the new quat as rotation for the transform
		newTransform.setIdentity();
		newTransform.setRotation(newQuat);
		newTransform.setOrigin(oldTransform.getOrigin());
		// Update the transform
		rigidbody->getMotionState()->setWorldTransform(newTransform);
	}
}

void GameObject::setOrientation(Ogre::Quaternion quat) {
	if(physicsType == NONE) {
		sceneNode->setOrientation(quat);
	} else {
		// Load the old transform
		btTransform newTransform;
		btTransform oldTransform;
		rigidbody->getMotionState()->getWorldTransform(oldTransform);

		// Set the new quat as rotation for the transform
		btQuaternion btQuat(quat.x, quat.y, quat.z, quat.w);
		newTransform.setIdentity();
		newTransform.setRotation(btQuat);
		newTransform.setOrigin(oldTransform.getOrigin());
		// Update the transform
		rigidbody->getMotionState()->setWorldTransform(newTransform);
	}
}

GameObject::~GameObject() {
}

void GameObject::enableCollisions() {
	// Add collision information
	collisionContext = new CollisionContext();
	contactCallback = new BulletContactCallback(*rigidbody, *collisionContext);
	shouldCollide = true;
}

// NOTE: Attach after you set position and rotation on the GO
void GameObject::attachBoxCollider(Ogre::Vector3 dimensions, float mass) {

	physicsType = DYNAMIC;

	// Create bt Transform
	btTransform transform;
	transform.setIdentity();

	// Set Transform rotation
	Ogre::Quaternion quaternion = sceneNode->getOrientation();
	btQuaternion btQuat(quaternion.x, quaternion.y, quaternion.z, quaternion.w);
	transform.setRotation(btQuat);

	// Set Transform position
	transform.setOrigin(btVector3(sceneNode->getPosition().x, sceneNode->getPosition().y, sceneNode->getPosition().z));

	// Set shape and variables
	btScalar body_mass = mass;
	btVector3 localInertia(0, 0, 0);
	btCollisionShape* rigidShape = new btBoxShape(btVector3(dimensions.x * 0.5, dimensions.y * 0.5, dimensions.z * 0.5));
	rigidShape->calculateLocalInertia(body_mass, localInertia);

	if (!motionState)
		motionState = new OgreMotionState(transform, sceneNode);
	else {
		motionState->setWorldTransform(transform);
		motionState->setSceneNode(sceneNode);
	}

	// Create rigidbody
	btRigidBody::btRigidBodyConstructionInfo rbInfo(body_mass, motionState, rigidShape, localInertia);
	rigidbody = new btRigidBody(rbInfo);
	rigidbody->setRestitution(1.00f);
	rigidbody->setFriction(0);
	rigidbody->setUserPointer(this);

	// Add to engine
	physicsEngine->dynamicsWorld->addRigidBody(rigidbody);
}

/* Sphere physics are currently dynamic. */
void GameObject::attachSphereCollider(int radius, float mass) {

	physicsType = DYNAMIC;

	// Create bt Transform
	btTransform transform;
	transform.setIdentity();
	// Set Transform position
	transform.setOrigin(btVector3(sceneNode->getPosition().x, sceneNode->getPosition().y, sceneNode->getPosition().z));

	// Set shape and variables
	btScalar body_mass = mass;
	btVector3 localInertia(0, 0, 0);
	btCollisionShape* rigidShape = new btSphereShape(radius);
	rigidShape->calculateLocalInertia(body_mass, localInertia);

	if (!motionState)
		motionState = new OgreMotionState(transform, sceneNode);
	else {
		motionState->setWorldTransform(transform);
		motionState->setSceneNode(sceneNode);
	}

	// Create rigidbody
	btRigidBody::btRigidBodyConstructionInfo rbInfo(body_mass, motionState, rigidShape, localInertia);
	rigidbody = new btRigidBody(rbInfo);
	rigidbody->setRestitution(1.00f);
	rigidbody->setFriction(0);
	rigidbody->setUserPointer(this);

	// Add to engine
	physicsEngine->dynamicsWorld->addRigidBody(rigidbody);
}

void GameObject::setKinematic() {
	if(physicsType != NONE) {
		physicsType = KINEMATIC;
		rigidbody->setCollisionFlags( rigidbody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		rigidbody->setActivationState( DISABLE_DEACTIVATION );
	} else {
		std::cerr << "Tried to setKinematic on object without physics named " << getName() << std::endl;
	}
}

/**
 * Sets the gravity for the GameObject's rigid body
 *
 * Call this AFTER adding the rigid body to the world. i.e., after a call to
 * attach___Collider()
 * @param acceleration the new acceleration for gravity
 */
void GameObject::setGravity(const btVector3 &acceleration) {
	rigidbody->setGravity(acceleration);
}


void GameObject::makeImmovable() {
	if(physicsType != NONE) {
		btVector3 zeroVec(0.0, 0.0, 0.0);
		rigidbody->setGravity(zeroVec);
		rigidbody->setLinearFactor(zeroVec);
		rigidbody->setAngularFactor(zeroVec);
	}
}

// Executes OnCollision method in all scripts attatched to this game object
void GameObject::OnCollision(GameObject *other){
	for (int i = 0; i < scripts.size(); ++i){
		scripts[i]->OnCollision(other);
	}
}

// Executes OnStep method in all scripts attatched to this game object
void GameObject::OnFrame(const Ogre::FrameEvent& fe){
	for (int i = 0; i < scripts.size(); ++i){
		scripts[i]->OnFrame(fe);
	}
}
