#include <string>
#include <stdlib.h>     /* srand, rand */
#include <sstream>

#include <OgreRoot.h>

#include "ScriptHandler.h"
#include "GameObject.h"

class BallSpawner : public ScriptHandler {

  // Keeps track of the number of balls spawned from this spawner
  int balls_spawned;
  // An float representing the %chance that a ball spawns each step
  float spawn_percent;


public:
  BallSpawner(GameObject* parentObject, float spawn_chance):
    ScriptHandler(parentObject),
    spawn_percent(spawn_chance) {
      balls_spawned = 0;
  }

  void OnFrame(const Ogre::FrameEvent& fe) {
    float x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/100));
    if (spawn_percent >= x){
      // spawn a ball relative to parent
      Ogre::Vector3 parent_position = this->parent->getPosition();

      float radius = 20;
      GameObject* bouncy = this->parent->game_manager->createSphereGameObject(this->parent->getName() +"_SpawnedBall_" + patch::to_string(balls_spawned), radius);
      // FIXME Just spwaning 100 units above object, need a way to spawn above any object relative to its size
    	bouncy->setPosition(Ogre::Vector3(parent_position.x, parent_position.y + 300, parent_position.z));
    	bouncy->attachSphereCollider(radius, 0.1);
      int x_f = rand() % 4 - 2;
      int y_f = rand() % 4;
      int z_f = rand() % 4 - 2;
    	bouncy->rigidbody->applyCentralImpulse(btVector3(x_f, y_f, z_f) * 5);
    	bouncy->initialize();

      balls_spawned++;
    }
    return;
  }
};
