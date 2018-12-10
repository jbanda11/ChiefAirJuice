	/*
-----------------------------------------------------------------------------
Filename:    GameApplication.cpp
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

#include "GameApplication.h"
#include "PlayerMotionState.h"
#include <unistd.h>
#include <limits.h>
#include <iostream>

//---------------------------------------------------------------------------

const Ogre::Real GameApplication::WALL_SIZE = 8000;
const Ogre::Real GameApplication::CAM_SPEED = 800;
const Ogre::Real GameApplication::MOUSE_SENSITIVITY = .13;

//
GameApplication::GameApplication(void) : isServer(true)
{
	dead = false;
	// Initialize players and health info, only server actually knows its own id so far
	gameInfo = new GameInfo();
	for (int i = 0; i < 5; ++i){
		gameInfo->healths.push_back(-1);
	}
	gameInfo->healths[0] = 3;
	if(isServer) {
		gameInfo->myId = 0;
	}
}
//---------------------------------------------------------------------------
GameApplication::~GameApplication(void)
{
}

void GameApplication::initCEGUI(void) {
	// Initialize UI system
	mRenderer = &CEGUI::OgreRenderer::bootstrapSystem();
	CEGUI::ImageManager::setImagesetDefaultResourceGroup("General");
	CEGUI::Font::setDefaultResourceGroup("General");
	CEGUI::Scheme::setDefaultResourceGroup("General");
	CEGUI::WidgetLookManager::setDefaultResourceGroup("General");
	CEGUI::WindowManager::setDefaultResourceGroup("General");

	CEGUI::SchemeManager::getSingleton().createFromFile("TaharezLook.scheme");
	CEGUI::System::getSingleton().getDefaultGUIContext().getMouseCursor().setDefaultImage("TaharezLook/MouseArrow");

	// This startMenu is supposed to show the cursor
	wmgr = &CEGUI::WindowManager::getSingleton();
	startMenu = wmgr->createWindow("DefaultWindow", "CEGUIDemo/Start");
	gameUI = wmgr->createWindow("DefaultWindow", "CEGUIDemo/Game");
}

void GameApplication::startMenuUI() {
	gameState = GAME_NETWORK_SETUP;

	if (isServer){
		CEGUI::FrameWindow* ipTextWindow = static_cast<CEGUI::FrameWindow*>( wmgr->createWindow( "TaharezLook/StaticText", "IpAddrText" ));
		ipTextWindow->setPosition(CEGUI::UVector2(CEGUI::UDim(0.365,0),CEGUI::UDim(0.475,0)));
		ipTextWindow->setSize(CEGUI::USize(CEGUI::UDim(0,350),CEGUI::UDim(0,50)));
		std::ostringstream infoStr;
		infoStr << "Server IP is \"" << netMgr->getIPstring().c_str() << "\"";
		ipTextWindow->setText(infoStr.str());
		startMenu->addChild(ipTextWindow);
	} else {
		IPEditBox = static_cast<CEGUI::FrameWindow*>( wmgr->createWindow( "TaharezLook/Editbox", "IPBoxUI" ));
		IPEditBox->setPosition(CEGUI::UVector2(CEGUI::UDim(0.365,0),CEGUI::UDim(0.475,0)));
		IPEditBox->setSize(CEGUI::USize(CEGUI::UDim(0,350),CEGUI::UDim(0,50)));
		IPEditBox->setText("000.00.000.00");
		startMenu->addChild(IPEditBox);
	}

	CEGUI::FrameWindow* startGameBtn = static_cast<CEGUI::FrameWindow*>( wmgr->createWindow( "TaharezLook/Button", "startButtonUI" ));
	startGameBtn->setPosition(CEGUI::UVector2(CEGUI::UDim(0.445,0),CEGUI::UDim(0.6,0)));
	startGameBtn->setSize(CEGUI::USize(CEGUI::UDim(0,200),CEGUI::UDim(0,100)));
	startGameBtn->setText("Start Game");
	startMenu->addChild(startGameBtn);

	startGameBtn->subscribeEvent(CEGUI::PushButton::EventClicked,CEGUI::Event::Subscriber(&GameApplication::startButton, this));

	// Set up this GUI
	CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(startMenu);
}

bool GameApplication::startButton(const CEGUI::EventArgs& e){
	hostName = IPEditBox->getText().c_str();
	startBasicNetworking();
	startGame();
	return true;
};

void GameApplication::startGameUI(void) {

	// Set up this GUI
	CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(gameUI);

	// Create your health
	yourHealthDisplay = createGUIWindow("YourHealth", 0.75, 0.85, 0.2, 0.1, "You: -");
	gameUI->addChild(yourHealthDisplay);

	float infoXStart = 0.01;
	float infoYStart = 0.01;
	float infoWidth = 0.2;
	float infoHeight = 0.05;
	// Create all healths
	CEGUI::FrameWindow* gameInfoHeader = createGUIWindow("GameInfoHeader", infoXStart, infoYStart, infoWidth*4, infoHeight, "Players");
	if(isServer) {
		std::ostringstream infoStr;

		infoStr << "Tell your friends to connect to hostname " << serverHostName;
		gameInfoHeader->setText(infoStr.str());
	}
	gameUI->addChild(gameInfoHeader);

	for(int i = 0; i < 5; i++) {
		float y = infoYStart + (1)*infoHeight;
		float x = infoXStart + (i)*infoWidth;
		std::ostringstream winName;
		winName << "HealthDisplay " << i;
		std::ostringstream placeholder;
		placeholder << "<player " << i << " info>";
		healthDisplays.push_back(createGUIWindow(winName.str(), x, y, infoWidth, infoHeight, placeholder.str()));
		gameUI->addChild(healthDisplays[i]);
	}
}

void GameApplication::updateGameInfo(void) {

	if (gameState == GAME_RUNNING){
		for(int i = 0; i < 5; i++) {

			std::ostringstream text;

			if(gameInfo->healths[i] < 0) {
				text << "< not connected >";
			} else if (gameInfo->healths[i] == 0) {
				text << "Player " << i << "     " << "x__x";

				Ogre::Vector3 position(3500, 3500, -3500);
				Ogre::Vector3 exile(9999, 9999, 9999);

				// I'm the dead one D':
				if(i == gameInfo->myId) {
					if(!dead) {
						dead = true;
						player->setPosition(exile);
						manager->getCamera()->setPosition(position);
						yourHealthDisplay->setText("You died )-:");
					}
				} else {
					players[i]->setPosition(exile);
				}

			} else {
				text << "Player " << i << "     " << gameInfo->healths[i] << "HP";
				std::ostringstream yourText;
				yourText << "You: " << gameInfo->healths[i] << "HP";
				if(i == gameInfo->myId) {
					yourHealthDisplay->setText(yourText.str());
				}
			}

			healthDisplays[i]->setText(text.str());
		}
	}
}

CEGUI::FrameWindow* GameApplication::createGUIWindow(std::string name, float x, float y, float w, float h, std::string text) {
	CEGUI::FrameWindow* window = static_cast<CEGUI::FrameWindow*>( wmgr->createWindow( "TaharezLook/StaticText", name));
	window->setPosition(CEGUI::UVector2(CEGUI::UDim(x, 0), CEGUI::UDim(y, 0)));
	window->setSize(CEGUI::USize(CEGUI::UDim(w, 0), CEGUI::UDim(h, 0)));
	window->setText(text);
	return window;
}

//---------------------------------------------------------------------------
void GameApplication::createScene(void)
{
	initCEGUI();
	if(isServer){
		startBasicNetworking();
		startGame();
	} else {
		startMenuUI();
	}
}

void GameApplication::startBasicNetworking() {
	netMgr = new NetManager();
	numClients = 0;
	netMgr->initNetManager();

	for (int i = 0; i < 5; ++i){
		players.push_back(NULL);
	}

	if(isServer) {
		char hostname[HOST_NAME_MAX];
		if (gethostname(hostname, HOST_NAME_MAX) < 0) {
			std::cerr << "Error occurred while attempting to retrieve the hostname\n";
		}
		else {
			serverHostName = hostname;
		}

		netMgr->addNetworkInfo();
		netMgr->acceptConnections();
		if (netMgr->startServer()){
			printf("%s\n", "Server started successfully!");
			printf("=======================Server Info=======================\n");
			std::cout << "Hostname: " << hostname << '\n';
			printf("Note: If no hostname is provided, use IPaddr as server hostname.\n");
			printf("=========================================================\n");
		} else {
			printf("%s\n", "Server failed to start!");
		}
	} else {
		// TODO Make port a parameter
		netMgr->addNetworkInfo(PROTOCOL_ALL, hostName.c_str(), 51215);
		if (netMgr->startClient()){
			printf("%s\n", "Client started successfully!");
		} else {
			printf("%s\n", "Client failed to start!");
		}
	}
}

void GameApplication::updateNetworkState(){
	if(isServer) {
		serverUpdate();
	} else {
		clientUpdate();
	}
}

// Called by clientUpdate and serverUpdate for general packet logic
// UDP ONLY
void GameApplication::packetUpdate(GamePacket* packet) {
	// Create a new ball as requested by the packet
	if(packet->type == PACKET_BALL) {
		Ogre::Vector3 pos(packet->ballPos[0], packet->ballPos[1], packet->ballPos[2]);
		Ogre::Vector3 dir(packet->ballDir[0], packet->ballDir[1], packet->ballDir[2]);
		GameObject* ball = spawnBall(pos, dir);
		ball->customLabel = packet->playerIdx;
	}
}

// Only clients should call this
void GameApplication::clientUpdate(void) {
	assert(!isServer);

	netMgr->scanForActivity();

	if(netMgr->tcpServerData.updated) {
		netMgr->tcpServerData.updated = false;
		GamePacket* packet = (GamePacket*) netMgr->tcpServerData.output;
		packetUpdate(packet);

		if(packet->type == PACKET_GAME_INFO) {
			gameInfo->myId = packet->yourId;
			gameInfo->healths[0] = packet->health0;
			gameInfo->healths[1] = packet->health1;
			gameInfo->healths[2] = packet->health2;
			gameInfo->healths[3] = packet->health3;
			gameInfo->healths[4] = packet->health4;
		}

		// Update player positions
		if (packet->type == PACKET_PLAYER_POS){
			Ogre::Vector3 position(packet->playerPos[0], packet->playerPos[1], packet->playerPos[2]);
			int id = packet->playerIdx;
			if (players[id] == NULL){
				players[id] = spawnPlayerCube(position);
			} else {
				players[id]->setPosition(position);
			}
		}
	}

}

// Only servers should call this
void GameApplication::serverUpdate(void) {
	assert(isServer);

	netMgr->scanForActivity();
	int newNumClients = netMgr->getClients();
	//printf("%d\n", netMgr->getClients());
	if(newNumClients > numClients) {
		printf("We have %d new players for a total of %d players!\n", newNumClients - numClients, newNumClients + 1);
		numClients = newNumClients;
	}

	// Check each client packet
	for(int i = 0; i < numClients; i++) {
		if(netMgr->tcpClientData[i]->updated) {
			netMgr->tcpClientData[i]->updated = false;
			GamePacket* packet = (GamePacket*) netMgr->tcpClientData[i]->output;
			packetUpdate(packet);

			// If it's a ball, send to all other clients too
			if(packet->type == PACKET_BALL) {
				messageAllBut(i, *packet);
			}

			// Player position update, TODO also send to other clients with client number
			if (packet->type == PACKET_PLAYER_POS){
				Ogre::Vector3 position(packet->playerPos[0], packet->playerPos[1], packet->playerPos[2]);

				// New player case
				if (players[i+1] == NULL){
					players[i+1] = spawnPlayerCube(position);
					gameInfo->healths[i+1] = 3;
					players[i+1]->customLabel = i+1;
				} else {
					// Just update player position
					players[i+1]->setPosition(position);
					GamePacket posPacket;
					posPacket.type = PACKET_PLAYER_POS;
					posPacket.playerIdx = i+1;
					posPacket.playerPos[0] = packet->playerPos[0];
					posPacket.playerPos[1] = packet->playerPos[1];
					posPacket.playerPos[2] = packet->playerPos[2];
					messageAllBut(i, posPacket);
				}
			}

		}
	}
}

void GameApplication::messageGameInfo() {
	// Message everyone a game info packet
	GamePacket infoPack;
	infoPack.type = PACKET_GAME_INFO;
	infoPack.health0 = gameInfo->healths[0];
	infoPack.health1 = gameInfo->healths[1];
	infoPack.health2 = gameInfo->healths[2];
	infoPack.health3 = gameInfo->healths[3];
	infoPack.health4 = gameInfo->healths[4];

	for(int i = 0; i < numClients; i++) {
		infoPack.yourId = i+1;
		netMgr->messageClient(PROTOCOL_TCP, i, (char*) &infoPack, sizeof(infoPack));
	}
}

// Message all clients except client #i
void GameApplication::messageAllBut(int client, GamePacket packet) {
	for(int i = 0; i < numClients; i++) {
		if(i != client) {
			netMgr->messageClient(PROTOCOL_TCP, i, (char *)&packet, sizeof(packet));
		}
	}
}

void GameApplication::startGame() {
	startGameUI();

	SDL_Init(SDL_INIT_AUDIO);
	initAudio();
	// Time as random seed
	std::srand(std::time(NULL));

	// Create physics simulation
	physicsEngine = new Simulator();
	// Creates basic camera and lights
	manager = new GameManager(mSceneMgr, mCamera, physicsEngine);
	manager->isServer = isServer;
	physicsEngine->initialize();
	manager->soundOn = true;
	// Add ambient light and enable shadows
	mSceneMgr->setAmbientLight(Ogre::ColourValue(.85, .85, .85));
	mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_MODULATIVE);

	makeWalls();

	/* Create player */
	createPlayer();

	manager->printGameObjects();

	gameState = GAME_RUNNING;
}

