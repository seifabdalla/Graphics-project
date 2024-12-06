#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <cmath>
#include <iostream>
#include "Car.h"
#include "fmod.hpp"
#include <string>


#define DEG2RAD(a) (a * 0.0174532925)
#define M_PI 3.14159265358979323846
using namespace std;

bool track1Win = false;
bool cutScene1 = false;

FMOD::System* systemsound = nullptr;
FMOD::Sound* backgroundMusic = nullptr;
FMOD::Sound* carMovingSound = nullptr;
FMOD::Sound* carHittingSound = nullptr;
FMOD::Sound* collectibleHittingSound = nullptr;
FMOD::Channel* musicChannel = nullptr;
FMOD::Channel* carMovingChannel = nullptr;
FMOD::Channel* carHittingChannel = nullptr;
FMOD::Channel* collectibleHittingChannel = nullptr;
void initFMOD() {
	FMOD::System_Create(&systemsound);
	systemsound->init(512, FMOD_INIT_NORMAL, 0);
	systemsound->createSound("background.mp3", FMOD_LOOP_NORMAL, 0, &backgroundMusic);
	systemsound->createSound("carMoving.mp3", FMOD_DEFAULT, 0, &carMovingSound);
	systemsound->createSound("car_Hit.mp3", FMOD_DEFAULT, 0, &carHittingSound);
	systemsound->createSound("collectibleHit.mp3", FMOD_DEFAULT, 0, &collectibleHittingSound);
	systemsound->playSound(backgroundMusic, 0, false, &musicChannel);
	musicChannel->setVolume(0.1f);

}
void playcarMovingSound() {
	bool isPlaying = false;
	carMovingChannel->isPlaying(&isPlaying);
	if(!isPlaying)
	systemsound->playSound(carMovingSound, 0, false, &carMovingChannel);

	carMovingChannel->setVolume(0.03);
	

}

void playcarHitSound() {
	bool isPlaying = false;
	carMovingChannel->stop();
	carHittingChannel->isPlaying(&isPlaying);
	if (!isPlaying)
		systemsound->playSound(carHittingSound, 0, false, &carHittingChannel);



}

void playCollectibleSound() {
	bool isPlaying = false;
	//carMovingChannel->stop();
	collectibleHittingChannel->isPlaying(&isPlaying);
	if (!isPlaying)
		systemsound->playSound(collectibleHittingSound, 0, false, &collectibleHittingChannel);

	collectibleHittingChannel->setVolume(1);

}
int track = 1;

bool bolt1=true;bool bolt2 = true;bool bolt3 = true;bool bolt4 = true;bool bolt5 = true;

int WIDTH = 1280;
int HEIGHT = 720;
int score = 0;
float PlayerXPos = 0.0f;
float PlayerZPos = 0.0f;
float PlayerYPos = 2.0f;  // Adjust for ground level
float ObjXPos = 5.0f;
float ObjZPos = 45.0f;
float Obj2XPos = -55.0f;
float Obj2ZPos = 120.0f;
float Obj3XPos = -95.0f;
float Obj3ZPos = 145.0f;

float Obj4XPos = 20;
float Obj4ZPos = 208.5;

float Obj5XPos = 120;
float Obj5ZPos = 223.5;

float Obj6XPos = 160;
float Obj6ZPos = 85;

float Obj7XPos = 180;
float Obj7ZPos = 195;

float Obj8XPos = 100;
float Obj8ZPos = 15;

float Obj9XPos = -85;
float Obj9ZPos = 185;

// Global variables for cutscene animation
float cageYPos = 0.0f;          // Cage's upward movement position
float materRotationAngle = 0.0f; // Mater's rotation angle
float materJumpHeight = 0.0f;   // Mater's vertical jump height
int jumpStage = 0;              // Jump stage (0: up, 1: down)
bool animationComplete = false; // Flag to track if animation has completed
float cutsceneTimer = 0.0f;     // Timer to limit cutscene duration

// Timer interval (milliseconds)
const int TIMER_INTERVAL = 16; // ~60 FPS

// Function to update animation states for the cutscene
void updateCutscene() {
	if (cutScene1 && !animationComplete) {
		// Animate the cage moving upwards
		if (cageYPos < 8.0f) {
			cageYPos += 0.05f; // Incrementally move the cage up
		}

		if (cageYPos >= 8) {// Animate Mater rotating around a point and jumping
			materRotationAngle += 2.0f; // Increment rotation angle
			if (materRotationAngle > 360.0f) {
				materRotationAngle -= 360.0f; // Keep angle within bounds
			}

			// Jump logic: Increment or decrement jump height
			if (jumpStage == 0) {
				materJumpHeight += 0.05f; // Move up
				if (materJumpHeight >= 1.5f) {
					jumpStage = 1; // Switch to falling down
				}
			}
			else if (jumpStage == 1) {
				materJumpHeight -= 0.05f; // Move down
				if (materJumpHeight <= 0.0f) {
					jumpStage = 0; // Switch back to jumping up
				}
			}
		}
		// Update the cutscene timer
		cutsceneTimer += TIMER_INTERVAL / 1000.0f; // Convert milliseconds to seconds
		if (cutsceneTimer >= 10.0f) { // End cutscene after 10 seconds
			animationComplete = true;
		}
	}
	glutPostRedisplay(); // Redraw the scene
}

float boltRotationAngle = 0.0f; // Rotation angle for bolts
const float rotationSpeed = 0.7f; // Adjust rotation speed

void updateBoltRotation() {
	boltRotationAngle += rotationSpeed;
	if (boltRotationAngle >= 360.0f) {
		boltRotationAngle -= 360.0f; // Reset angle to prevent overflow
	}
}

float characterAngle = 0.0f;
float rotationAngle = 0.0f;

