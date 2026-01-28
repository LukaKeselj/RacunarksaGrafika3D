#include "../Header/TextRenderer3D.h"
#include <iostream>

TextRenderer3D::TextRenderer3D(unsigned int shader) 
    : shaderProgram(shader)
{
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library (3D)" << std::endl;
        return;
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 5, NULL, GL_DYNAMIC_DRAW);
    
    // Position (3D) + TexCoords
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

TextRenderer3D::~TextRenderer3D() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

bool TextRenderer3D::loadFont(const char* fontPath, unsigned int fontSize) {
    if (FT_New_Face(ft, fontPath, 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font at: " << fontPath << " (3D)" << std::endl;
        return false;
    }

    FT_Set_Pixel_Sizes(face, 0, fontSize);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYPE: Failed to load Glyph: " << c << " (3D)" << std::endl;
            continue;
        }

        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character3D character = {
            texture,
            (int)face->glyph->bitmap.width,
            (int)face->glyph->bitmap.rows,
            face->glyph->bitmap_left,
            face->glyph->bitmap_top,
            (unsigned int)face->glyph->advance.x
        };
        Characters.insert(std::pair<char, Character3D>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    std::cout << "FreeType 3D font loaded successfully: " << fontPath << std::endl;
    return true;
}

void TextRenderer3D::renderText3D(const std::string& text, glm::vec3 position, glm::mat4 view, glm::mat4 projection,
                                  float scale, float r, float g, float b, float alpha) {
    glUseProgram(shaderProgram);
    
    int viewLoc = glGetUniformLocation(shaderProgram, "uView");
    int projLoc = glGetUniformLocation(shaderProgram, "uProjection");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    int colorLoc = glGetUniformLocation(shaderProgram, "uTextColor");
    glUniform3f(colorLoc, r, g, b);
    
    int alphaLoc = glGetUniformLocation(shaderProgram, "uAlpha");
    glUniform1f(alphaLoc, alpha);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    float x = position.x;
    
    for (char c : text) {
        Character3D ch = Characters[c];

        float xpos = x + ch.BearingX * scale * 0.0005f;
        float ypos = position.y - (ch.SizeY - ch.BearingY) * scale * 0.0005f;

        float w = ch.SizeX * scale * 0.0005f;
        float h = ch.SizeY * scale * 0.0005f;

        float vertices[6][5] = {
            { xpos,     ypos,       position.z, 0.0f, 1.0f },
            { xpos,     ypos + h,   position.z, 0.0f, 0.0f },
            { xpos + w, ypos + h,   position.z, 1.0f, 0.0f },

            { xpos,     ypos,       position.z, 0.0f, 1.0f },
            { xpos + w, ypos + h,   position.z, 1.0f, 0.0f },
            { xpos + w, ypos,       position.z, 1.0f, 1.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.Advance >> 6) * scale * 0.0005f;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

float TextRenderer3D::getTextWidth(const std::string& text, float scale) {
    float width = 0;
    for (char c : text) {
        Character3D ch = Characters[c];
        width += (ch.Advance >> 6) * scale * 0.0005f;
    }
    return width;
}
