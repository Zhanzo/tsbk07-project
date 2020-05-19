#pragma once
#if !defined(FONT_H)
#define FONT_H

#include <ft2build.h>
#include FT_FREETYPE_H

#include "shader.h"

FT_Library ft_lib;
Shader font_shader;

static void font_init() {
    if (FT_Init_FreeType(&ft_lib)) {
        error("FreeType init failed!\n");
    }

    shaderCompile(font_shader, "shaders/text.vert", "shaders/text.frag");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

struct Font {
    FT_Face face;

    struct Character {
        unsigned int texture;
        glm::vec2 size;
        glm::vec2 bearing;
        float advance;
    };

    std::map<char, Character> characters;
    unsigned int VAO, VBO;
};

static void font_load(Font* font, const char* filename, int size) {
    if (auto ec = FT_New_Face(ft_lib, filename, 0, &font->face); ec) {
        error("FreeType load font failed: '%s'\n", filename);
    }

    FT_Set_Pixel_Sizes(font->face, 0, size);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(font->face, c, FT_LOAD_RENDER)) {
            error("FreeType glyph failed!\n");
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, font->face->glyph->bitmap.width,
                     font->face->glyph->bitmap.rows, 0, GL_RED,
                     GL_UNSIGNED_BYTE, font->face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        auto glyph     = font->face->glyph;
        auto character = Font::Character{
            texture, glm::vec2(glyph->bitmap.width, glyph->bitmap.rows),
            glm::vec2(glyph->bitmap_left, glyph->bitmap_top),
            (float)glyph->advance.x};
        font->characters.insert(std::make_pair(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenVertexArrays(1, &font->VAO);
    glGenBuffers(1, &font->VBO);
    glBindVertexArray(font->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, font->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void font_projection(float width, float height) {
    auto projection = glm::ortho(0.0f, width, 0.0f, height);
    shaderBind(font_shader);
    shaderSetMat4(font_shader, "projection", projection);
}

float font_width(Font* font, const char* message) {
    auto scale = 1.0f;
    auto x     = 0.0f;
    auto len   = strlen(message);
    for (auto i = 0; i < len; i++) {
        auto c  = (int)message[i];
        auto ch = font->characters[c];
        x += (ch.advance / 64.0f) * scale;
    }

    return x;
}

void font_draw(Font* font, float x, float y, glm::vec3 color,
               const char* message) {
    shaderBind(font_shader);
    shaderSetVec3(font_shader, "textColor", color);
    shaderSetTexture(font_shader, "text", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(font->VAO);

    auto scale = 1.0f;
    auto len   = strlen(message);
    for (auto i = 0; i < len; i++) {
        auto c  = (int)message[i];
        auto ch = font->characters[c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f},    {xpos, ypos, 0.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 1.0f},

            {xpos, ypos + h, 0.0f, 0.0f},    {xpos + w, ypos, 1.0f, 1.0f},
            {xpos + w, ypos + h, 1.0f, 0.0f}};

        glBindTexture(GL_TEXTURE_2D, ch.texture);
        glBindBuffer(GL_ARRAY_BUFFER, font->VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.advance / 64.0f) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void font_draw(Font* font, float x, float y, const char* message) {
    font_draw(font, x, y, glm::vec3{1, 1, 1}, message);
}

void font_drawf(Font* font, float x, float y, const char* format,
                va_list args) {
    char buffer[1024];
    vsprintf(buffer, format, args);
    font_draw(font, x, y, buffer);
}

void font_drawf(Font* font, float x, float y, const char* format, ...) {
    va_list args;
    va_start(args, format);
    font_drawf(font, x, y, format, args);
    va_end(args);
}

#endif