/* Create a box, position camera above box */
void GameApplication::createPlayer() {
	Ogre::Vector3 position(0, wallDown->getPosition().y + 50, 1200);

	player = spawnPlayerCube(position);
	player->rigidbody->setUserPointer(player);
	player->setGravity(btVector3(0.0f, 0.0f, 0.0f));
	player->rigidbody->setActivationState(DISABLE_DEACTIVATION);

	// Move camera close to the player
	manager->getCamera()->setPosition(player->getPosition() + Ogre::Vector3(0, 50, 0));
}

GameObject* GameApplication::spawnPlayerCube(Ogre::Vector3 position) {
	GameObject* cube = manager->createBoxGameObject(
		"Player",
		Ogre::Vector3(40, 40, 40),
		"White",
		new Play
	);
	cube->label = LABEL_PLAYER;
	cube->setPosition(position);
	cube->attachBoxCollider(Ogre::Vector3(40, 40, 40), 1.0);
	// cube->setKinematic();
	cube->initialize();
	cube->enableCollisions();

	return cube;
}

void GameApplication::shootBall() {

	// Ball params
	Ogre::Vector3 dir(mCamera->getRealDirection());
	Ogre::Vector3 pos(manager->getCamera()->getPosition() + dir*100);
	// Packet
	GamePacket packet;
	packet.type = PACKET_BALL;
	packet.playerIdx = gameInfo->myId;
	packet.ballPos[0] = pos.x;
	packet.ballPos[1] = pos.y;
	packet.ballPos[2] = pos.z;
	packet.ballDir[0] = dir.x;
	packet.ballDir[1] = dir.y;
	packet.ballDir[2] = dir.z;

	GameObject* ball = spawnBall(pos, dir);
	ball->customLabel = gameInfo->myId;

	if(isServer) {
		// Message all clients
		if(numClients > 0) {
			netMgr->messageClients(PROTOCOL_TCP, (char *)&packet, sizeof(packet));
		}

	} else {
		// Message server, who will create a ball and message back to all clients FIXME (THEORETICALLY, DOESN'T WORK )-:)
		netMgr->messageServer(PROTOCOL_TCP, (char *)&packet, sizeof(packet));
	}
}

