#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <string>

struct Character3D {
    unsigned int TextureID;
    int SizeX, SizeY;
    int BearingX, BearingY;
    unsigned int Advance;
};

class TextRenderer3D {
private:
    std::map<char, Character3D> Characters;
    unsigned int VAO, VBO;
    unsigned int shaderProgram;
    FT_Library ft;
    FT_Face face;

public:
    TextRenderer3D(unsigned int shader);
    ~TextRenderer3D();
    
    bool loadFont(const char* fontPath, unsigned int fontSize);
    void renderText3D(const std::string& text, glm::vec3 position, glm::mat4 view, glm::mat4 projection,
                      float scale, float r, float g, float b, float alpha = 1.0f);
    float getTextWidth(const std::string& text, float scale);
};
