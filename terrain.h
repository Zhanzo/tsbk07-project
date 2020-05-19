#pragma once
#if !defined(TERRAIN_H)
#define TERRAIN_H

#include "model.h"
#include "texture.h"

struct Terrain {
    GLfloat* vertices;
    GLfloat* normals;
    GLfloat* texCoords;
    GLuint* indices;

    glm::vec3 scale;
    int width;
    int height;

    int vertexCount;
    int indexCount;

    Model model;
};

struct TerrainPosition {
    glm::vec3 position;
    glm::vec3 normal;
    float height;
};

glm::vec3 normal_from_points(GLfloat* v0a, GLfloat* v1a, GLfloat* v2a) {
    glm::vec3 v0 = {v0a[0], v0a[1], v0a[2]};
    glm::vec3 v1 = {v1a[0], v1a[1], v1a[2]};
    glm::vec3 v2 = {v2a[0], v2a[1], v2a[2]};

    glm::vec3 d0 = (v0 - v1);
    glm::vec3 d1 = (v0 - v2);
    return normalize(cross(d0, d1));
}

static void terrainCreate(Terrain* terrain, Texture* tex, glm::vec3 scale) {
    int vertexCount   = tex->width * tex->height;
    int triangleCount = (tex->width - 1) * (tex->height - 1) * 2;
    int x, z;

    auto vertexArray   = (GLfloat*)malloc(sizeof(GLfloat) * 3 * vertexCount);
    auto normalArray   = (GLfloat*)malloc(sizeof(GLfloat) * 3 * vertexCount);
    auto texCoordArray = (GLfloat*)malloc(sizeof(GLfloat) * 2 * vertexCount);
    auto indexArray    = (GLuint*)malloc(sizeof(GLuint) * triangleCount * 3);

    for (x = 0; x < tex->width; x++)
        for (z = 0; z < tex->height; z++) {
            auto height = tex->data[4 * (x + z * tex->width)];
            // Vertex array. You need to scale this properly
            vertexArray[(x + z * tex->width) * 3 + 0] = x * scale.x;
            vertexArray[(x + z * tex->width) * 3 + 1] = height * scale.y;
            vertexArray[(x + z * tex->width) * 3 + 2] = z * scale.z;
            // Normal vectors. You need to calculate these.
            normalArray[(x + z * tex->width) * 3 + 0] = 0.0;
            normalArray[(x + z * tex->width) * 3 + 1] = 0.0;
            normalArray[(x + z * tex->width) * 3 + 2] = 0.0;
            // Texture coordinates. You may want to scale them.
            texCoordArray[(x + z * tex->width) * 2 + 0] = (float)x / tex->width;
            texCoordArray[(x + z * tex->width) * 2 + 1] =
                (float)z / tex->height;
        }
    for (x = 0; x < tex->width - 1; x++)
        for (z = 0; z < tex->height - 1; z++) {
            // Triangle 1
            indexArray[(x + z * (tex->width - 1)) * 6 + 0] = x + z * tex->width;
            indexArray[(x + z * (tex->width - 1)) * 6 + 1] =
                x + (z + 1) * tex->width;
            indexArray[(x + z * (tex->width - 1)) * 6 + 2] =
                x + 1 + z * tex->width;

            auto normal0 =
                normal_from_points(&vertexArray[(x + z * tex->width) * 3],
                                   &vertexArray[(x + (z + 1) * tex->width) * 3],
                                   &vertexArray[(x + 1 + z * tex->width) * 3]);
            normalArray[(x + z * tex->width) * 3 + 0] += normal0.x;
            normalArray[(x + z * tex->width) * 3 + 1] += normal0.y;
            normalArray[(x + z * tex->width) * 3 + 2] += normal0.z;

            normalArray[(x + (z + 1) * tex->width) * 3 + 0] += normal0.x;
            normalArray[(x + (z + 1) * tex->width) * 3 + 1] += normal0.y;
            normalArray[(x + (z + 1) * tex->width) * 3 + 2] += normal0.z;

            normalArray[(x + 1 + z * tex->width) * 3 + 0] += normal0.x;
            normalArray[(x + 1 + z * tex->width) * 3 + 1] += normal0.y;
            normalArray[(x + 1 + z * tex->width) * 3 + 2] += normal0.z;

            // Triangle 2
            indexArray[(x + z * (tex->width - 1)) * 6 + 3] =
                x + 1 + z * tex->width;
            indexArray[(x + z * (tex->width - 1)) * 6 + 4] =
                x + (z + 1) * tex->width;
            indexArray[(x + z * (tex->width - 1)) * 6 + 5] =
                x + 1 + (z + 1) * tex->width;

            auto normal1 = normal_from_points(
                &vertexArray[(x + 1 + z * tex->width) * 3],
                &vertexArray[(x + (z + 1) * tex->width) * 3],
                &vertexArray[(x + 1 + (z + 1) * tex->width) * 3]);
            normalArray[(x + 1 + z * tex->width) * 3 + 0] += normal1.x;
            normalArray[(x + 1 + z * tex->width) * 3 + 1] += normal1.y;
            normalArray[(x + 1 + z * tex->width) * 3 + 2] += normal1.z;

            normalArray[(x + (z + 1) * tex->width) * 3 + 0] += normal1.x;
            normalArray[(x + (z + 1) * tex->width) * 3 + 1] += normal1.y;
            normalArray[(x + (z + 1) * tex->width) * 3 + 2] += normal1.z;

            normalArray[(x + 1 + (z + 1) * tex->width) * 3 + 0] += normal1.x;
            normalArray[(x + 1 + (z + 1) * tex->width) * 3 + 1] += normal1.y;
            normalArray[(x + 1 + (z + 1) * tex->width) * 3 + 2] += normal1.z;
        }

    for (x = 0; x < tex->width; x++)
        for (z = 0; z < tex->height; z++) {
            glm::vec3 normal   = {normalArray[(x + z * tex->width) * 3 + 0],
                                normalArray[(x + z * tex->width) * 3 + 1],
                                normalArray[(x + z * tex->width) * 3 + 2]};
            glm::vec3 renormal = normalize(normal);
            normalArray[(x + z * tex->width) * 3 + 0] = renormal.x;
            normalArray[(x + z * tex->width) * 3 + 1] = renormal.y;
            normalArray[(x + z * tex->width) * 3 + 2] = renormal.z;
        }

    terrain->vertices  = vertexArray;
    terrain->normals   = normalArray;
    terrain->texCoords = texCoordArray;
    terrain->indices   = indexArray;

    terrain->vertexCount = vertexCount;
    terrain->indexCount  = triangleCount * 3;

    terrain->scale  = scale;
    terrain->width  = tex->width;
    terrain->height = tex->height;

    modelCreate(&terrain->model, terrain->vertexCount, terrain->vertices,
                terrain->normals, terrain->texCoords, terrain->indexCount,
                terrain->indices);
}