/* Create a ball in a position with a direction. */
GameObject* GameApplication::spawnBall(Ogre::Vector3 pos, Ogre::Vector3 dir) {
	float rad = 10;
	float force = 100;

	GameObject* ball = manager->createSphereGameObject("ShotBall", rad);
	ball->label = LABEL_BALL;
	ball->setPosition(pos);
	ball->attachSphereCollider(rad, 0.1);
	btVector3 btLook(dir.x, dir.y, dir.z);
	ball->rigidbody->applyCentralImpulse(btLook * force);
	ball->enableCollisions();
	DestroyOnCollision* script = new DestroyOnCollision(ball);
	ball->scripts.push_back(script);
	ball->initialize();

	return ball;
}

void GameApplication::makeWalls() {
	/* Create wall GameObjects and store them in walls */
	std::vector<GameObject*> walls;
	wallFacing = createWall("WallFacingNode", Ogre::Vector3(0, 0, -WALL_SIZE/2), Ogre::Vector3::ZERO);
	wallBack = createWall("WallBackNode", Ogre::Vector3(0, 0, WALL_SIZE/2), Ogre::Vector3(180, 0, 0));
	wallLeft = createWall("WallLeftNode", Ogre::Vector3(-WALL_SIZE/2, 0, 0), Ogre::Vector3(90, 0, 0));
	wallRight = createWall("WallRightNode", Ogre::Vector3(WALL_SIZE/2, 0, 0),  Ogre::Vector3(-90, 0, 0));
	wallUp = createWall("WallUpNode",  Ogre::Vector3(0, WALL_SIZE/2, 0),  Ogre::Vector3(0, 90, 0), "Sky");
	wallDown = createWall("WallDownNode", Ogre::Vector3(0, -WALL_SIZE/2, 0), Ogre::Vector3(0, -90, 0), "Field");
	walls.push_back(wallFacing);
	walls.push_back(wallBack);
	walls.push_back(wallLeft);
	walls.push_back(wallRight);
	walls.push_back(wallUp);
	walls.push_back(wallDown);
}

