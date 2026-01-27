#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "TextRenderer.h"

struct Target {
    float x, y;
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
    unsigned int texturedCircleShaderProgram;
    unsigned int VAO, VBO;
    unsigned int textVAO, textVBO;
    unsigned int textureVAO, textureVBO;
    unsigned int studentInfoTexture;
    unsigned int backgroundTexture;
    unsigned int terroristTexture;
    unsigned int counterTexture;
    unsigned int heartTexture;
    unsigned int emptyHeartTexture;
    unsigned int akTexture;
    unsigned int uspTexture;
    TextRenderer* textRenderer;
    
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
    
    void initBuffers();
    void spawnTarget();
    void updateDifficulty();
    void drawCircle(float x, float y, float radius, unsigned int texture);
    void drawRect(float x, float y, float width, float height, float r, float g, float b, float alpha = 1.0f);
    void drawTexture(float x, float y, float width, float height, unsigned int texture, float alpha = 1.0f);
    bool isPointInRect(float px, float py, float rx, float ry, float rw, float rh);
    
public:
    AimTrainer(int width, int height);
    ~AimTrainer();
    
    void update(float deltaTime);
    void render();
    void handleMouseClick(double mouseX, double mouseY);
    void handleMousePress(double mouseX, double mouseY);
    void handleMouseRelease();
    void setFireMode(FireMode mode);
    void restart();
    bool isGameOver() const { return gameOver; }
    bool shouldExit() const;
};
