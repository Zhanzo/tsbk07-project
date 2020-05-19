#pragma once
#if !defined(MODEL_H)
#define MODEL_H

#define GLEW_STATIC
#include <GL/glew.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <glm/glm.hpp>
#include <vector>

#include "error.h"
#include "type.h"

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

struct Material {};

struct Model {
    u32 VAO, VBO, EBO;
    int indexCount;

    glm::vec3 lower_bound;
    glm::vec3 upper_bound;
    glm::vec3 radius;
};

static void modelCreate(Model* model, const std::vector<Vertex>& vertices,
                        const std::vector<u32>& indices) {
    model->indexCount = indices.size();

    glGenVertexArrays(1, &model->VAO);
    glGenBuffers(1, &model->VBO);
    glGenBuffers(1, &model->EBO);

    glBindVertexArray(model->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, model->VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
                 &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(u32),
                 &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, texCoords));

    glBindVertexArray(0);
}

static void modelCreate(Model* model, int vertexCount, GLfloat* positions,
                        GLfloat* normals, GLfloat* texcoords, int indexCount,
                        GLuint* indices) {

    std::vector<Vertex> vertices_array;
    std::vector<u32> indices_array;

    for (auto i = 0; i < vertexCount; i++) {
        Vertex vertex{};
        vertex.position = {positions[3 * i + 0], positions[3 * i + 1],
                           positions[3 * i + 2]};

        if (normals) {
            vertex.normal = {normals[3 * i + 0], normals[3 * i + 1],
                             normals[3 * i + 2]};
        } else {
            vertex.normal = {0, 1, 0};
        }

        if (texcoords) {
            vertex.texCoords = {texcoords[2 * i + 0], texcoords[2 * i + 1]};
        }

        vertices_array.push_back(vertex);
    }

    for (auto i = 0; i < indexCount; i++) {
        indices_array.push_back(indices[i]);
    }

    modelCreate(model, vertices_array, indices_array);
}

// load an OBJ using tinyobjloader
void loadModel(Model* model, std::string file_name) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;

    bool ret{tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              file_name.c_str(), "materials/")};

    if (!warn.empty()) error(warn.c_str());
    if (!err.empty()) error(err.c_str());
    if (!ret) exit(1);

    std::vector<Vertex> vertices;
    std::vector<u32> indices;

    model->upper_bound = {-10000, -10000, -10000};
    model->lower_bound = {+10000, +10000, +10000};

    // Loop over shapes
    glm::vec3 minValues;
    glm::vec3 maxValues;
    bool first{true};
    for (auto const& shape : shapes) {
        // Loop over faces(polygon)
        for (auto const& index : shape.mesh.indices) {
            Vertex vertex{};
            vertex.position = {attrib.vertices[3 * index.vertex_index + 0],
                               attrib.vertices[3 * index.vertex_index + 1],
                               attrib.vertices[3 * index.vertex_index + 2]};

            if (first) {
                minValues = vertex.position;
                maxValues = vertex.position;
                first     = false;
            } else {
                minValues.x = glm::min(vertex.position.x, minValues.x);
                minValues.y = glm::min(vertex.position.y, minValues.y);
                minValues.z = glm::min(vertex.position.z, minValues.z);
                maxValues.x = glm::max(vertex.position.x, maxValues.x);
                maxValues.y = glm::max(vertex.position.y, maxValues.y);
                maxValues.z = glm::max(vertex.position.z, maxValues.z);
            }

            model->lower_bound = glm::min(model->lower_bound, vertex.position);
            model->upper_bound = glm::max(model->upper_bound, vertex.position);

            if (index.normal_index >= 0) {
                vertex.normal = {attrib.normals[3 * index.normal_index + 0],
                                 attrib.normals[3 * index.normal_index + 1],
                                 attrib.normals[3 * index.normal_index + 2]};
            } else {
                vertex.normal = {0, 1, 0};
            }

            if (index.texcoord_index >= 0) {
                vertex.texCoords = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]};
            }

            vertices.push_back(vertex);
            indices.push_back(indices.size());
        }
    }

    model->radius = (maxValues - minValues) / 2.0f;
    modelCreate(model, vertices, indices);
}

void drawModel(Model* model) {
    glBindVertexArray(model->VAO);
    glDrawElements(GL_TRIANGLES, model->indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void cleanUpModel(Model* model) {
    glDeleteVertexArrays(1, &model->VAO);
    glDeleteBuffers(1, &model->VBO);
    glDeleteBuffers(1, &model->EBO);
}

#endif
