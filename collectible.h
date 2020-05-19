#pragma once
#if !defined(COLLECTIBLE_H)
#define COLLECTIBLE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Collectible {
  public:
    glm::vec3 position;

    float scale{0.3f};
    float radius;

    int pointsWorth;

    Collectible(glm::vec3 position, glm::vec3 _radius, int pointsWorth = 1)
        : position{position}, pointsWorth{pointsWorth} {
        radius = scale * glm::compAdd(_radius) / 3.0f;
    }

    glm::mat4 getMatrix() {
        glm::vec3 pos{position};
        pos.y += radius;
        glm::mat4 modelMatrix{glm::translate(glm::mat4(1), pos)};

        modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
        return modelMatrix;
    }
};

#endif