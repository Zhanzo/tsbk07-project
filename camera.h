#pragma once
#if !defined(CAMERA_H)
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "player.h"
#include "wall.h"

struct Camera {
    glm::vec3 position;
    float distanceFromPlayer{5.0f};
    float yaw{0.0f};
    float pitch{20.0f};
    float sensitivity{0.01f};
};

void moveCamera(Camera* camera, double xOffset, double yOffset) {
    xOffset *= camera->sensitivity;
    yOffset *= camera->sensitivity;

    camera->yaw += xOffset;
    camera->pitch -= yOffset;

    camera->pitch = glm::clamp(camera->pitch, 10.0f, 60.0f);
}

void cameraUpdatePosition(Camera* camera, Player const* player,
                          std::vector<Wall> walls) {
    float horizontalDistance{camera->distanceFromPlayer *
                             cos(glm::radians(camera->pitch))};
    float verticalDistance{camera->distanceFromPlayer *
                           sin(glm::radians(camera->pitch))};
    float playerAngle{player->angle + camera->yaw};

    float xOffset{horizontalDistance * sin(glm::radians(playerAngle))};
    float zOffset{horizontalDistance * cos(glm::radians(playerAngle))};

    camera->position = {player->position.x - xOffset,
                        player->position.y + verticalDistance,
                        player->position.z - zOffset};

    // make sure that the camera never goes outside of the walls
    camera->position.x = glm::clamp(camera->position.x,
                                    walls.at(1).position.x + walls.at(1).width,
                                    walls.at(3).position.x - walls.at(3).width);
    camera->position.z = glm::clamp(camera->position.z,
                                    walls.at(0).position.z + walls.at(0).width,
                                    walls.at(2).position.z - walls.at(2).width);
}

glm::mat4 getViewMatrix(Camera* camera, Player const* player) {
    return glm::lookAt(camera->position, player->position,
                       glm::vec3{0.0f, 1.0f, 0.0f});
}

#endif