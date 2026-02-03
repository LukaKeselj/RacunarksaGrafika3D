#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

struct OBJMesh {
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;
    unsigned int indexCount;
    unsigned int texture;
    
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    void cleanup() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
    }
};

class OBJLoader {
public:
    static bool loadOBJ(const std::string& path, OBJMesh& outMesh);
    static void setupMesh(OBJMesh& mesh);
};