float moveSpeed = 1.5f;         // Player movement speed
bool speedBoostActive = false; // Whether the speed boost is active
float speedBoostTimer = 0.0f;  // Timer for speed boost countdown

GLuint tex;
GLuint texSky;
char title[] = "3D Model Loader Sample";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 100;

class Vector
{
public:
	GLdouble x, y, z;
	Vector() {}
	Vector(GLdouble _x, GLdouble _y, GLdouble _z) : x(_x), y(_y), z(_z) {}
	//================================================================================================//
	// Operator Overloading; In C++ you can override the behavior of operators for you class objects. //
	// Here we are overloading the += operator to add a given value to all vector coordinates.        //
	//================================================================================================//
	void operator +=(float value)
	{
		x += value;
		y += value;
		z += value;
	}
};

Vector Eye(20, 5, 20);
Vector At(0, 0, 0);
Vector Up(0, 1, 0);

enum CameraView {
	THIRD,
	FIRST,
	TOP
};


class Vector3f {
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f& v) {
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f& v) {
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n) {
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n) {
		return Vector3f(x / n, y / n, z / n);
	}

	Vector3f unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3f cross(Vector3f v) {
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};


float playerXPos = 0.0f;
float playerZPos = 0.0f;


void updateSpeedBoostTimer(float deltaTime) {
	if (speedBoostActive) {
		
		speedBoostTimer -= deltaTime; // Reduce the timer
		if (speedBoostTimer <= 0.0f) {
			speedBoostActive = false; // Deactivate boost
			moveSpeed -= 1;       // Reset speed
			speedBoostTimer = 0.0f;   // Reset timer
		}
	}
}


class Camera {
public:
	float distanceFromPlayer;
	float pitch;
	float angleAroundPlayer;
	Vector3f playerPosition;

	CameraView currentView;

	Camera(float distance = 10.0f, float pitch = 10.0f, float angle = 0.0f) {
		distanceFromPlayer = distance;
		this->pitch = pitch;
		angleAroundPlayer = angle;
		playerPosition = Vector3f(0.0f, 0.0f, 0.0f);
		initialCameraPosition();
	}
	void initialCameraPosition() {
		eye = playerPosition + Vector3f(0.0f, 5.0f, -distanceFromPlayer); // Adjust these offsets as needed
		center = playerPosition + Vector3f(0.0f, 2.0f, 0.0f); // Looking slightly above the player's position
		up = Vector3f(0.0f, 0.1f, 0.0f);

	}
	void updateCameraPosition(float playerAngle) {
		if (currentView == THIRD) {
			// Camera positioned slightly above and behind the player
			playerPosition = Vector3f(playerXPos, 1.75f, playerZPos);

			// Compute the camera's relative position based on the player's angle
			float offsetX = distanceFromPlayer * sin(playerAngle);
			float offsetZ = distanceFromPlayer * cos(playerAngle);

			eye = playerPosition + Vector3f(-offsetX, 5.0f, -offsetZ); // Slightly above and behind
			center = playerPosition + Vector3f(0.0f, 2.0f, 0.0f); // Looking directly above the player's position
			up = Vector3f(0.0f, 1.0f, 0.0f); // Keep the up direction consistent
		}
		else if (currentView == FIRST) {
			// Camera positioned slightly above and ahead of the player
			playerPosition = Vector3f(playerXPos, 1.75f, playerZPos);

			// Compute the forward offset based on the player's angle
			float offsetX = 4.7f * sin(playerAngle);
			float offsetZ = 4.7f * cos(playerAngle);

			eye = playerPosition + Vector3f(1.0*sin(playerAngle), 1.7f, 1.0f*cos(playerAngle)); // Slightly above the player's position
			center = playerPosition + Vector3f(offsetX, 1.6f, offsetZ); // Looking forward in the player's direction
			up = Vector3f(0.0f, 1.0f, 0.0f); // Keep the up direction consistent
		}
		else if (currentView == TOP) {
			// Camera positioned high above the player, looking straight down
			playerPosition = Vector3f(playerXPos, 1.75f, playerZPos);
			eye = playerPosition + Vector3f(0.0f, 250.0f, 0.0f); // High above the player
			center = playerPosition; // Looking directly at the player
			up = Vector3f(0.0f, 0.0f, 1.0f); // Up vector flipped for a downward view
		}
	}

	void look() {
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z
		);
	}
	Vector3f eye, center, up;
};

Camera camera;
int lastMouseX, lastMouseY;
bool rightMouseDown = false;

void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 640.0 / 480.0, 0.001, 10000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}



void MouseMotion(int x, int y) {
	if (rightMouseDown) {
		float sensitivity = 0.1f;

		// Adjust pitch with right-click + vertical mouse movement
		float deltaY = y - lastMouseY;
		camera.pitch -= deltaY * sensitivity; // Move pitch based on vertical mouse movement

		// Clamp pitch to avoid flipping the camera upside down
		if (camera.pitch > 89.0f) camera.pitch = 89.0f;
		if (camera.pitch < -89.0f) camera.pitch = -89.0f;
	}
	else {
		// Adjust angleAroundPlayer with horizontal mouse movement
		float sensitivity = 0.35f;
		float deltaX = x - lastMouseX;
		camera.angleAroundPlayer += deltaX * sensitivity;
	}

	// Update camera position based on new pitch and angleAroundPlayer
	camera.updateCameraPosition(characterAngle);

	lastMouseX = x;
	lastMouseY = y;

	glutPostRedisplay();
}


