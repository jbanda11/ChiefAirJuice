# Assignment 3: Networking Extension

Team Ac!d
Javier Banda, Mindy Chi, Benjamin Johnson

# Ideas

* Game design/theme:
	- You are a planet swatting comets back and forth?
	- Character is just a sphere, with lots of comets coming toward you
	- Swat them to stop em
	- Mario Tennis was really cool

# Systems

* Object hierarchy:
	- Parental hierarchy of GameObjects
	- Everything is childed to a root node
* Overall-level
	- Active camera
	- Scene
	- Sound
* Game Object:
	- Transform (Position, Rotation, Scale)
	- Name
	- Parent
	- Children
	- Components
* Components:
	- MeshRenderer
	- Rigidbody
	- Camera

* Canned GameObjects:
	- Plane
	- Sphere

# Design Doc updates for project 3
To network our game, we’ll have one machine be the server while the other machines are clients. The clients and server will be connect and disconnect by TCP connections and normal game info will be sent by UDP. We plan to have clients and servers each have the game and simulation running. Clients will constantly send information about their position and if they’ve fired their weapon. We will have the server send updates of other players to the clients, as well as double check if client info is feasible else it will resolve issues and update clients including rollbacks if the client sends information that is way off from the server. For example, hits of a projectile on a player will not be confirmed unless the server is also close to seeing a collision.

What follows is a breakdown of out detailed design. Ben and I will focus on the networking while pair programming so as to make sure that the important parts are bug-free. Mindy will also work more on game logic and player input. Ben will work on the stretch goal of extending collision code. Javier will focus on tweaking the GUI.

# Work and division of labor:
	* Game: Players can rotate with mouse FPS style/over-the-shoulder
		- Shoot balls to damage each other, Balls destroyed on collision
		- Separate health bars and death conditions
		- Destroy GameObject on contact
	* Fixing collisions
		- Recognize which object collided with
		- Too many collisions (contact)-- maybe scale down world will fix
	* Network server and client
		- Initialize server and host socket data using slide 3
		- Use pseudo code on slide 4 to have main server loop
	* Sending packets with information to sync game states
	* GUI tweaks
		- Create lobby system for host and client pre-game set up
		- All player HP/names listed
		- Class that tracks names/HPs
