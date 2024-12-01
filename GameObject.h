#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Model_3DS.h"
#include <glut.h>

class GameObject {
public:
    Model_3DS model;  // The 3D model for the object
    float x, y, z;    // Position of the object

    // Constructor accepts a model and initial position
    GameObject(Model_3DS _model, float _x = 0.0f, float _y = 0.0f, float _z = 0.0f)
        : model(_model), x(_x), y(_y), z(_z) {}

    // Render the GameObject
    virtual void render() {
        glPushMatrix();
        glTranslatef(x, y, z);  // Position the object
        model.Draw();            // Draw the 3D model
        glPopMatrix();
    }

    // Set the position of the object
    void setPosition(float _x, float _y, float _z) {
        x = _x;
        y = _y;
        z = _z;
    }
    float getX();
    float getY();
    float getZ();
    void setX(float x);
    void setY(float y);
    void setZ(float z);
};

#endif