bool isColliding(float playerX, float playerZ, float objX, float objZ) {
	// Simple collision detection
	if (playerX >= objX - 2.4 && playerX <= objX + 2.4 && playerZ >= objZ - 2.4 && playerZ <= objZ + 2.4)
		return true;
	else
		return false;
}
bool checkCollision(float playerX, float playerZ, float boltX, float boltZ, float threshold) {
	// Calculate distance between player and bolt
	float deltaX = playerX - boltX;
	float deltaZ = playerZ - boltZ;

	// Check if within collision threshold
	return (deltaX * deltaX + deltaZ * deltaZ) <= (threshold * threshold);
}

void activateSpeedBoost() {
	score += 100;
	moveSpeed +=1 ;          // Increase speed
	speedBoostActive = true;
	// Activate boost
	speedBoostTimer = 2.5f; 
	// Set 2-second timer
	playCollectibleSound();
}

void checkBoltCollisions(float playerX, float playerZ) {
	float collisionThreshold = 5.0f; // Adjust based on the size of the player and bolts

	// Check each bolt individually
	if (bolt1 && checkCollision(playerX, playerZ, 0, 78, collisionThreshold)) {
		bolt1 = false;
		activateSpeedBoost(); // Activate speed boost
		std::cout << "Collision with Bolt 1!" << std::endl;
	}
	if (bolt2 && checkCollision(playerX, playerZ, -15, 224, collisionThreshold)) {
		bolt2 = false;
		activateSpeedBoost();
		std::cout << "Collision with Bolt 2!" << std::endl;
	}
	if (bolt3 && checkCollision(playerX, playerZ, 125, 212, collisionThreshold)) {
		bolt3 = false;
		activateSpeedBoost();
		std::cout << "Collision with Bolt 3!" << std::endl;
	}
	if (bolt4 && checkCollision(playerX, playerZ, 135, 78, collisionThreshold)) {
		bolt4 = false;
		activateSpeedBoost();
		std::cout << "Collision with Bolt 4!" << std::endl;
	}
	if (bolt5 && checkCollision(playerX, playerZ, 178, -62, collisionThreshold)) {
		bolt5 = false;
		activateSpeedBoost();
		std::cout << "Collision with Bolt 5!" << std::endl;
	}
}







void MouseInit(int x, int y) {
	lastMouseX = x;
	lastMouseY = y;
}

void MouseButton(int button, int state, int x, int y) {
	if (button == GLUT_RIGHT_BUTTON) {
		rightMouseDown = (state == GLUT_DOWN);  // Set flag when right mouse button is pressed or released
	}
}

class Boundary {
public:
	float minX, maxX, minZ, maxZ;

	// Default Constructor
	Boundary() : minX(0.0f), maxX(0.0f), minZ(0.0f), maxZ(0.0f) {}

	// Parameterized Constructor
	Boundary(float minX, float maxX, float minZ, float maxZ)
		: minX(minX), maxX(maxX), minZ(minZ), maxZ(maxZ) {}

	// Method to check if a point is within the boundary
	bool contains(float x, float z) const {
		return x >= minX && x <= maxX && z >= minZ && z <= maxZ;
	}
};

// Individual boundary instances
Boundary track1(-7.9f, 12.8f, 0.0f, 125.0f);    // Track 1
Boundary uTurn12(-19.0f, -3.5f, 96.0f, 135.0f);  // U-turn 12
Boundary track2(-97.0f, -2.0f, 114.0f, 137.0f); // Track 2
Boundary track3(-101.0f, -70.0f, 128.0f, 202.0f); // Track 3
Boundary uTurn34(-101.0f, -67.0f, 198.0f, 220.0f); // U-turn 34
Boundary track4(-68.0f, 157.0f, 207.5f, 231.0f);// Track 4
Boundary uTurn23(0,0,0,0);
Boundary trackj1(146.0,187.0,200.0f,225.0f);// Track 4
Boundary roadj1(160.0, 187.95, 80.9, 206.0);
Boundary trackj2(158.0f, 180.83f, 69.9521f, 85.0f);// Track 4
Boundary roadj2(90.0f,159.0f,63.669f,85.7f);// Track 4
///////////////////////////////////////////////////////////////
Boundary trackj3(74.0f, 97.0f,60.0f, 82.0f);// Track 4
Boundary roadj3(69.6f,90.0f,30.0f,70.0f);// Track 4
//////////////////////////////////
Boundary trackj4(68.0f,90.0f,12.0f,45.0f);// Track 4
Boundary roadj4(86.0f, 172.0f, 0.0f, 35.0f);// Track 4
/////////////////////////////////////////////////////////////
Boundary trackj5(161.294f,179.294f, -62.6447f, -46.6447f);// Track 4
Boundary roadj5(162.7f,180.7f,-46.6f,9.5f);// Track 4
Boundary trackj6(163.818f,178.18f, -68.6447f, -46.6447f);// Track 4
/////////////////////////////////////////
Boundary roadj6(175.0f,230.f,-80.0f,-59.32f);// Track 4
Boundary trackj7(214.0f,239.0f,-83.2103,-63.2103);// Track 4
Boundary roadj7(222.0f,244.5f,-216.0f,-83.0f);// Track 4
Boundary finish1(210,260,-150,-140);


