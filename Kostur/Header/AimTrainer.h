#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "TextRenderer.h"
#include "TextRenderer3D.h"
#include "Camera.h"

struct Target {
    glm::vec3 position;  // 3D pozicija
    float radius;
    float lifeTime;
    float maxLifeTime;
    bool active;
    unsigned int texture;
};

struct Button {
    float x, y;
    float width, height;
};

enum class FireMode {
    USP,
    AK47
};

class AimTrainer {
private:
    unsigned int rectShaderProgram;
    unsigned int textureShaderProgram;
    unsigned int freetypeShaderProgram;
    unsigned int text3DShaderProgram;
    unsigned int texturedCircleShaderProgram;
    unsigned int sphere3DShaderProgram;
    unsigned int gameOverShaderProgram;  // Dedicated Game Over shader
    unsigned int roomShaderProgram;
    unsigned int VAO, VBO;
    unsigned int textVAO, textVBO;
    unsigned int textureVAO, textureVBO;
    unsigned int sphereVAO, sphereVBO, sphereEBO;
    unsigned int cylinderVAO, cylinderVBO, cylinderEBO;
    unsigned int roomVAO, roomVBO, roomEBO;
    unsigned int studentInfoTexture;
    unsigned int backgroundTexture;
    unsigned int terroristTexture;
    unsigned int counterTexture;
    unsigned int heartTexture;
    unsigned int emptyHeartTexture;
    unsigned int akTexture;
    unsigned int uspTexture;
    unsigned int wallTexture;
    unsigned int floorTexture;
    unsigned int ceilingTexture;
    TextRenderer* textRenderer;
    TextRenderer3D* textRenderer3D;
    Camera* camera;  // 3D kamera
    
    std::vector<Target> targets;
    Button restartButton;
    Button exitButton;
    int score;
    int lives;
    int maxLives;
    double startTime;
    double lastHitTime;
    double totalHitTime;
    int hitCount;
    bool gameOver;
    float spawnTimer;
    float spawnInterval;
    float initialSpawnInterval;
    float minSpawnInterval;
    float targetLifeTimeMultiplier;
    float minTargetLifeTime;
    int windowWidth;
    int windowHeight;
    
    double gameOverTime;
    double survivalTime;
    double avgHitSpeed;
    bool exitRequested;
    int totalClicks;
    
    FireMode fireMode;
    bool isMousePressed;
    double lastShotTime;
    double fireRate;
    
    bool depthTestEnabled;   // Toggle za depth test
    bool faceCullingEnabled; // Toggle za face culling
    bool gameOverPrintedOnce; // Flag za Game Over console output
    
    void initBuffers();
    void initSphere();
    void initCylinder();
    void initRoom();
    void spawnTarget();
    void updateDifficulty();
    void drawSphere3D(const glm::vec3& position, float radius, unsigned int texture);
    void drawCylinder3D(const glm::vec3& position, float radius, float depth, unsigned int texture);
    void drawRoom();
    void drawCircle(float x, float y, float radius, unsigned int texture);
    void drawRect(float x, float y, float width, float height, float r, float g, float b, float alpha = 1.0f);
    void drawTexture(float x, float y, float width, float height, unsigned int texture, float alpha = 1.0f);
    bool isPointInRect(float px, float py, float rx, float ry, float rw, float rh);
    bool raySphereIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDir, 
                                const glm::vec3& sphereCenter, float sphereRadius);
    
public:
    AimTrainer(int width, int height);
    ~AimTrainer();
    
    void update(float deltaTime);
    void render();
    void handleMouseClick(double mouseX, double mouseY);
    void handleMousePress(double mouseX, double mouseY);
    void handleMouseRelease();
    void setFireMode(FireMode mode);
    void toggleDepthTest();   // Toggle depth test
    void toggleFaceCulling(); // Toggle face culling
    void processMouseMovement(float xoffset, float yoffset);
    void restart();
    bool isGameOver() const { return gameOver; }
    bool shouldExit() const;
};
