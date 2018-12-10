#ifndef __GameObject_h_
#define __GameObject_h_

#include <string>

#include <OgreSceneManager.h>
#include <OgreEntity.h>
#include <OgreRoot.h>
#include <OgreMotionState.h>

#include "Simulator.h"
#include "ScriptHandler.h"
#include "CollisionContext.h"

// string conversion work aroung from:
// https://stackoverflow.com/questions/12975341/to-string-is-not-a-member-of-std-says-g-mingw
namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

enum ObjectLabel {
    LABEL_DEFAULT,
    LABEL_BALL,
    LABEL_PLAYER
};

// FIXME: This is one example of circular dependencies that are resulting from
// either my bad understanding of component based architecture (likely), or the
// structure of our component based architecture - Ben
class GameManager;

class GameObject {


  // TODO: Don't make everything public lol
  public:
    // TODO use these
    enum PhysicsType { NONE, KINEMATIC, DYNAMIC, STATIC };

    // Used to keep track of number of objects in game
    static uint objectCount;
    // This instance's object id
    uint object_id;
    ObjectLabel label;
    int customLabel; // MUST HAVE ON SERVER for player # and for who spawned a ball

    // Used and initialized for anything not given a material
    std::string DEFAULT_MATERIAL_NAME;

    // Important representations
    Ogre::SceneManager* scene;
    Simulator* physicsEngine;
    Ogre::Entity* entity;
    Ogre::SceneNode* sceneNode;

    std::string name;
    std::string meshName;
    std::string materialName;
    GameManager* game_manager;

    // Physics components
    PhysicsType physicsType;
    btRigidBody* rigidbody;
    OgreMotionState* motionState;

    // Collision components
    bool shouldCollide;
    CollisionContext* collisionContext;
    BulletContactCallback* contactCallback;

    //ScriptHandler objects attatched to this game object
    std::vector<ScriptHandler *> scripts;

    GameObject(
        Ogre::SceneManager* scene,
        GameManager* manager,
        Simulator* physicsEngine,
        std::string name,
        std::string meshName,
        std::string materialName
    );
    void initialize(void);
    virtual ~GameObject(void);

    // GameObject logic handling methods
    std::string getName(void) { return name; }
    void setName(std::string n) { name = n; }
    void setParent(GameObject* parent);
    void setParent(Ogre::SceneNode* parent);

    // Ogre methods (some also interact with Bullet)
    Ogre::Vector3 getPosition() { return sceneNode->getPosition(); }
    void setPosition(Ogre::Vector3 position);
    void translate(Ogre::Vector3 position);
    void rotate(Ogre::Vector3 yawPitchRoll); // yawPitchRoll for rotation
    void setOrientation(Ogre::Quaternion quat); // yawPitchRoll for rotation
    void setMaterial(std::string materialName) { entity->setMaterialName(materialName); }

    // Bullet rigidbody and collider methods
    void attachBoxCollider(Ogre::Vector3 dimensions, float mass);
    void attachSphereCollider(int radius, float mass);
    btRigidBody* getRigidBody(){
      return rigidbody;
    }
    void setKinematic(void);
    void setGravity(const btVector3 &acceleration);
    void makeImmovable(void);

    // ScriptHandler iteration methods
    void OnCollision(GameObject *other);
    void OnFrame(const Ogre::FrameEvent& fe);

    void enableCollisions(void);
  };


#endif // #ifndef __GameObject_h_