bool isWithinBoundaries(float x, float z, const std::string& type) {
	static int currentTrack = 1; // Default to track 1

	// Check for each boundary individually
	
	if (track1.contains(x, z)) {
		currentTrack = 1;
		std::cout << "Track: " << currentTrack << ", X: " << x << ", Z: " << z << std::endl;
		return true;
	}
	else if (uTurn12.contains(x, z)) {
		std::cout << "Car is in U-turn 1!" << std::endl;
		return true;
	}
	else if (track2.contains(x, z)) {
		currentTrack = 2;
		std::cout << "Track: " << currentTrack << ", X: " << x << ", Z: " << z << std::endl;
		return true;
	}
	else if (uTurn23.contains(x, z)) {
		std::cout << "Car is in U-turn 2!" << std::endl;
		return true;
	}
	else if (track3.contains(x, z)) {
		currentTrack = 3;
		std::cout << "Track: " << currentTrack << ", X: " << x << ", Z: " << z << std::endl;
		return true;
	}
	else if (uTurn34.contains(x, z)) {
		std::cout << "Car is in U-turn 3!" << std::endl;
		return true;
	}
	else if (track4.contains(x, z)) {
		currentTrack = 4;
		std::cout << "Track: " << currentTrack << ", X: " << x << ", Z: " << z << std::endl;
		return true;
	}
	else if (trackj1.contains(x, z)) {
		std::cout << "Car is in U-turn 3!" << std::endl;
		return true;
	}
	else if (roadj1.contains(x, z)) {
		currentTrack = 5;
		std::cout << "Track: " << currentTrack << ", X: " << x << ", Z: " << z << std::endl;
		return true;
	}
	else if (trackj2.contains(x, z)) {
		std::cout << "Car is in U-turn 3!" << std::endl;
		return true;
	}
	else if (roadj2.contains(x, z)) {
		currentTrack = 6;
		std::cout << "Track: " << currentTrack << ", X: " << x << ", Z: " << z << std::endl;
		return true;
	}
	else if (trackj3.contains(x, z)) {
		std::cout << "Car is in U-turn 3!" << std::endl;
		return true;
	}
	else if (roadj3.contains(x, z)) {
		currentTrack = 7;
		std::cout << "Track: " << currentTrack << ", X: " << x << ", Z: " << z << std::endl;
		return true;
	}
	else if (trackj4.contains(x, z)) {
		std::cout << "Car is in U-turn 3!" << std::endl;
		return true;
	}
	else if (roadj4.contains(x, z)) {
		currentTrack = 8;
		std::cout << "Track: " << currentTrack << ", X: " << x << ", Z: " << z << std::endl;
		return true;
	}
	else if (trackj5.contains(x, z)) {
		std::cout << "Car is in U-turn 3!" << std::endl;
		return true;
	}
	else if (roadj5.contains(x, z)) {
		currentTrack = 9;
		std::cout << "Track: " << currentTrack << ", X: " << x << ", Z: " << z << std::endl;
		return true;
	}
	else if (trackj6.contains(x, z)) {
		std::cout << "Car is in U-turn 3!" << std::endl;
		return true;
	}
	else if (roadj6.contains(x, z)) {
		currentTrack = 10;
		std::cout << "Track: " << currentTrack << ", X: " << x << ", Z: " << z << std::endl;
		return true;
	}
	else if (trackj7.contains(x, z)) {
		std::cout << "Car is in U-turn 3!" << std::endl;
		return true;
	}
	else if (roadj7.contains(x, z)) {
		currentTrack = 11;
		std::cout << "Track: " << currentTrack << ", X: " << x << ", Z: " << z << std::endl;
		return true;
	}
	// If no boundary matches, the position is out of bounds
	std::cout << "False Track: " << currentTrack << ", X: " << x << ", Z: " << z << std::endl;
	return false;
}



bool canMove(float x, float z, float angle, float speed, bool isForward) {
	return true;
	float carLength = 3.0f;
	float carWidth = 1.7f;

	// Half dimensions
	float halfLength = carLength / 2.0f;
	float halfWidth = carWidth / 2.0f;

	// Direction multiplier (forward or backward)
	float direction = isForward ? 1.0f : -1.0f;

	// Calculate new position
	float newX = x + direction * speed * sin(angle);
	float newZ = z + direction * speed * cos(angle);

	// Rotation matrix for corners
	float sinAngle = sin(angle);
	float cosAngle = cos(angle);

	// Corner offsets relative to the center (unrotated)
	float offsets[4][2] = {
		{-halfLength, -halfWidth},  // Back-right
		{ halfLength, -halfWidth},  // Front-right
		{-halfLength,  halfWidth},  // Back-left
		{ halfLength,  halfWidth}   // Front-left
	};

	// Calculate rotated corners
	for (int i = 0; i < 4; i++) {
		float cornerX = newX + (offsets[i][0] * cosAngle - offsets[i][1] * sinAngle);
		float cornerZ = newZ + (offsets[i][0] * sinAngle + offsets[i][1] * cosAngle);

		// Check boundary
		if (!isWithinBoundaries(cornerX, cornerZ, "") || isColliding(cornerX,cornerZ,ObjXPos,ObjZPos) || isColliding(cornerX,cornerZ,Obj2XPos,Obj2ZPos ) || isColliding(cornerX, cornerZ, Obj3XPos, Obj3ZPos)
			|| isColliding(cornerX, cornerZ, Obj4XPos, Obj4ZPos) || isColliding(cornerX, cornerZ, Obj5XPos, Obj5ZPos) || isColliding(cornerX, cornerZ, Obj6XPos, Obj6ZPos)
			|| isColliding(cornerX, cornerZ, Obj7XPos, Obj7ZPos) || isColliding(cornerX, cornerZ, Obj8XPos, Obj8ZPos) || isColliding(cornerX, cornerZ, Obj9XPos, Obj9ZPos)) {
			playcarHitSound();
			return false; // One corner is out of bounds
		}



	}
	   
	playcarMovingSound();
	return true; // All corners are within bounds
}
void moveFront(float x, float z, float angle, float speed) {
	if (canMove(x, z, angle, speed, true)) {
		playerXPos += speed * sin(angle);
		playerZPos += speed * cos(angle);
	}
	else {

		playerXPos += -1*sin(angle);
		playerZPos += -1*cos(angle);
	}
}

