#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <cmath>
#include <iostream>


#define DEG2RAD(a) (a * 0.0174532925)



int WIDTH = 1280;
int HEIGHT = 720;
float PlayerXPos = 0.0f;
float PlayerZPos = 0.0f;
float PlayerYPos = 0.5f;  // Adjust for ground level
float ObjXPos = 5.0f;
float ObjZPos = 5.0f;


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
	}

	void updateCameraPosition() {
		if (currentView == THIRD) {
			// Camera positioned slightly above and behind the car
			playerPosition = Vector3f(playerXPos, 1.75f, playerZPos);
			eye = playerPosition + Vector3f(0.0f, 5.0f, -distanceFromPlayer); // Adjust these offsets as needed
			center = playerPosition + Vector3f(0.0f, 2.0f, 0.0f); // Looking slightly above the player's position
			up = Vector3f(0.0f, 1.0f, 0.0f);


		}
		else if (currentView == FIRST) {
			// Camera positioned at the level of the car and slightly forward
			playerPosition = Vector3f(playerXPos, 1.75f, playerZPos - 4);
			eye = playerPosition + Vector3f(0.0f, 0.8f, 4.0); // Slightly above and forward of the car's position
			center = playerPosition + Vector3f(0.0f, 0.8f, 4.3); // Look further ahead from the car
			up = Vector3f(0.0f, 0.1f, 0.0f);
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
	gluPerspective(60, 640.0 / 480.0, 0.1, 100.0);

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
	camera.updateCameraPosition();

	lastMouseX = x;
	lastMouseY = y;

	glutPostRedisplay();
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

void keyboard(unsigned char key, int x, int y) {
	float speed = 1.0f; // Adjust movement speed

	if (key == 's') {
		playerZPos -= speed;
		// Zoom in

	}
	else if (key == 'w') {
		playerZPos += speed;

	}
	else if (key == 'd') {
		playerXPos -= speed;
		camera.pitch += 1.0f;  // Increase pitch (move camera upwards)
		if (camera.pitch > 89.0f) camera.pitch = 89.0f;  // Prevent flipping upside down
	}
	else if (key == 'a') {
		playerXPos += speed;
		camera.pitch -= 1.0f;  // Decrease pitch (move camera downwards)
		if (camera.pitch < -89.0f) camera.pitch = -89.0f;  // Prevent flipping upside down
	}

	else if (key == 't') {
		camera.currentView = THIRD;
	}

	else if (key == 'f') {
		camera.currentView = FIRST;
	}

	std::cout << "Player's X Position: " << playerXPos << std::endl;
	std::cout << "Player's Z Position: " << playerZPos << std::endl;
	std::cout << "OBJ's X Position: " << ObjXPos << std::endl;
	std::cout << "OBJ's Z Position: " << ObjZPos << std::endl;
	camera.playerPosition = Vector3f(playerXPos, PlayerYPos, playerZPos);
	camera.updateCameraPosition();

	glutPostRedisplay();
}


bool isColliding(float playerX, float playerZ, float objX, float objZ) {
	if (playerX >= objX - 3 && playerX <= objX + 7 && playerZ == objZ - 3)
		return true;
	else
		return false;
}




void updatePlayerPosition(float& playerX, float& playerZ, float objX, float objZ) {
	if (isColliding(playerX, playerZ, objX, objZ)) {
		playerZ -= 5.0f; // Move backward

		std::cout << "Collision detected: Player moved back. New Z position: " << playerZ << std::endl;
	}
	else {
		std::cout << "No collision. Player Z position: " << playerZ << std::endl;
	}

}




// Model Variables
Model_3DS model_house;
Model_3DS model_tree;
Model_3DS model_car;
Model_3DS model_obstacle;
Model_3DS model_track;
// Textures
GLTexture tex_ground;
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
void RenderGround()
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-20, 0, -20);
	glTexCoord2f(5, 0);
	glVertex3f(20, 0, -20);
	glTexCoord2f(5, 5);
	glVertex3f(20, 0, 20);
	glTexCoord2f(0, 5);
	glVertex3f(-20, 0, 20);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

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

	// Draw Ground
	RenderGround();
	setupCamera();

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
	//glScalef(0.02f, 0.02f, 0.02f); // Adjust the scale of the car if needed
	model_car.Draw(); // Draw the car model

	glPopMatrix();
	
	model_track.Draw();
	glPushMatrix();
	glTranslatef(ObjXPos, 0.0, ObjZPos);
	glRotated(90, 0, 1, 0);
	glScalef(0.2f, 0.2f, 0.2f);
	model_obstacle.Draw();
	glPopMatrix();


	//sky box
	glPushMatrix();
	GLUquadricObj* qobj;
	qobj = gluNewQuadric();
	glTranslated(50, 0, 0);
	glRotated(90, 1, 0, 1);
	glBindTexture(GL_TEXTURE_2D, tex);
	gluQuadricTexture(qobj, true);
	gluQuadricNormals(qobj, GL_SMOOTH);
	gluSphere(qobj, 100, 100, 100);
	gluDeleteQuadric(qobj);


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
	model_house.Load("Models/house/house.3DS");
	model_tree.Load("Models/tree/Tree1.3ds");
	model_car.Load("Models/car/car/queen.3ds");
	model_obstacle.Load("Models/obstacle/obstacle.3ds");
	model_track.Load("Track/source/DriftTrack3.3ds");
	// Loading texture files
	tex_ground.Load("Textures/ground.bmp");
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