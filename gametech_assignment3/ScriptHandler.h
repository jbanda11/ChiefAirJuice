/*
-----------------------------------------------------------------------------
Filename:    ScriptHandler.h
-----------------------------------------------------------------------------
Created by: Benjamin Johnson

// File for abstract Script class, which allows for custom scripts
// to be added to game objects
*/

#ifndef _ScriptHandler_H_
#define _ScriptHandler_H_

class GameObject;

class ScriptHandler
{
protected:
  GameObject* parent;
public:
    // Defines how a script behaves on object collision
    // Must override!
    virtual void OnCollision(GameObject* other) {
      return;
    }

    // Defines how a script behaves on a step of the physics engine
    // Must override!
    virtual void OnStep() {
      return;
    }

    virtual void OnFrame(const Ogre::FrameEvent& fe) {
      return;
    }

    ScriptHandler(GameObject* parentObject):
      parent(parentObject)
      {}
};

#endif