void moveBackward(float x, float z, float angle, float speed) {
	if (canMove(x, z, angle, speed, false)) {
		playerXPos -= speed * sin(angle);
		playerZPos -= speed * cos(angle);
	}
	else {

		playerXPos -= -1 * sin(angle);
		playerZPos -= -1 * cos(angle);
	}
}
	

void checkWin() {
	if (finish1.contains(playerXPos, playerZPos)) {
		cutScene1 = true;
		track1Win = true;
		std::cout << "Car reached finish line 1" << std::endl;
	}
}


void rotateRight(float& angle, float speed, float rotationSpeed) {
	float newAngle = angle - rotationSpeed;
	if (newAngle < 0) newAngle += 2 * M_PI;

	// Predict the new position
	float newX = playerXPos + speed * sin(newAngle);
	float newZ = playerZPos + speed * cos(newAngle);

	// Check if the new position is within boundaries
	if (canMove(playerXPos, playerZPos, newAngle, speed, true)) {
		angle = newAngle;  // Update the angle
		playerXPos = newX; // Update the position
		playerZPos = newZ;
	}
	else {
		playerXPos += -1 * sin(angle);
		playerZPos += -1 * cos(angle);
	}
}
void rotateLeft(float& angle, float speed, float rotationSpeed) {
	float newAngle = angle + rotationSpeed;
	if (newAngle >= 2 * M_PI) newAngle -= 2 * M_PI;

	// Predict the new position
	float newX = playerXPos + speed * sin(newAngle);
	float newZ = playerZPos + speed * cos(newAngle);

	// Check if the new position is within boundaries
	if (canMove(playerXPos, playerZPos, newAngle, speed, true) ){
		angle = newAngle;  // Update the angle
		playerXPos = newX; // Update the position
		playerZPos = newZ;
	}
	else {
		playerXPos += -1 * sin(angle);
		playerZPos += -1 * cos(angle);
	}
}
void keyboard(unsigned char key, int x, int y) {
	float speed = moveSpeed;                  // Movement speed
	float rotationSpeed = 5.0f * (M_PI / 180.0f); // Convert degrees to radians for rotation

	if (key == 's' && !cutScene1) {
		moveBackward(playerXPos, playerZPos, characterAngle, speed);
	}
	else if (key == 'w' && !cutScene1) {
		moveFront(playerXPos, playerZPos, characterAngle, speed);
	}
	else if (key == 'd' && !cutScene1) {
		rotateRight(characterAngle, speed, rotationSpeed);
	}
	else if (key == 'a' && !cutScene1) {
		rotateLeft(characterAngle, speed, rotationSpeed);
	}
	else if (key == 't')
		camera.currentView = THIRD;
	else if (key == 'f')
		camera.currentView = FIRST;
	else if (key == 'y')
		camera.currentView = TOP;

	// Update camera and redraw scene
	camera.playerPosition = Vector3f(playerXPos, PlayerYPos, playerZPos);
	camera.updateCameraPosition(characterAngle);
	glutPostRedisplay();
	std::cout << "Player's X Position: " << playerXPos << std::endl;
	std::cout << "Player's Z Position: " << playerZPos << std::endl;
	std::cout << "Character Angle: " << characterAngle << std::endl;
}








// Model Variables
Model_3DS model_car;
Model_3DS model_obstacle;
Model_3DS model_track;
Model_3DS model_cage;
Model_3DS model_matar;
Model_3DS model_bolt;
Model_3DS model_stadium;
Model_3DS model_tunnle;
Car car(model_car, 0, 0, 0);

// Textures
GLTexture tex_obs;

//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(fovy, aspectRatio, zNear, zFar);
	//*******************************************************************************************//
	// fovy:			Angle between the bottom and top of the projectors, in degrees.			 //
	// aspectRatio:		Ratio of width to height of the clipping plane.							 //
	// zNear and zFar:	Specify the front and back clipping planes distances from camera.		 //
	//*******************************************************************************************//

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	//*******************************************************************************************//
	// EYE (ex, ey, ez): defines the location of the camera.									 //
	// AT (ax, ay, az):	 denotes the direction where the camera is aiming at.					 //
	// UP (ux, uy, uz):  denotes the upward orientation of the camera.							 //
	//*******************************************************************************************//

	InitLightSource();

	InitMaterial();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
}

//=======================================================================
// Render Ground Function
//=======================================================================

//=======================================================================
// Display Function
//=======================================================================
float timeOfDay = 0.0f;  // Value from 0.0 (night) to 4.0 (day)
float transitionSpeed = 0.001f;  // Speed of transition (adjust this value)
bool isDay = true;  // Track if it's day or night

// Function to interpolate between two values
float lerp(float start, float end, float t) {
	float smoothedT = (1 - cos(t * M_PI)) / 2; // Smoothstep-like function
	return start + smoothedT * (end - start);
}

// Function to update the time of day and the cycle behavior
void UpdateTimeOfDay() {
	timeOfDay += transitionSpeed;
	if (timeOfDay > 4.0f) {
		timeOfDay = 0.0f;  // Reset to night when it reaches day
	}
}