static bool terrainGetPosition(Terrain* terrain, float xPos, float zPos,
                               TerrainPosition* position) {
    xPos /= terrain->scale.x;
    zPos /= terrain->scale.z;

    int xPosInt = (int)xPos;
    int zPosInt = (int)zPos;

    auto texWidth = terrain->width;
    if (xPos >= texWidth || xPos < 0) return false;
    if (zPos >= texWidth || zPos < 0) return false;

    // which triangle is it, upper or lower?
    auto isUpperTriangle = (xPos - xPosInt + zPos - zPosInt > 1) ? true : false;

    glm::vec3 vertices[3];
    if (isUpperTriangle) {
        vertices[0] = glm::vec3{
            terrain
                ->vertices[((xPosInt + 1) + (zPosInt + 1) * texWidth) * 3 + 0],
            terrain
                ->vertices[((xPosInt + 1) + (zPosInt + 1) * texWidth) * 3 + 1],
            terrain
                ->vertices[((xPosInt + 1) + (zPosInt + 1) * texWidth) * 3 + 2]};
        vertices[1] = glm::vec3{
            terrain->vertices[(xPosInt + (zPosInt + 1) * texWidth) * 3 + 0],
            terrain->vertices[(xPosInt + (zPosInt + 1) * texWidth) * 3 + 1],
            terrain->vertices[(xPosInt + (zPosInt + 1) * texWidth) * 3 + 2]};
        vertices[2] = glm::vec3{
            terrain->vertices[((xPosInt + 1) + zPosInt * texWidth) * 3 + 0],
            terrain->vertices[((xPosInt + 1) + zPosInt * texWidth) * 3 + 1],
            terrain->vertices[((xPosInt + 1) + zPosInt * texWidth) * 3 + 2]};
    } else {
        vertices[0] = glm::vec3{
            terrain->vertices[(xPosInt + zPosInt * texWidth) * 3 + 0],
            terrain->vertices[(xPosInt + zPosInt * texWidth) * 3 + 1],
            terrain->vertices[(xPosInt + zPosInt * texWidth) * 3 + 2]};
        vertices[1] = glm::vec3{
            terrain->vertices[((xPosInt + 1) + zPosInt * texWidth) * 3 + 0],
            terrain->vertices[((xPosInt + 1) + zPosInt * texWidth) * 3 + 1],
            terrain->vertices[((xPosInt + 1) + zPosInt * texWidth) * 3 + 2]};
        vertices[2] = glm::vec3{
            terrain->vertices[(xPosInt + (zPosInt + 1) * texWidth) * 3 + 0],
            terrain->vertices[(xPosInt + (zPosInt + 1) * texWidth) * 3 + 1],
            terrain->vertices[(xPosInt + (zPosInt + 1) * texWidth) * 3 + 2]};
    }

    // calculate the normal of the three vertices
    auto v1     = (vertices[1] - vertices[0]);
    auto v2     = (vertices[2] - vertices[0]);
    auto normal = normalize(cross(v1, v2));

    xPos *= terrain->scale.x;
    zPos *= terrain->scale.z;

    // scalar equation of plane: ax + by + cz = d
    float A = normal.x, B = normal.y, C = normal.z;
    float D    = -A * vertices[0].x - B * vertices[0].y - C * vertices[0].z;
    float yPos = -(D + A * xPos + C * zPos) / B;
    *position  = {glm::vec3{xPos, yPos, zPos}, normal, yPos};
    return true;
}

#endif
