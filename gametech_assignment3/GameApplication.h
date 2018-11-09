/*
-----------------------------------------------------------------------------
Filename:    GameApplication.h
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/
Tutorial Framework (for Ogre 1.9)
http://www.ogre3d.org/wiki/
-----------------------------------------------------------------------------
*/

#ifndef __GameApplication_h_
#define __GameApplication_h_

#include "BaseApplication.h"
#include "GameObject.h"
#include "GameManager.h"
#include "Simulator.h"
#include "NetManager.h"

// ScriptHandler child scripts to attatch in this scene
#include "ScriptHandlers/ChangeMaterialOnCollision.h"
#include "ScriptHandlers/BallSpawner.h"
#include "ScriptHandlers/RotateAnimationScript.h"
#include "ScriptHandlers/CollisionHealth.h"
#include "ScriptHandlers/DestroyOnCollision.h"

#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>

//---------------------------------------------------------------------------

// TODO Use to control game state
enum GameState {
	GAME_NETWORK_SETUP, // GUI shows options to start a server or connect to a server
	GAME_RUNNING, // Game actually running
	GAME_END // Game is over, win/lose screen and go back to network setup (Stretch)
};

enum PacketType {
	PACKET_DEFAULT = 400,
	PACKET_BALL = 401,
	PACKET_PLAYER_POS = 402,
	PACKET_GAME_INFO = 403,
	PACKET_DEATH = 404
};

// TODO: Move networking struct somewhere else
struct GamePacket {
	PacketType type;

	// BALL
	int playerIdx; // playerPos uses too
	float ballPos[3];
	float ballDir[3];

	// PLAYER
	float playerPos[3];

	// GAME INFO
	int yourId;
	int numPlayers;
	int health0;
	int health1;
	int health2;
	int health3;
	int health4;
};

struct GameInfo {
	int myId;
	std::vector<int> healths;
};

class GameApplication : public BaseApplication
{
private:
	static const Ogre::Real WALL_SIZE;
	static const Ogre::Real CAM_SPEED;
	static const Ogre::Real BALL_RADIUS;
	static const Ogre::Real BALL_SPEED;
	static const Ogre::Vector3 DEFAULT_BALL_DIRECTION;
	static const Ogre::Real MOUSE_SENSITIVITY;

	GameState gameState;

	Ogre::SceneNode* camNode;

	std::vector<GameObject> topLevelGameObjects;
	std::vector<GameObject> allGameObjects;

	GameManager* manager;
	Simulator* physicsEngine;
	NetManager* netMgr;

	// Walls
	GameObject* wallFacing;
	GameObject* wallBack;
	GameObject* wallLeft;
	GameObject* wallRight;
	GameObject* wallUp;
	GameObject* wallDown;

	// Main actors
	GameObject* player;
	std::vector<GameObject*> players;
	GameInfo* gameInfo;
	bool dead;

	// UI elements
	CEGUI::WindowManager* wmgr;
	CEGUI::Window* startMenu;
	CEGUI::FrameWindow* IPEditBox; // This is the textbox on the start menu, used for entering IP address
	bool startButton(const CEGUI::EventArgs& e);
	CEGUI::Window* gameUI;

	// Health stuff
	CEGUI::FrameWindow* yourHealthDisplay;
	std::vector<CEGUI::FrameWindow*> healthDisplays;

public:
	// Used or networking to determine if this game instance is the server
	bool isServer;
	std::string hostName;
	int numClients;

  GameApplication(void);
  virtual ~GameApplication(void);

protected:
    virtual void createScene(void);
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& fe);
    virtual bool mouseMoved(const OIS::MouseEvent& me);
private:
	// High level game flow controllers
	void initCEGUI(void);
	void startMenuUI(void);
	void startGameUI(void);
	void startGame(void);
	// Handle networking events
	void startBasicNetworking(void);

	// Game logic
	bool processUnbufferedInput(const Ogre::FrameEvent& fe);
	bool processNonPhysicsObjects(const Ogre::FrameEvent& fe);
	void makeWalls(void);
	GameObject* createWall(const std::string& name, Ogre::Vector3 position, Ogre::Vector3 direction);
	std::vector<GameObject*> spawnBalls(GameObject* source);
	Ogre::Real getBallWallThreshold(void);
	Ogre::Real generateRandomScale(void);
	void println(const std::string str);
	void createPlayer(void);
	int getPlayerIndex(int obj_id);
	GameObject* spawnPlayerCube(Ogre::Vector3 position);
	void shootBall(void);
	GameObject* spawnBall(Ogre::Vector3 pos, Ogre::Vector3 dir);


	// Controller for client update or server update
	void updateNetworkState(void);
	// Client network logic
	void clientUpdate(void);
	// Server network logic
	void serverUpdate(void);
	// Update logic for the packet received
	void packetUpdate(GamePacket* packet);
	void messageAllBut(int client, GamePacket packet);

	// Add new GUI health bar for a player that's not this player
	void messageGameInfo(void);
	CEGUI::FrameWindow* createGUIWindow(std::string name, float x, float y, float w, float h, std::string text);
	void updateGameInfo(void);
	void updatePlayerHealth(int i);
	void killPlayer(int i);
};

//---------------------------------------------------------------------------

#endif // #ifndef __GameApplication_h_

//---------------------------------------------------------------------------