void setupHeadlights() {
	// Enable lighting for headlights 
	glEnable(GL_LIGHTING);

	// Calculate headlight positions based on car's position and angle 
	float offsetX = 1.0f * sin(characterAngle);
	float offsetZ = 1.0f * cos(characterAngle);

	// Left headlight 
	GLfloat headlightPosition1[4] = {
		playerXPos +offsetX ,  // Lateral offset 
		1.4f,  // Headlight height 
		playerZPos +offsetZ,
		1.0f   // Positional light 
	};

	// Right headlight 
	GLfloat headlightPosition2[4] = {
		playerXPos + offsetX,  // Lateral offset 
		1.4f,  // Headlight height 
		playerZPos + offsetZ,
		1.0f   // Positional light 
	};

	// Headlight ambient - slightly dimmer to enhance contrast
	GLfloat headlightAmbient[4] = { 0.1f, 0.1f, 0.1f, 1.0f };  // Dimmer ambient light

	GLfloat headlightDiffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };  // Reduced intensity
	GLfloat headlightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };  // Reduced specular light



	// Configure first headlight 
	glLightfv(GL_LIGHT1, GL_POSITION, headlightPosition1);
	glLightfv(GL_LIGHT1, GL_AMBIENT, headlightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, headlightDiffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, headlightSpecular);

	// Spotlight parameters for more focused, intense light 
	GLfloat spotDirection1[3] = { sin(characterAngle), -0.1f, cos(characterAngle) };
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 20.0f);  // Slightly wider beam 
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDirection1);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 5.0f);  // Much higher intensity falloff 
	glEnable(GL_LIGHT1);



	// Same configuration for second headlight 
	glLightfv(GL_LIGHT2, GL_POSITION, headlightPosition2);
	glLightfv(GL_LIGHT2, GL_AMBIENT, headlightAmbient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, headlightDiffuse);
	glLightfv(GL_LIGHT2, GL_SPECULAR, headlightSpecular);

	GLfloat spotDirection2[3] = { sin(characterAngle), -0.1f, cos(characterAngle) };
	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 20.0f);  // Slightly wider beam 
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spotDirection2);
	glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 5.0f);  // Much higher intensity falloff 
	glEnable(GL_LIGHT2);
	
}



// Function to update the time of day and the cycle behavior
void SetupLight() {
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Light position (simulating sunlight direction)
	GLfloat lightPosition[] = { 0.0f, 1.0f, 0.0f, 0.0f }; // Directional light
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	// Initialize light properties
	GLfloat ambientLight[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat diffuseLight[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat specularLight[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	float t = timeOfDay / 4.0f;  // Normalize time to [0, 1]
	std::cout << t << endl;
	// Smooth day-night transitions using phases
	if (timeOfDay >= 0.0f && timeOfDay <= 1.0f) {
		// Dawn (night to day transition)
		ambientLight[0] = lerp(0.05f, 0.4f, t*5);  // Low red
		ambientLight[1] = lerp(0.05f, 0.5f, t * 5);  // Moderate green
		ambientLight[2] = lerp(0.1f, 0.8f, t * 5);   // Strong blue
		diffuseLight[0] = lerp(0.1f, 0.8f, t * 5);
		diffuseLight[1] = lerp(0.1f, 0.9f, t * 5);
		diffuseLight[2] = lerp(0.2f, 1.0f, t * 5);
	}
	else if (timeOfDay > 1.0f && timeOfDay <= 2.0f) {
		// Daytime (full brightness)
		float t2 = (timeOfDay - 1.0f) / 1.0f;
		ambientLight[0] = lerp(0.4f, 0.5f, t2);  // Balanced lighting
		ambientLight[1] = lerp(0.5f, 0.5f, t2);
		ambientLight[2] = lerp(0.8f, 0.6f, t2);
		diffuseLight[0] = 1.0f;
		diffuseLight[1] = 1.0f;
		diffuseLight[2] = 0.9f;
	}
	else if (timeOfDay > 2.0f && timeOfDay <= 3.0f) {
		// Dusk (day to night transition)
		float t3 = (timeOfDay - 2.0f) / 1.0f;
		ambientLight[0] = lerp(0.5f, 0.1f, t3);
		ambientLight[1] = lerp(0.5f, 0.05f, t3);
		ambientLight[2] = lerp(0.6f, 0.2f, t3);
		diffuseLight[0] = lerp(1.0f, 0.3f, t3);
		diffuseLight[1] = lerp(0.9f, 0.2f, t3);
		diffuseLight[2] = lerp(0.8f, 0.1f, t3);
	}
	else if (timeOfDay > 3.0f && timeOfDay <= 4.0f) {
		// Nighttime (dim light)
		float t4 = (timeOfDay - 3.0f) / 1.0f;
		ambientLight[0] = lerp(0.1f, 0.05f, t4);
		ambientLight[1] = lerp(0.05f, 0.05f, t4);
		ambientLight[2] = lerp(0.2f, 0.1f, t4);
		diffuseLight[0] = lerp(0.3f, 0.1f, t4);
		diffuseLight[1] = lerp(0.2f, 0.05f, t4);
		diffuseLight[2] = lerp(0.1f, 0.05f, t4);
	}

	if ((timeOfDay > 1.0f && timeOfDay <= 4.0f)||!isDay)
	{
		setupHeadlights();
	}
	else {
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);

	}

	// Apply lighting properties
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
}


void renderScore() {
	// Switch to orthographic projection
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, 1280, 0, 720);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);

	// Render score
	glColor3f(1.0f, 1.0f, 1.0f);
	glRasterPos2f(50, 650); // Adjust for your screen resolution
	std::string scoreText = "Score: " + std::to_string(score);
	for (char c : scoreText) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
	}

	glEnable(GL_DEPTH_TEST);

	// Restore matrices
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}




