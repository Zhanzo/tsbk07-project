#pragma once
#if !defined(PLAYER_H)
#define PLAYER_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <vector>

#include "camera.h"
#include "collectible.h"
#include "math_utils.h"
#include "obstacle.h"
#include "terrain.h"
#include "wall.h"

enum Movement_Direction { FORWARD, BACKWARD, LEFT, RIGHT };

class Player {
  public:
    glm::vec3 position;
    glm::vec3 oldPosition;
    glm::vec3 normal;
    glm::vec3 velocity{5.0f, 0.0f, 5.0f};

    float scale{0.5f};
    float angle{0.0f};
    float turnDirection{0.0f};
    float moveDirection{0.0f};
    float turnSpeed{100.0f};
    float gravity{-0.2f};
    float jumpPower{0.1f};
    float radius;

    int health;
    int score;
    int highScore;

    bool canJump{true};

    void initialize(glm::vec3 _position, glm::vec3 _normal, glm::vec3 _radius,
                    int _health) {
        position      = _position;
        normal        = _normal;
        radius        = scale * glm::compAdd(_radius) / 3.0f;
        health        = _health;
        score         = 0;
        angle         = 0.0f;
        turnDirection = 0.0f;
        moveDirection = 0.0f;
    }

    void update(Terrain* terrain, std::vector<Wall>& walls,
                std::vector<Obstacle>& obstacles,
                std::vector<Collectible>* collectibles, float deltaTime) {
        // handle running
        float runSpeed{moveDirection * deltaTime};
        position.x += runSpeed * velocity.x * sin(glm::radians(angle));
        position.z += runSpeed * velocity.z * cos(glm::radians(angle));

        // handle turning
        angle += turnDirection * turnSpeed * deltaTime;
        angle = wrapAngle(angle);

        collisionWaitTime += deltaTime;

        checkWallCollisions(walls);
        checkObstacleCollisions(obstacles, deltaTime);
        checkCollectibleCollisions(collectibles);

        // handle jumping
        TerrainPosition terrPos{};
        terrainGetPosition(terrain, position.x, position.z, &terrPos);
        velocity.y += gravity * deltaTime;
        position.y += velocity.y;

        if (position.y <= terrPos.height) {
            position   = terrPos.position;
            normal     = terrPos.normal;
            velocity.y = 0.0f;
            canJump    = true;
        }
    }
    glm::mat4 getMatrix() {
        glm::vec3 pos{position};
        pos.y += radius;
        glm::mat4 modelMatrix{glm::translate(glm::mat4(1.0f), pos)};

        modelMatrix *= rotateAlign(glm::vec3(0.0f, 0.0f, 1.0f), normal);
        modelMatrix = rotateX(modelMatrix, -90.0f);
        modelMatrix = rotateY(modelMatrix, angle);

        modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
        return modelMatrix;
    }

  private:
    float collisionWaitTime{0.0f};

    void checkWallCollisions(std::vector<Wall>& walls) {
        position.x =
            glm::clamp(position.x, walls.at(1).position.x + walls.at(1).width,
                       walls.at(3).position.x - walls.at(3).width);
        position.z =
            glm::clamp(position.z, walls.at(0).position.z + walls.at(0).width,
                       walls.at(2).position.z - walls.at(2).width);
    }

    void checkObstacleCollisions(std::vector<Obstacle>& obstacles,
                                 float deltaTime) {
        for (auto& obstacle : obstacles) {
            float distance{glm::length(position - obstacle.position)};
            float totalRadius{radius + obstacle.radius};
            if (distance < totalRadius) {
                if (collisionWaitTime >= 3.0f) {
                    health--;
                    collisionWaitTime = 0.0f;
                }

                float overlap{0.5f * (distance - totalRadius)};

                position -= overlap * (position - obstacle.position) / distance;
                obstacle.position +=
                    overlap * (position - obstacle.position) / distance;
            }
        }
    }

    void checkCollectibleCollisions(std::vector<Collectible>* collectibles) {
        int indxToRemove{-1};
        for (int i = 0; i < collectibles->size(); i++) {
            Collectible collectible{collectibles->at(i)};
            float distance{glm::length(position - collectible.position)};
            float totalRadius{radius + collectible.radius};
            if (distance < totalRadius) {
                score += collectible.pointsWorth;
                indxToRemove = i;
                break;
            }
        }
        if (indxToRemove != -1)
            collectibles->erase(collectibles->begin() + indxToRemove);
    }
};

#endif
