#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <cmath>
#include <iostream>
#include "Car.h"

#define DEG2RAD(a) (a * 0.0174532925)
#define M_PI 3.14159265358979323846
using namespace std;

int track = 1;

int WIDTH = 1280;
int HEIGHT = 720;
float PlayerXPos = 0.0f;
float PlayerZPos = 0.0f;
float PlayerYPos = 2.0f;  // Adjust for ground level
float ObjXPos = 5.0f;
float ObjZPos = 45.0f;
float Obj2XPos = -55.0f;
float Obj2ZPos = 120.0f;
float Obj3XPos = -95.0f;
float Obj3ZPos = 145.0f;

float characterAngle = 0.0f;
float rotationAngle = 0.0f;
float moveSpeed = 0.1f;
GLuint tex;
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

			eye = playerPosition + Vector3f(0.0f, 1.6f, 0.0f); // Slightly above the player's position
			center = playerPosition + Vector3f(offsetX, 1.6f, offsetZ); // Looking forward in the player's direction
			up = Vector3f(0.0f, 1.0f, 0.0f); // Keep the up direction consistent
		}
		else if (currentView == TOP) {
			// Camera positioned high above the player, looking straight down
			playerPosition = Vector3f(playerXPos, 1.75f, playerZPos);
			eye = playerPosition + Vector3f(0.0f, 100.0f, 0.0f); // High above the player
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
	if (playerX >= objX - 3 && playerX <= objX + 3 && playerZ >= objZ - 3 && playerZ <= objZ + 3)
		return true;
	else
		return false;
}



bool updatePlayerPosition(float playerX, float playerZ, float objX, float objZ) {
	if (!isColliding(playerX, playerZ, objX, objZ)) {
		// Allow movement if no collision
		return true;
	}
	else {
		return false;
		// Prevent movement on collision
		std::cout << "Collision detected: Player cannot move. Current Position: ("
			<< playerX << ", " << playerZ << ")" << std::endl;
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
Boundary track1(-7.9f, 10.8f, 0.0f, 123.0f);    // Track 1
Boundary uTurn12(-17.0f, -7.5f, 98.0f, 123.0f);  // U-turn 12
Boundary track2(-95.0f, -4.0f, 116.0f, 135.0f); // Track 2
Boundary uTurn23(-92.0f, -74.0f, 0.0f, 0.0f);    // U-turn 23 (placeholders)
Boundary track3(-99.0f, -79.0f, 130.0f, 200.0f); // Track 3
Boundary uTurn34(-99.0f, -64.0f, 200.0f, 220.0f); // U-turn 34
Boundary track4(-64.0f, 153.0f, 207.5f, 235.0f);// Track 4







Boundary trackj1(150.0,183.0,203.0f,223.0f);// Track 4

Boundary roadj1(165.0, 187.95, 82.9, 204.0);

Boundary trackj2(160.0f, 182.83f, 71.9521f, 83.0f);// Track 4

Boundary roadj2(90.0f,163.0f,63.669f,85.7f);// Track 4


///////////////////////////////////////////////////////////////




Boundary trackj3(74.0f, 97.0f,60.0f, 82.0f);// Track 4

Boundary roadj3(69.6f,90.0f,30.0f,70.0f);// Track 4






//////////////////////////////////





Boundary trackj4(70.0f,86.0f,15.0f,34.0f);// Track 4


Boundary roadj4(86.0f, 172.0f, 7.0f, 29.0f);// Track 4
/////////////////////////////////////////////////////////////



Boundary trackj5(164.294f,179.294f, -68.6447f, -46.6447f);// Track 4

Boundary roadj5(162.7f,180.7f,-46.6f,9.5f);// Track 4

Boundary trackj6(163.818f,178.18f, -68.6447f, -46.6447f);// Track 4


/////////////////////////////////////////
Boundary roadj6(175.0f,230.f,-80.0f,-59.32f);// Track 4

Boundary trackj7(214.0f,239.0f,-83.2103,-63.2103);// Track 4

Boundary roadj7(222.0f,244.5f,-216.0f,-83.0f);// Track 4


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
		if (!isWithinBoundaries(cornerX, cornerZ, "") || isColliding(cornerX,cornerZ,ObjXPos,ObjZPos) || isColliding(cornerX,cornerZ,Obj2XPos,Obj2ZPos ) || isColliding(cornerX, cornerZ, Obj3XPos, Obj3ZPos)) {
			return false; // One corner is out of bounds
		}



	}
	   

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
	float speed = 1.0f;                  // Movement speed
	float rotationSpeed = 5.0f * (M_PI / 180.0f); // Convert degrees to radians for rotation

	if (key == 's') {
		moveBackward(playerXPos, playerZPos, characterAngle, speed);
	}
	else if (key == 'w') {
		moveFront(playerXPos, playerZPos, characterAngle, speed);
	}
	else if (key == 'd') {
		rotateRight(characterAngle, speed, rotationSpeed);
	}
	else if (key == 'a') {
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
void myDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



	GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
	GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);
	setupCamera();
	// Draw Ground
	

	// Draw Tree Model
	/*glPushMatrix();
	glTranslatef(10, 0, 0);
	glScalef(0.7, 0.7, 0.7);
	model_tree.Draw();
	glPopMatrix();*/

	// Draw house Model
	/*glPushMatrix();
	glRotatef(90.f, 1, 0, 0);
	model_house.Draw();
	glPopMatrix();*/


	updatePlayerPosition(playerXPos, playerZPos, ObjXPos, ObjZPos);

	glPushMatrix();
	glTranslatef(playerXPos, PlayerYPos, playerZPos);// Position the car
	glRotatef(characterAngle * 180.0f / M_PI, 0.0f, 1.0f, 0.0f);

	//glScalef(0.02f, 0.02f, 0.02f); // Adjust the scale of the car if needed
	
	car.render();// Draw the car model

	glPopMatrix();
	

	glPushMatrix();
	glTranslatef(ObjXPos, 0.0, ObjZPos);
	glRotated(90, 0, 1, 0);
	glScalef(0.14f, 0.14f, 0.14f);
	model_obstacle.Draw();
	glPopMatrix();
	glPushMatrix();
	glTranslatef(Obj2XPos, 1.5, Obj2ZPos);
	//glRotated(90, 0, 1, 0);
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
	glTranslatef(-55.0, 0.0, -40.0);

	glScalef(0.02f, 0.02f, 0.02f);
	model_track.Draw();
	glPopMatrix();


	





	//sky box
	glPushMatrix();
	//glTranslatef(camera.playerPosition.x, camera.playerPosition.y, camera.playerPosition.z);  // Center skybox around camera
	glScalef(50, 50, 50);  // Increase the size of the skybox to fully cover the camera view

	// Enable texture mapping
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);

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

	glutSwapBuffers();
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
	FILE* bin3ds = fopen("Models/car/car/queen.3ds", "rb");
	if (!bin3ds) {
		perror("Failed to open file");
		exit(EXIT_FAILURE);
	}

	// Loading Model files
	car.model.Load("Models/car/car/queen.3ds");
	model_obstacle.Load("Models/obstacle/obstacle.3ds");
	model_track.Load("Track/source/weloTrack.3ds");
	//model_matar.Load("Models/Matar/mater.3DS");
	//model_cage.Load("Track/source/cagee.3ds");
	// 
	// Loading texture files
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
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

	glutDisplayFunc(myDisplay);

	glutKeyboardFunc(myKeyboard);

	glutKeyboardFunc(keyboard);

	glutMotionFunc(myMotion);

	glutMouseFunc(myMouse);

	glutReshapeFunc(myReshape);

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