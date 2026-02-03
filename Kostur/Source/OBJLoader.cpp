#include "../Header/OBJLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

bool OBJLoader::loadOBJ(const std::string& path, OBJMesh& outMesh) {
    std::cout << "\n[OBJ LOADER] Attempting to load: " << path << std::endl;

    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "[OBJ LOADER] ? Failed to open file: " << path << std::endl;
        return false;
    }

    std::cout << "[OBJ LOADER] ? File opened successfully" << std::endl;

    int lineCount = 0;
    std::string line;

    while (std::getline(file, line)) {
        lineCount++;
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            glm::vec3 position;
            iss >> position.x >> position.y >> position.z;
            temp_positions.push_back(position);
        }
        else if (prefix == "vt") {
            glm::vec2 uv;
            iss >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        }
        else if (prefix == "vn") {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (prefix == "f") {
            std::string vertex1, vertex2, vertex3;
            iss >> vertex1 >> vertex2 >> vertex3;

            // Process each vertex of the face
            for (const auto& vertexStr : { vertex1, vertex2, vertex3 }) {
                std::istringstream viss(vertexStr);
                std::string indexStr;

                int posIdx = -1, uvIdx = -1, normalIdx = -1;

                // Parse position index
                if (std::getline(viss, indexStr, '/')) {
                    posIdx = std::stoi(indexStr) - 1;
                }

                // Parse UV index
                if (std::getline(viss, indexStr, '/') && !indexStr.empty()) {
                    uvIdx = std::stoi(indexStr) - 1;
                }

                // Parse normal index
                if (std::getline(viss, indexStr) && !indexStr.empty()) {
                    normalIdx = std::stoi(indexStr) - 1;
                }

                // Build vertex data (Position + Normal + UV = 8 floats)
                // Position (3 floats)
                if (posIdx >= 0 && posIdx < temp_positions.size()) {
                    outMesh.vertices.push_back(temp_positions[posIdx].x);
                    outMesh.vertices.push_back(temp_positions[posIdx].y);
                    outMesh.vertices.push_back(temp_positions[posIdx].z);
                }
                else {
                    outMesh.vertices.push_back(0.0f);
                    outMesh.vertices.push_back(0.0f);
                    outMesh.vertices.push_back(0.0f);
                }

                // Normal (3 floats)
                if (normalIdx >= 0 && normalIdx < temp_normals.size()) {
                    outMesh.vertices.push_back(temp_normals[normalIdx].x);
                    outMesh.vertices.push_back(temp_normals[normalIdx].y);
                    outMesh.vertices.push_back(temp_normals[normalIdx].z);
                }
                else {
                    outMesh.vertices.push_back(0.0f);
                    outMesh.vertices.push_back(1.0f);
                    outMesh.vertices.push_back(0.0f);
                }

                // UV (2 floats)
                if (uvIdx >= 0 && uvIdx < temp_uvs.size()) {
                    outMesh.vertices.push_back(temp_uvs[uvIdx].x);
                    outMesh.vertices.push_back(temp_uvs[uvIdx].y);
                }
                else {
                    outMesh.vertices.push_back(0.0f);
                    outMesh.vertices.push_back(0.0f);
                }

                // Add index (simple sequential indexing)
                outMesh.indices.push_back(static_cast<unsigned int>(outMesh.indices.size()));
            }
        }
    }

    file.close();

    outMesh.indexCount = static_cast<unsigned int>(outMesh.indices.size());

    std::cout << "[OBJ LOADER] File parsing complete:" << std::endl;
    std::cout << "  - Lines read: " << lineCount << std::endl;
    std::cout << "  - Temp positions: " << temp_positions.size() << std::endl;
    std::cout << "  - Temp UVs: " << temp_uvs.size() << std::endl;
    std::cout << "  - Temp normals: " << temp_normals.size() << std::endl;
    std::cout << "  - Final vertices (floats): " << outMesh.vertices.size() << std::endl;
    std::cout << "  - Final indices: " << outMesh.indexCount << std::endl;
    std::cout << "  - Triangles: " << (outMesh.indexCount / 3) << std::endl;

    if (outMesh.vertices.empty() || outMesh.indices.empty()) {
        std::cout << "[OBJ LOADER] ? No valid geometry data generated!" << std::endl;
        return false;
    }

    // Debug: Print first few vertices
    std::cout << "[OBJ LOADER] First vertex (8 floats): ";
    for (int i = 0; i < std::min(8, (int)outMesh.vertices.size()); i++) {
        std::cout << outMesh.vertices[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "[OBJ LOADER] ? Mesh loaded successfully!" << std::endl;

    return true;
}

void OBJLoader::setupMesh(OBJMesh& mesh) {
    std::cout << "[OBJ LOADER] Setting up OpenGL buffers..." << std::endl;

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);

    glBindVertexArray(mesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float),
        mesh.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int),
        mesh.indices.data(), GL_STATIC_DRAW);

    // Vertex layout: Position (3) + Normal (3) + UV (2) = 8 floats per vertex

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
        (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // UV attribute (location = 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
        (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // Check for OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cout << "[OBJ LOADER] ? OpenGL error during setup: " << err << std::endl;
    }
    else {
        std::cout << "[OBJ LOADER] ? Mesh setup complete!" << std::endl;
        std::cout << "  - VAO: " << mesh.VAO << std::endl;
        std::cout << "  - VBO: " << mesh.VBO << std::endl;
        std::cout << "  - EBO: " << mesh.EBO << std::endl;
        std::cout << "  - Index count: " << mesh.indexCount << std::endl;
    }
}