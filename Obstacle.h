#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "GameObject.h"
#include <cmath>

class Car; // Forward declaration of the Car class

class Obstacle : public GameObject {
private:
    float width, height, depth; // Dimensions of the obstacle

public:
    Obstacle(float x, float y, float z, float width, float height, float depth)
        : GameObject(x, y, z), width(width), height(height), depth(depth) {}

    bool checkCollision(const Car& car);
};

#endif // OBSTACLE_H
