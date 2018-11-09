#include <string>

#include "ScriptHandler.h"
#include "GameObject.h"

class DestroyOnCollision : public ScriptHandler {

public:
  DestroyOnCollision(GameObject* parentObject):
    ScriptHandler(parentObject) {}

  void OnCollision(GameObject* other){
  	if(other->label != LABEL_PLAYER) {
	    this->parent->game_manager->destroyObject(this->parent);
  	}
  }

};
