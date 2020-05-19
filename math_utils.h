#pragma once
#if !defined(MATH_UTILS_H)
#define MATH_UTILS_H

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <algorithm>

glm::mat4 rotateAlign(glm::vec3 u1, glm::vec3 u2) {
    glm::vec3 axis     = glm::normalize(glm::cross(u1, u2));
    float dotProduct   = glm::dot(u1, u2);
    dotProduct         = std::clamp(dotProduct, -1.0f, 1.0f);
    float angleRadians = acosf(dotProduct);
    glm::mat4 result   = glm::rotate(angleRadians, axis);
    return result;
}

glm::mat4 rotateX(glm::mat4 matrix, float angle) {
    return glm::rotate(matrix, glm::radians(angle),
                       glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::mat4 rotateY(glm::mat4 matrix, float angle) {
    return glm::rotate(matrix, glm::radians(angle),
                       glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 rotateZ(glm::mat4 matrix, float angle) {
    return glm::rotate(matrix, glm::radians(angle),
                       glm::vec3(0.0f, 0.0f, 1.0f));
}

double wrapAngle(float angle) {
    return angle - 360 * floor(angle / 360);
}

double wrapRadAngle(double angle) {
    return angle - (2 * M_PI) * floor(angle / (2 * M_PI));
}

#endif