GameObject* GameApplication::createWall(const std::string& name, Ogre::Vector3 position, Ogre::Vector3 yawPitchRoll) {
	return createWall(name, position, yawPitchRoll, "");
}

GameObject* GameApplication::createWall(const std::string& name, Ogre::Vector3 position, Ogre::Vector3 yawPitchRoll, std::string materialName) {
	Ogre::Vector3 dimensions = Ogre::Vector3(WALL_SIZE, WALL_SIZE, 10);
	GameObject* wall = manager->createBoxGameObject(name, dimensions, materialName);
	wall->setPosition(position);
	wall->rotate(yawPitchRoll);
	wall->attachBoxCollider(dimensions, 1.0);
	wall->makeImmovable();
	wall->initialize();

	return wall;
}

// Generate a random number between -1 and 1
Ogre::Real GameApplication::generateRandomScale() {
	return ((Ogre::Real) rand() / (RAND_MAX)) * 2 - 1;
}

// Called every frame
bool GameApplication::frameRenderingQueued(const Ogre::FrameEvent& fe) {

	bool ret = BaseApplication::frameRenderingQueued(fe);

	if (gameState != GAME_RUNNING){
		return true;
	}

	updateNetworkState();

	static float sinceGameInfoUpdate = 0;

	sinceGameInfoUpdate += fe.timeSinceLastFrame;

	// Updates health GUI
	updateGameInfo();

	// TODO: Move all this step and ScriptHandler logic into GameManager
	physicsEngine->stepSimulation(fe.timeSinceLastFrame);

	// Calls OnFrame for every script attatched to every gameobject
	for (int i = 0; i < manager->allGameObjects.size(); ++i){
		GameObject* obj = manager->allGameObjects[i];
		obj->OnFrame(fe);

		// Collisions
		if(obj->shouldCollide) {
			physicsEngine->dynamicsWorld->contactTest(obj->rigidbody, *(obj->contactCallback));
			if(obj->collisionContext->hit && ((GameObject*) obj->collisionContext->body->getUserPointer() != (GameObject*) obj->collisionContext->body)) {
				GameObject* other = (GameObject*) obj->collisionContext->body->getUserPointer();
				obj->OnCollision(other);

				// Lazily going to put player collision code here instead of script-- too complicated to pass in application context :p
				if(obj->label == LABEL_PLAYER && other->label == LABEL_BALL) {
					// Only server detects collisions for health
					if (isServer){
						int hitPlayer = obj->customLabel;
						printf("DONK, player #%d was hit by player #%d!\n", hitPlayer, other->customLabel);
						if(gameInfo->healths[hitPlayer] > 0) {
							gameInfo->healths[hitPlayer]--;
						}

						if(gameInfo->healths[hitPlayer] == 0) {
							killPlayer(hitPlayer);
						}
					}
					manager->destroyObject(other);
				}

			}
			obj->collisionContext->hit = false;
		}
	}

	if(!processUnbufferedInput(fe)) {
		return false;
	}

	return ret;
}

