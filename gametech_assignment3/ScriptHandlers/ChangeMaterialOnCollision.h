#include <string>

#include "ScriptHandler.h"
#include "GameObject.h"

class ChangeMaterialOnCollision : public ScriptHandler {

  std::string new_material;

public:
  ChangeMaterialOnCollision(GameObject* parentObject, std::string materialName):
    ScriptHandler(parentObject),
    new_material(materialName) {}

  void OnCollision(GameObject* other){
    other->setMaterial(new_material);
  }

};
