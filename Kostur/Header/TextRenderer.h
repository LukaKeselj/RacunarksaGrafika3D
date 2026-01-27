#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <string>

struct Character {
    unsigned int TextureID;
    int SizeX, SizeY;
    int BearingX, BearingY;
    unsigned int Advance;
};

class TextRenderer {
private:
    std::map<char, Character> Characters;
    unsigned int VAO, VBO;
    unsigned int shaderProgram;
    FT_Library ft;
    FT_Face face;
    int windowWidth, windowHeight;

public:
    TextRenderer(unsigned int shader, int width, int height);
    ~TextRenderer();
    
    bool loadFont(const char* fontPath, unsigned int fontSize);
    void renderText(const std::string& text, float x, float y, float scale, float r, float g, float b, float alpha = 1.0f);
    float getTextWidth(const std::string& text, float scale);
};