void GameApplication::killPlayer(int i) {

	Ogre::Vector3 position(3500, 3500, -3500);
	Ogre::Vector3 exile(9999, 9999, 9999);

	// I'm the dead one D':
	if(i == gameInfo->myId && !dead) {
		dead = true;
		player->setPosition(exile);
		manager->getCamera()->setPosition(position);
		yourHealthDisplay->setText("You died )-:");
	} else {
		players[i]->setPosition(exile);
	}
}

// Find the player in the players list and send that index to client
// (objects ids not in sync on server and client)
// Gives the players index in the list of players
int GameApplication::getPlayerIndex(int obj_id){
	for (int i = 0; i < players.size(); ++i){
		if (players[i]->object_id == obj_id){
			return i;
		}
	}
}

bool GameApplication::mouseMoved(const OIS::MouseEvent& me)
{
	if(gameState == GAME_RUNNING) {
		Ogre::Radian yaw(-MOUSE_SENSITIVITY * me.state.X.rel);
		Ogre::Radian pitch(-MOUSE_SENSITIVITY * me.state.Y.rel);

		// FPS Camera controls
		manager->getCamera()->yaw(Ogre::Degree(yaw.valueRadians()), Ogre::Node::TS_WORLD);
		manager->getCamera()->pitch(Ogre::Degree(pitch.valueRadians()), Ogre::Node::TS_LOCAL);
	}

	if(gameState == GAME_NETWORK_SETUP){
		return BaseApplication::mouseMoved(me);
	}
	return true;
}