float blendFactor;
float darknessFactor = 1.0f;  // 1.0 = fully bright, 0.0 = fully dark           // Tracks if it’s day or night
int frameCounter = 0;        // Frame counter to manage timing
const int totalCycleTime = 600; // Total time for one full day-night cycle (in frames)
const int holdTime = 100;
void RenderSkybox(float blendFactor) {
	glPushMatrix();
	//glTranslatef(camera.playerPosition.x, camera.playerPosition.y, camera.playerPosition.z);  // Center skybox around camera
	glScalef(50, 50, 50);  // Increase the size of the skybox to fully cover the camera view

	// Enable texture mapping
	glEnable(GL_TEXTURE_2D);
	if (timeOfDay > 2.0f) {
		// Nighttime (use night skybox texture)
		glBindTexture(GL_TEXTURE_2D, tex);
	}
	else {
		// Daytime (use day skybox texture)
		glBindTexture(GL_TEXTURE_2D, texSky);
	}

	// Draw the skybox cube
	glBegin(GL_QUADS);

	// Front face
	glTexCoord2f(0.0, 0.0); glVertex3f(-100.0, -100.0, 100.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(100.0, -100.0, 100.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(100.0, 100.0, 100.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-100.0, 100.0, 100.0);

	// Back face
	glTexCoord2f(0.0, 0.0); glVertex3f(-100.0, -100.0, -100.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(100.0, -100.0, -100.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(100.0, 100.0, -100.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-100.0, 100.0, -100.0);

	// Top face
	glTexCoord2f(0.0, 0.0); glVertex3f(-100.0, 100.0, -100.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(100.0, 100.0, -100.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(100.0, 100.0, 100.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-100.0, 100.0, 100.0);

	// Bottom face
	glTexCoord2f(0.0, 0.0); glVertex3f(-100.0, -100.0, -100.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(100.0, -100.0, -100.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(100.0, -100.0, 100.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-100.0, -100.0, 100.0);

	// Right face
	glTexCoord2f(0.0, 0.0); glVertex3f(100.0, -100.0, -100.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(100.0, 100.0, -100.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(100.0, 100.0, 100.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(100.0, -100.0, 100.0);

	// Left face
	glTexCoord2f(0.0, 0.0); glVertex3f(-100.0, -100.0, -100.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(-100.0, 100.0, -100.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(-100.0, 100.0, 100.0);
	glTexCoord2f(0.0, 1.0); glVertex3f(-100.0, -100.0, 100.0);

	glEnd();

	glDisable(GL_TEXTURE_2D);

	glPopMatrix();


}

void myDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	SetupLight();
	setupCamera();



	glPushMatrix();
	glTranslatef(playerXPos, PlayerYPos, playerZPos);// Position the car
	glRotatef(characterAngle * 180.0f / M_PI, 0.0f, 1.0f, 0.0f);

	//glScalef(0.02f, 0.02f, 0.02f); // Adjust the scale of the car if needed

	car.render();// Draw the car model

	glPopMatrix();

	

	if (bolt1) {
	glPushMatrix();
	glTranslatef(0, 4.2, 78);
	glScalef(0.01, 0.01, 0.01);
	glRotated(boltRotationAngle, 0, 1, 0);
	model_bolt.Draw();
	glPopMatrix();
}

	if (bolt2) {

		glPushMatrix();
		glTranslatef(-15, 4.2, 224);
		glScalef(0.01, 0.01, 0.01);
		glRotated(boltRotationAngle, 0, 1, 0);
		model_bolt.Draw();
		glPopMatrix();
	}

	if (bolt3) {
		glPushMatrix();
		glTranslatef(125, 4.2, 212);
		glScalef(0.01, 0.01, 0.01);
		glRotated(boltRotationAngle, 0, 1, 0);
		model_bolt.Draw();
		glPopMatrix();
	}
	


	if (bolt4) {
		glPushMatrix();
		glTranslatef(135, 4.2, 78);
		glScalef(0.01, 0.01, 0.01);
		glRotated(boltRotationAngle, 0, 1, 0);
		model_bolt.Draw();
		glPopMatrix();
	}

	if (bolt5) {
		glPushMatrix();
		glTranslatef(178, 4.2, -62);
		glScalef(0.01, 0.01, 0.01);
		glRotated(boltRotationAngle, 0, 1, 0);
		model_bolt.Draw();
		glPopMatrix();
	}

	glPushMatrix();
	glTranslatef(ObjXPos, 1.5, ObjZPos);
	glRotated(90, 0, 1, 0);
	glScalef(0.14f, 0.14f, 0.14f);
	model_obstacle.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(Obj2XPos, 1.5, Obj2ZPos);
	glScalef(0.14f, 0.14f, 0.14f);
	model_obstacle.Draw();
	glPopMatrix();
	
	glPushMatrix();
	glTranslatef(Obj3XPos, 1.5, Obj3ZPos);
	glRotated(90, 0, 1, 0);
	glScalef(0.14f, 0.14f, 0.14f);
	model_obstacle.Draw();
	glPopMatrix();
	


	glPushMatrix();
	glTranslatef(Obj4XPos, 1.5, Obj4ZPos);
	glScalef(0.14f, 0.14f, 0.14f);
	model_obstacle.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(Obj5XPos, 1.5, Obj5ZPos);
	glScalef(0.14f, 0.14f, 0.14f);
	model_obstacle.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(Obj6XPos, 1.5, Obj6ZPos);
	glScalef(0.14f, 0.14f, 0.14f);
	model_obstacle.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(Obj7XPos, 1.5, Obj7ZPos);
	glRotated(90, 0, 1, 0);
	glScalef(0.14f, 0.14f, 0.14f);
	model_obstacle.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(Obj8XPos, 1.5, Obj8ZPos);
	glScalef(0.14f, 0.14f, 0.14f);
	model_obstacle.Draw();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(Obj9XPos, 1.5, Obj9ZPos);
	glRotated(90, 0, 1, 0);
	glScalef(0.14f, 0.14f, 0.14f);
	model_obstacle.Draw();
	glPopMatrix();

	

	if (!cutScene1) {
		// Draw cage and Mater in their initial positions
		glPushMatrix();
		glTranslated(230, 2, -160);
		glScaled(0.04, 0.04, 0.04);
		model_matar.Draw();
		glPopMatrix();

		glPushMatrix();
		glTranslated(230, 2, -160);
		glScaled(0.45, 0.45, 0.45);
		model_cage.Draw();
		glPopMatrix();
	}
	else {
		// Animate the cage moving upward
		glPushMatrix();
		glTranslated(230, 2 + cageYPos, -160); // Apply upward translation
		glScaled(0.45, 0.45, 0.45);
		model_cage.Draw();
		glPopMatrix();

		// Animate Mater rotating and jumping
		glPushMatrix();
		glTranslated(230 + 5 * cos(materRotationAngle * M_PI / 180.0),
			2 + materJumpHeight,
			-160 + 5 * sin(materRotationAngle * M_PI / 180.0)); // Rotate and jump
		glScaled(0.04, 0.04, 0.04);
		model_matar.Draw();
		glPopMatrix();
	}

	glPushMatrix();
	glTranslatef(-55.0, 0.0, -40.0);

	glScalef(0.02f, 0.02f, 0.02f);
	//model_track.Draw();
	glPopMatrix();

	glPushMatrix();
	model_tunnle.Draw();
	glPopMatrix();




	RenderSkybox(blendFactor);
	renderScore();
	glutSwapBuffers();
}
    // Time to hold textures (in frames)
void UpdateDayNightCycle() {
	// Increment frame counter
	frameCounter++;

	// Calculate the current phase of the cycle
	int transitionTime = (totalCycleTime - 2 * holdTime) / 2;

	if (frameCounter <= transitionTime) {
		// Day-to-night transition: Gradually darken
		darknessFactor = 1.0f - (float)frameCounter / transitionTime;
	}
	else if (frameCounter <= transitionTime + holdTime) {
		// Hold night state: Fully dark
		darknessFactor = 0.0f;
		isDay = false;
	}
	else if (frameCounter <= 2 * transitionTime + holdTime) {
		// Night-to-day transition: Gradually brighten
		darknessFactor = (float)(frameCounter - transitionTime - holdTime) / transitionTime;
	}
	else if (frameCounter <= 2 * transitionTime + 2 * holdTime) {
		// Hold day state: Fully bright
		darknessFactor = 1.0f;
		isDay = true;
	}
	else {
		// Reset cycle
		frameCounter = 0;
	}
}

void updateFMOD() {
	systemsound->update();
}

void idleFunction() {
	float deltaTime = 0.016f; // Assume ~60 FPS (16ms per frame)
	updateFMOD();
	UpdateDayNightCycle();
	UpdateTimeOfDay();
	  // Outputs 0.0 to 1.0
	updateBoltRotation();                 // Rotate the bolts
	checkBoltCollisions(playerXPos, playerZPos); // Check collisions
	updateSpeedBoostTimer(deltaTime);     // Update the speed boost timer
	checkWin();
	updateCutscene();
	glutPostRedisplay();                  // Trigger a redisplay
}


//=======================================================================
// Keyboard Function
//=======================================================================
void myKeyboard(unsigned char button, int x, int y)
{
	switch (button)
	{
	case 'w':
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case 'r':
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 27:
		exit(0);
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

//=======================================================================
// Motion Function
//=======================================================================


int cameraZoom = 0;


void myMotion(int x, int y)
{
	y = HEIGHT - y;

	if (cameraZoom - y > 0)
	{
		Eye.x += -0.1;
		Eye.z += -0.1;
	}
	else
	{
		Eye.x += 0.1;
		Eye.z += 0.1;
	}

	cameraZoom = y;

	glLoadIdentity();	//Clear Model_View Matrix

	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);	//Setup Camera with modified paramters

	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glutPostRedisplay();	//Re-draw scene 
}

//=======================================================================
// Mouse Function
//=======================================================================
void myMouse(int button, int state, int x, int y)
{
	y = HEIGHT - y;

	if (state == GLUT_DOWN)
	{
		cameraZoom = y;
	}
}


//=======================================================================
// Reshape Function
//=======================================================================
void myReshape(int w, int h)
{
	if (h == 0) {
		h = 1;
	}

	WIDTH = w;
	HEIGHT = h;

	// set the drawable region of the window
	glViewport(0, 0, w, h);

	// set up the projection matrix 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLdouble)WIDTH / (GLdouble)HEIGHT, zNear, zFar);

	// go back to modelview matrix so we can move the objects about
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
	FILE* bin3ds = fopen("Models/collectible/bolt.3ds", "rb");
	if (!bin3ds) {
		perror("Failed to open file");
		exit(EXIT_FAILURE);
	}

	// Loading Model files
	car.model.Load("Models/car/car/queen.3ds");
	model_obstacle.Load("Models/obstacle/obstacle.3ds");
	model_track.Load("Track/source/desert.3ds");
	model_stadium.Load("Track/source/stadium.3ds");
	model_bolt.Load("Models/collectible/bolt.3ds");
	model_tunnle.Load("Track/source/tunnle.3ds");

	model_matar.Load("Models/Matar/mater.3DS");
	model_cage.Load("Track/source/cage.3ds");
	// 
	// Loading texture files
	loadBMP(&tex, "Textures/nightSkybmp.bmp", true);
	loadBMP(&texSky, "Textures/SkySun.bmp", true);

}

//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{


	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(WIDTH, HEIGHT);

	glutInitWindowPosition(100, 150);

	glutCreateWindow(title);
	initFMOD();

	glutDisplayFunc(myDisplay);

	glutKeyboardFunc(myKeyboard);

	glutKeyboardFunc(keyboard);

	glutMotionFunc(myMotion);

	glutMouseFunc(myMouse);

	glutReshapeFunc(myReshape);
	glutIdleFunc(idleFunction); // Set idle function for smooth updates


	myInit();

	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	glutMainLoop();
}