#include "ScriptHandler.h"
#include "GameObject.h"
#include <OgreRoot.h>

#include <SDL2/SDL.h>
#include "audio.h"

class CollisionHealth : public ScriptHandler {

public:
  int health;

  CollisionHealth(GameObject* parentObject, int start_health):
    ScriptHandler(parentObject),
    health(start_health) {}

};