// Our function for immediate processing
bool GameApplication::processUnbufferedInput(const Ogre::FrameEvent& fe)
{
	/* 	Static variables in a function are allocated for the lifetime of the program.
		Space is allocated only once and value carries through to the next function calls.
		Good for carrying over previous states of functions, and coroutines.
	 */
	static bool leftMouseDownLastFrame = false;
	static bool rightMouseDownLastFrame = false;
	static bool soundButtonDownLastFram = false;
	static bool followBallMode = false;
	static Ogre::Real move = 250;
	bool leftMouseDown = mMouse->getMouseState().buttonDown(OIS::MB_Left);
	bool rightMouseDown = mMouse->getMouseState().buttonDown(OIS::MB_Right);
	bool soundButtonDown = mKeyboard->isKeyDown(OIS::KC_P);

	static float toggleTimer = 0.0;
	toggleTimer -= fe.timeSinceLastFrame;
	if(toggleTimer < 0) {
		toggleTimer = 0.2;
		messageGameInfo();
	}

	// M1 toggle
	if(gameState == GAME_RUNNING && leftMouseDown && !leftMouseDownLastFrame) {
		if(!dead) {
			shootBall();
		}
	}

	// Toggle Sound
	if(soundButtonDown && !soundButtonDownLastFram) {
		manager->soundOn = !manager->soundOn;
	}

	leftMouseDownLastFrame = leftMouseDown;
	rightMouseDownLastFrame = rightMouseDown;
	soundButtonDownLastFram = soundButtonDown;
	toggleTimer -= fe.timeSinceLastFrame;

	// Player movement, accumulate inputs
	Ogre::Vector3 playerVector = Ogre::Vector3::ZERO;
	// Forward
	if(mKeyboard->isKeyDown(OIS::KC_W)) {
		playerVector += mCamera->getRealDirection();
	}
	// Backward
	if(mKeyboard->isKeyDown(OIS::KC_S)) {
		playerVector -=  mCamera->getRealDirection();
	}
	// Left
	if(mKeyboard->isKeyDown(OIS::KC_A)) {
		playerVector -= mCamera->getRealRight();
	}
	// Right
	if(mKeyboard->isKeyDown(OIS::KC_D)) {
		playerVector += mCamera->getRealRight();
	}

	// Camera movement
	Ogre::Vector3 camVector = Ogre::Vector3::ZERO;
	// Forward
	if(mKeyboard->isKeyDown(OIS::KC_UP)) {
		camVector += mCamera->getRealDirection();
	}
	// Backward
	if(mKeyboard->isKeyDown(OIS::KC_DOWN)) {
		camVector -=  mCamera->getRealDirection();
	}
	// Left
	if(mKeyboard->isKeyDown(OIS::KC_LEFT)) {
		camVector -= mCamera->getRealRight();
	}
	// Right
	if(mKeyboard->isKeyDown(OIS::KC_RIGHT)) {
		camVector += mCamera->getRealRight();
	}
	int camVertical = 0;

	playerVector = Ogre::Vector3(playerVector.x, 0, playerVector.z);
	playerVector.normalise();
	playerVector *= CAM_SPEED * 0.5;

	camVector = Ogre::Vector3(camVector.x, camVertical, camVector.z);
	camVector.normalise();
	camVector *= CAM_SPEED;

	// Act on camera inputs
	Ogre::Vector3 newPosition = player->getPosition() + playerVector * fe.timeSinceLastFrame;
	if (dead) {
		manager->getCamera()->translate(camVector * fe.timeSinceLastFrame);	
	}

	if(!dead) {
		// Also move player
		
		player->rigidbody->setLinearVelocity(btVector3(playerVector.x, playerVector.y, playerVector.z));
		player->rigidbody->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));

		Ogre::Vector3 position = player->getPosition();

		if(isServer) {
			GamePacket packet;
			packet.type = PACKET_PLAYER_POS;
			packet.playerIdx = 0;
			packet.playerPos[0] = position.x;
			packet.playerPos[1] = position.y;
			packet.playerPos[2] = position.z;
			if(numClients > 0) {
				netMgr->messageClients(PROTOCOL_TCP, (char *)&packet, sizeof(packet));
			}
		} else {
			GamePacket packet;
			packet.type = PACKET_PLAYER_POS;
			packet.playerPos[0] = position.x;
			packet.playerPos[1] = position.y;
			packet.playerPos[2] = position.z;
			netMgr->messageServer(PROTOCOL_TCP, (char *)&packet, sizeof(packet));
		}
	}
	return true;
}


//---------------------------------------------------------------------------

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
	INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
	int main(int argc, char *argv[])
#endif
	{
		// Create application object
		GameApplication app;

		if(argc > 1){
			if (strcmp(argv[1], "s") == 0){
				app.isServer = true;
			} else if (strcmp(argv[1], "c") == 0){
				app.isServer = false;
				if(argc > 2) {
					app.hostName = argv[2];
				} else {
				}
			}
		}

		try {
			app.go();
		} catch(Ogre::Exception& e)  {
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
			MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
			std::cerr << "An exception has occurred: " <<
				e.getFullDescription().c_str() << std::endl;
#endif
		}

		return 0;
	}

#ifdef __cplusplus
}
#endif

//---------------------------------------------------------------------------
