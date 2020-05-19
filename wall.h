#pragma once
#if !defined(WALL_H)
#define WALL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

#include "model.h"

struct Wall {
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 radius;

    float width{2.0f};
    float height{3.0f};
};

glm::mat4 getWallMatrix(Wall* wall) {
    glm::vec3 position{wall->position};
    position.y += 0.5f * wall->scale.y;

    glm::mat4 modelMatrix{glm::translate(glm::mat4(1), position)};
    modelMatrix = glm::scale(modelMatrix, wall->scale);
    return modelMatrix;
}

#endif
