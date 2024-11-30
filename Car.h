#ifndef CAR_H
#define CAR_H

#include "GameObject.h"

class Car : public GameObject {
public:
    float rotationAngle; // Rotation angle for the car

    // Constructor accepts a model and initial position
    Car(Model_3DS _model, float _x = 0.0f, float _y = 0.5f, float _z = 0.0f)
        : GameObject(_model, _x, _y, _z), rotationAngle(0.0f) {}

    // Override render to rotate the car as well
    void render() override {
        glPushMatrix();
        glTranslatef(x, y, z);     // Position the car
        glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f);  // Apply rotation
        model.Draw();               // Draw the car's 3D model
        glPopMatrix();
    }

    // Set the rotation of the car
    void setRotation(float angle) {
        rotationAngle = angle;
    }

    // Update car's position (can be extended for movement logic)
    void updatePosition(float _x, float _y, float _z) {
        setPosition(_x, _y, _z);
    }
};

#endif
