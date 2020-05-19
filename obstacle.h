#pragma once
#if !defined(OBSTACLE_H)
#define OBSTACLE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "math_utils.h"
#include "terrain.h"
#include "wall.h"

class Obstacle {
  public:
    glm::vec3 velocity{3.0f, 0.0f, 3.0f};
    glm::vec3 position;
    float radius;
    int myId;

    Obstacle(glm::vec3 position, glm::vec3 normal, glm::vec3 _radius)
        : position{position}, normal{normal} {
        static int id{0};
        myId   = id++;
        radius = scale * glm::compAdd(_radius) / 3.0f;
    }

    void update(Terrain* terrain, std::vector<Obstacle> obstacles,
                std::vector<Wall> walls, float deltaTime) {
        position += velocity * deltaTime;

        checkWallCollisions(walls);
        checkObstacleCollisions(obstacles);

        TerrainPosition terrPos{};
        terrainGetPosition(terrain, position.x, position.z, &terrPos);
        position = terrPos.position;
        normal   = terrPos.normal;

        angle -= glm::length(velocity) * deltaTime;
        angle = wrapRadAngle(angle);
    }

    glm::mat4 getMatrix() {
        glm::vec3 pos{position};
        pos.y += radius;
        glm::mat4 modelMatrix{glm::translate(glm::mat4(1), pos)};

        glm::vec3 rotationAxis{glm::cross(normal, glm::normalize(velocity))};
        modelMatrix = glm::rotate(modelMatrix, angle, rotationAxis);

        modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
        return modelMatrix;
    }

  private:
    glm::vec3 normal;
    float angle;
    float scale{1.0f};

    void checkWallCollisions(std::vector<Wall> walls) {
        if (position.z - radius <= walls.at(0).position.z + walls.at(0).width) {
            velocity.z = -velocity.z;
            position.z = radius + walls.at(0).position.z + walls.at(0).width;
        }
        if (position.x - radius <= walls.at(1).position.x + walls.at(1).width) {
            velocity.x = -velocity.x;
            position.x = radius + walls.at(1).position.x + walls.at(1).width;
        }
        if (position.z + radius >= walls.at(2).position.z - walls.at(2).width) {
            velocity.z = -velocity.z;
            position.z = walls.at(2).position.z - walls.at(2).width - radius;
        }
        if (position.x + radius >= walls.at(3).position.x - walls.at(3).width) {
            velocity.x = -velocity.x;
            position.x = walls.at(3).position.x - walls.at(3).width - radius;
        }
    }

    void checkObstacleCollisions(std::vector<Obstacle> obstacles) {
        for (auto& obstacle : obstacles) {
            if (myId != obstacle.myId) {
                float distance{glm::length(position - obstacle.position)};
                float totalRadius{radius + obstacle.radius};
                if (distance < totalRadius) {
                    float overlap{0.5f * (distance - totalRadius)};

                    position -=
                        overlap * (position - obstacle.position) / distance;
                    obstacle.position +=
                        overlap * (position - obstacle.position) / distance;

                    glm::vec3 v1{velocity};
                    glm::vec3 v2{obstacle.velocity};
                    glm::vec3 x1{position};
                    glm::vec3 x2{obstacle.position};

                    velocity = v1 - (glm::dot(v1 - v2, x1 - x2) /
                                     glm::length(x1 - x2)) *
                                        (x1 - x2);
                    obstacle.velocity = v2 - (glm::dot(v2 - v1, x2 - x1) /
                                              glm::length(x2 - x1)) *
                                                 (x2 - x1);
                }
            }
        }
    }
};

#endif