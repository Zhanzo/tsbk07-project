#pragma once
#if !defined(SHADER_H)
#define SHADER_H

#include "error.h"
#include "string.h"

struct Shader {
    u32 program;
    u32 vertex;
    u32 fragment;
};

// Compile vertex and fragment shader into a program shader
static bool shaderCompile(Shader& shader, std::string const& vertex_path,
                          std::string const& fragment_path) {
    // TODO: Clean up data for early returns

    String vertex_data{};
    if (!file_read(vertex_path.c_str(), vertex_data)) {
        error("Reading vertex file failed: %s", vertex_path.c_str());
        return false;
    }

    String fragment_data{};
    if (!file_read(fragment_path.c_str(), fragment_data)) {
        error("Reading fragment file failed: %s", fragment_path.c_str());
        return false;
    }

    auto success = 0;
    char info_buffer[4096];

    // Vertex
    auto vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertex_data.data, NULL);
    glCompileShader(vertex);

    // Check for compile errors
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, sizeof(info_buffer), NULL, info_buffer);
        error("Compiling vertex-shader failed!\n\n%s", info_buffer);
        return false;
    }

    // Fragment
    auto fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragment_data.data, NULL);
    glCompileShader(fragment);

    // Check for compile errors
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, sizeof(info_buffer), NULL, info_buffer);
        error("Compiling fragment-shader failed!\n\n%s", info_buffer);
        return false;
    }

    auto shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex);
    glAttachShader(shader_program, fragment);
    glLinkProgram(shader_program);

    // Check for link errors
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, sizeof(info_buffer), NULL,
                            info_buffer);
        error("Linking program failed!\n\n%s", info_buffer);
        return false;
    }

    shader.program  = shader_program;
    shader.vertex   = vertex;
    shader.fragment = fragment;
    return true;
}

static bool shaderSetFloat(Shader& shader, std::string const& name,
                           float value) {
    auto location = glGetUniformLocation(shader.program, name.c_str());
    if (location < 0) {
        error("Shader uniform not found! '%s'\n", name.c_str());
        return false;
    }

    glUniform1f(location, value);
    return true;
}

static bool shaderSetFloat(Shader& shader, std::string const& name, int index,
                           std::string const& member, float value) {
    char buffer[128];
    sprintf(buffer, "%s[%i].%s", name.c_str(), index, member.c_str());
    return shaderSetFloat(shader, buffer, value);
}

static bool shaderSetVec2(Shader& shader, std::string const& name,
                          glm::vec2 const& value) {
    auto location = glGetUniformLocation(shader.program, name.c_str());
    if (location < 0) {
        error("Shader uniform not found! '%s'\n", name.c_str());
        return false;
    }

    glUniform2fv(location, 1, glm::value_ptr(value));
    return true;
}

static bool shaderSetVec3(Shader& shader, std::string const& name,
                          glm::vec3 const& value) {
    auto location = glGetUniformLocation(shader.program, name.c_str());
    if (location < 0) {
        error("Shader uniform not found! '%s'\n", name.c_str());
        return false;
    }

    glUniform3fv(location, 1, glm::value_ptr(value));
    return true;
}

static bool shaderSetVec3(Shader& shader, std::string const& name, int index,
                          std::string const& member, glm::vec3 const& value) {
    char buffer[128];
    sprintf(buffer, "%s[%i].%s", name.c_str(), index, member.c_str());
    return shaderSetVec3(shader, buffer, value);
}

static bool shaderSetVec4(Shader& shader, std::string const& name,
                          glm::vec4 const& value) {
    auto location = glGetUniformLocation(shader.program, name.c_str());
    if (location < 0) {
        error("Shader uniform not found! '%s'\n", name.c_str());
        return false;
    }

    glUniform4fv(location, 1, glm::value_ptr(value));
    return true;
}

static bool shaderSetMat4(Shader& shader, std::string const& name,
                          glm::mat4 const& value) {
    auto location = glGetUniformLocation(shader.program, name.c_str());
    if (location < 0) {
        error("Shader uniform not found! '%s'\n", name.c_str());
        return false;
    }

    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
    return true;
}

static bool shaderSetTexture(Shader& shader, const std::string& name, int id) {
    auto location = glGetUniformLocation(shader.program, name.c_str());
    if (location < 0) {
        error("Shader uniform not found! '%s'\n", name.c_str());
        return false;
    }

    glUniform1i(location, id);
    return true;
}

// Bind shader to OpenGL context
static void shaderBind(Shader& shader) {
    glUseProgram(shader.program);
}

#endif
