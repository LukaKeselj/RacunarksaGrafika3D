#include "../Header/AimTrainer.h"
#include "../Header/Util.h"
#include "../Header/TextRenderer3D.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>

AimTrainer::AimTrainer(int width, int height) 
    : score(0), lives(3), maxLives(3), gameOver(false), spawnTimer(0.0f), 
      spawnInterval(1.5f), initialSpawnInterval(1.5f), minSpawnInterval(0.3f),
      targetLifeTimeMultiplier(1.0f), minTargetLifeTime(0.4f),
      windowWidth(width), windowHeight(height), hitCount(0), totalHitTime(0.0),
      lastHitTime(0.0), gameOverTime(0.0), survivalTime(0.0), avgHitSpeed(0.0),
      textRenderer(nullptr), textRenderer3D(nullptr), camera(nullptr), exitRequested(false), totalClicks(0),
      fireMode(FireMode::USP), isMousePressed(false), lastShotTime(0.0), fireRate(0.1),
      depthTestEnabled(true), faceCullingEnabled(true), gameOverPrintedOnce(false)
{
    srand(static_cast<unsigned int>(time(nullptr)));
    
    rectShaderProgram = createShader("Shaders/rect.vert", "Shaders/rect.frag");
    textureShaderProgram = createShader("Shaders/texture.vert", "Shaders/texture.frag");
    freetypeShaderProgram = createShader("Shaders/freetype.vert", "Shaders/freetype.frag");
    text3DShaderProgram = createShader("Shaders/text3d.vert", "Shaders/text3d.frag");
    texturedCircleShaderProgram = createShader("Shaders/textured_circle.vert", "Shaders/textured_circle.frag");
    sphere3DShaderProgram = createShader("Shaders/sphere3d.vert", "Shaders/sphere3d.frag");
    gameOverShaderProgram = createShader("Shaders/gameover.vert", "Shaders/gameover.frag");
    roomShaderProgram = createShader("Shaders/room.vert", "Shaders/room.frag");
    
    std::cout << "3D Text Shader Program ID: " << text3DShaderProgram << std::endl;
    
    camera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));
    
    // Postavi kameru da gleda prema centru spawn zone meta
    // Meta se spawuju u: x: [-3, 3], y: [-2, 2], z: [-5, -8]
    // Centar spawn zone: (0, 0, -6.5)
    glm::vec3 spawnZoneCenter(0.0f, 0.0f, -6.5f);
    camera->lookAt(spawnZoneCenter);
    
    textRenderer = new TextRenderer(freetypeShaderProgram, windowWidth, windowHeight);
    if (!textRenderer->loadFont("C:/Windows/Fonts/arial.ttf", 48)) {
        std::cout << "Warning: Failed to load Arial font" << std::endl;
    }
    
    textRenderer3D = new TextRenderer3D(text3DShaderProgram);
    if (!textRenderer3D->loadFont("C:/Windows/Fonts/arial.ttf", 48)) {
        std::cout << "Warning: Failed to load Arial font for 3D" << std::endl;
    } else {
        std::cout << "3D Text Renderer initialized successfully!" << std::endl;
    }
    
    studentInfoTexture = loadImageToTexture("Resources/indeks.png");
    backgroundTexture = loadImageToTexture("Resources/mirage.png");
    terroristTexture = loadImageToTexture("Resources/terrorist.png");
    counterTexture = loadImageToTexture("Resources/counter.png");
    heartTexture = loadImageToTexture("Resources/heart.png");
    emptyHeartTexture = loadImageToTexture("Resources/empty-heart.png");
    akTexture = loadImageToTexture("Resources/ak.png");
    uspTexture = loadImageToTexture("Resources/usp.png");
    wallTexture = loadImageToTexture("Resources/wall.png");
    floorTexture = loadImageToTexture("Resources/floor.jpg");
    ceilingTexture = loadImageToTexture("Resources/ceiling.png");
    
    std::cout << "=== Texture Loading ===" << std::endl;
    std::cout << "Wall texture ID: " << wallTexture << std::endl;
    if (wallTexture == 0) {
        std::cout << "ERROR: Wall texture failed to load!" << std::endl;
    } else {
        std::cout << "Wall texture loaded successfully!" << std::endl;
    }
    std::cout << "Floor texture ID: " << floorTexture << std::endl;
    if (floorTexture == 0) {
        std::cout << "ERROR: Floor texture failed to load!" << std::endl;
    } else {
        std::cout << "Floor texture loaded successfully!" << std::endl;
    }
    std::cout << "Ceiling texture ID: " << ceilingTexture << std::endl;
    if (ceilingTexture == 0) {
        std::cout << "ERROR: Ceiling texture failed to load!" << std::endl;
    } else {
        std::cout << "Ceiling texture loaded successfully!" << std::endl;
    }

    initBuffers();
    initSphere();
    initCylinder();
    initRoom();
    
    startTime = glfwGetTime();
    lastHitTime = startTime;
    
    float boxWidth = 450;
    float boxHeight = 400;
    float boxX = (windowWidth - boxWidth) / 2;
    float boxY = (windowHeight - boxHeight) / 2;
    
    restartButton.x = boxX + 105;
    restartButton.y = boxY + 270;
    restartButton.width = 240;
    restartButton.height = 50;
    
    exitButton.x = boxX + 105;
    exitButton.y = boxY + 330;
    exitButton.width = 240;
    exitButton.height = 50;
    
    std::cout << "=== 3D AIM TRAINER ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  D - Toggle Depth Test" << std::endl;
    std::cout << "  F - Toggle Face Culling" << std::endl;
    std::cout << "  1 - AK-47 (Full-Auto)" << std::endl;
    std::cout << "  2 - USP-S (Semi-Auto)" << std::endl;
    std::cout << "  R - Restart (when game over)" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
}

AimTrainer::~AimTrainer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);
    glDeleteVertexArrays(1, &textureVAO);
    glDeleteBuffers(1, &textureVBO);
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);
    glDeleteVertexArrays(1, &cylinderVAO);
    glDeleteBuffers(1, &cylinderVBO);
    glDeleteBuffers(1, &cylinderEBO);
    glDeleteVertexArrays(1, &roomVAO);
    glDeleteBuffers(1, &roomVBO);
    glDeleteBuffers(1, &roomEBO);
    glDeleteProgram(rectShaderProgram);
    glDeleteProgram(textureShaderProgram);
    glDeleteProgram(freetypeShaderProgram);
    glDeleteProgram(text3DShaderProgram);
    glDeleteProgram(texturedCircleShaderProgram);
    glDeleteProgram(sphere3DShaderProgram);
    glDeleteProgram(gameOverShaderProgram);
    glDeleteProgram(roomShaderProgram);
    glDeleteTextures(1, &studentInfoTexture);
    glDeleteTextures(1, &backgroundTexture);
    glDeleteTextures(1, &terroristTexture);
    glDeleteTextures(1, &counterTexture);
    glDeleteTextures(1, &heartTexture);
    glDeleteTextures(1, &emptyHeartTexture);
    glDeleteTextures(1, &akTexture);
    glDeleteTextures(1, &uspTexture);
    glDeleteTextures(1, &wallTexture);
    glDeleteTextures(1, &floorTexture);
    glDeleteTextures(1, &ceilingTexture);
    if (textRenderer) delete textRenderer;
    if (textRenderer3D) delete textRenderer3D;
    if (camera) delete camera;
}

void AimTrainer::initBuffers() {
    float vertices[] = {
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f
    };
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, NULL, GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glGenVertexArrays(1, &textureVAO);
    glGenBuffers(1, &textureVBO);
    
    glBindVertexArray(textureVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, NULL, GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

void AimTrainer::spawnTarget() {
    Target target;
    target.radius = 1.0f;
    
    // Dimenzije prostorije
    float roomWidth = 20.0f;
    float roomHeight = 10.0f;
    float roomDepth = 20.0f;
    
    float halfWidth = roomWidth / 2.0f;
    float halfHeight = roomHeight / 2.0f;
    float halfDepth = roomDepth / 2.0f;
    
    // Margine da se mete sigurno vide cele
    float marginX = target.radius * 3.5f;
    float marginY = target.radius * 3.5f;
    
    glm::vec3 cameraPos = camera->getPosition();
    glm::vec3 cameraFront = camera->getFront();
    glm::vec3 cameraRight = camera->getRight();
    glm::vec3 cameraUp = camera->getUp();
    
    // Pokušaj maksimalno 20 puta da na?eš dobru poziciju u FOV-u
    int maxAttempts = 20;
    int attempt = 0;
    bool foundValidPosition = false;
    
    glm::vec3 targetPos;
    
    while (!foundValidPosition && attempt < maxAttempts) {
        attempt++;
        
        // Odaberi random zid (0=front, 1=back, 2=left, 3=right)
        int wallChoice = rand() % 4;
        
        switch (wallChoice) {
            case 0: // Front wall (z = -halfDepth)
                targetPos.x = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * (halfWidth - marginX);
                targetPos.y = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * (halfHeight - marginY);
                targetPos.z = -halfDepth + 0.5f;
                break;
                
            case 1: // Back wall (z = halfDepth)
                targetPos.x = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * (halfWidth - marginX);
                targetPos.y = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * (halfHeight - marginY);
                targetPos.z = halfDepth - 0.5f;
                break;
                
            case 2: // Left wall (x = -halfWidth)
                targetPos.x = -halfWidth + 0.5f;
                targetPos.y = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * (halfHeight - marginY);
                targetPos.z = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * (halfDepth - marginX);
                break;
                
            case 3: // Right wall (x = halfWidth)
                targetPos.x = halfWidth - 0.5f;
                targetPos.y = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * (halfHeight - marginY);
                targetPos.z = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * (halfDepth - marginX);
                break;
        }
        
        // Proveri da li je meta u FOV-u kamere
        glm::vec3 toTarget = glm::normalize(targetPos - cameraPos);
        float dotProduct = glm::dot(toTarget, cameraFront);
        
        // Stroža provera FOV-a - ugao manji od 45 stepeni
        // cos(45°) ? 0.707
        if (dotProduct > 0.707f) {
            // Dodatna provera - meta mora biti dovoljno blizu centralnom pravcu pogleda
            // Projekcija na desnu i gornju osu
            float rightOffset = glm::dot(toTarget, cameraRight);
            float upOffset = glm::dot(toTarget, cameraUp);
            
            // Proveri da li je meta u "vidljivom pravougaoniku" FOV-a
            // Ograni?enje na približno 45° FOV horizontalno i vertikalno
            if (std::abs(rightOffset) < 0.5f && std::abs(upOffset) < 0.4f) {
                foundValidPosition = true;
                
                std::cout << "Target spawned on wall " << wallChoice << " at: (" 
                          << targetPos.x << ", " << targetPos.y << ", " << targetPos.z 
                          << ") after " << attempt << " attempts" << std::endl;
            }
        }
    }
    
    // Ako nismo našli poziciju u FOV-u posle svih pokušaja, ne spawuj metu
    if (!foundValidPosition) {
        std::cout << "Failed to spawn target in FOV after " << maxAttempts << " attempts" << std::endl;
        return;
    }
    
    // Postavi poziciju i ostale parametre
    target.position = targetPos;
    target.maxLifeTime = (2.0f + static_cast<float>(rand()) / RAND_MAX * 2.0f) * targetLifeTimeMultiplier;
    target.lifeTime = target.maxLifeTime;
    target.active = true;
    target.texture = (rand() % 2 == 0) ? terroristTexture : counterTexture;
    
    targets.push_back(target);
}

void AimTrainer::restart() {
    score = 0;
    lives = 3;
    gameOver = false;
    spawnTimer = 0.0f;
    spawnInterval = initialSpawnInterval;
    targetLifeTimeMultiplier = 1.0f;
    hitCount = 0;
    totalHitTime = 0.0;
    gameOverTime = 0.0;
    survivalTime = 0.0;
    avgHitSpeed = 0.0;
    totalClicks = 0;
    gameOverPrintedOnce = false;
    
    targets.clear();
    
    startTime = glfwGetTime();
    lastHitTime = startTime;
    
    GLFWwindow* window = glfwGetCurrentContext();
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    std::cout << "\n=== NOVA IGRA ===" << std::endl;
}

void AimTrainer::update(float deltaTime) {
    if (gameOver) return;
    
    if (fireMode == FireMode::AK47 && isMousePressed) {
        double currentTime = glfwGetTime();
        if (currentTime - lastShotTime >= fireRate) {
            GLFWwindow* window = glfwGetCurrentContext();
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            handleMouseClick(mouseX, mouseY);
            lastShotTime = currentTime;
        }
    }
    
    updateDifficulty();
    
    spawnTimer += deltaTime;
    if (spawnTimer >= spawnInterval) {
        spawnTarget();
        spawnTimer = 0.0f;
    }
    
    for (auto& target : targets) {
        if (target.active) {
            target.lifeTime -= deltaTime;
            if (target.lifeTime <= 0.0f) {
                target.active = false;
                lives--;
                std::cout << "MISS! Zivoti preostali: " << lives << std::endl;
                
                if (lives <= 0) {
                    gameOver = true;
                    gameOverTime = glfwGetTime();
                    survivalTime = gameOverTime - startTime;
                    if (hitCount > 0) {
                        avgHitSpeed = totalHitTime / hitCount;
                    }
                    
                    GLFWwindow* window = glfwGetCurrentContext();
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
            }
        }
    }
    
    targets.erase(
        std::remove_if(targets.begin(), targets.end(), 
            [](const Target& t) { return !t.active; }),
        targets.end()
    );
}

void AimTrainer::updateDifficulty() {
    double currentTime = glfwGetTime();
    double elapsedTime = currentTime - startTime;
    
    float difficultyFactor = static_cast<float>(elapsedTime) / 5.0f;
    
    spawnInterval = initialSpawnInterval - (difficultyFactor * 0.2f);
    if (spawnInterval < minSpawnInterval) {
        spawnInterval = minSpawnInterval;
    }
    
    targetLifeTimeMultiplier = 1.0f - (difficultyFactor * 0.12f);
    if (targetLifeTimeMultiplier < minTargetLifeTime) {
        targetLifeTimeMultiplier = minTargetLifeTime;
    }
}

void AimTrainer::render() {
    if (!gameOver) {
        // Draw room environment samo tokom igre
        drawRoom();
        
        // Then draw 3D targets (cilindri umesto sfera)
        for (const auto& target : targets) {
            if (target.active) {
                drawCylinder3D(target.position, target.radius, 0.15f, target.texture);
            }
        }
    }
    
    // Disable depth test for all 2D overlays
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(rectShaderProgram);
    
    float projection[16] = {
        2.0f / windowWidth, 0.0f, 0.0f, 0.0f,
        0.0f, -2.0f / windowHeight, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f
    };
    
    int projLoc = glGetUniformLocation(rectShaderProgram, "uProjection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
    
    if (!gameOver) {
        double currentTime = glfwGetTime();
        double elapsed = currentTime - startTime;
        
        // Draw statistics panel background (upper left)
        drawRect(10, 10, 700, 80, 0.0f, 0.0f, 0.0f, 0.7f);
        drawRect(12, 12, 696, 76, 0.2f, 0.2f, 0.2f, 0.8f);
        
        // Draw lives (hearts)
        for (int i = 0; i < maxLives; i++) {
            if (i < lives) {
                drawTexture(20 + i * 35, 22, 28, 28, heartTexture);
            } else {
                drawTexture(20 + i * 35, 22, 28, 28, emptyHeartTexture);
            }
        }
        
        // Time display
        int minutes = static_cast<int>(elapsed) / 60;
        int seconds = static_cast<int>(elapsed) % 60;
        int centiseconds = static_cast<int>((elapsed - static_cast<int>(elapsed)) * 100) % 100;
        
        std::stringstream timeStr;
        timeStr << std::setfill('0') << std::setw(2) << minutes << ":" 
                << std::setw(2) << seconds << ":" 
                << std::setw(2) << centiseconds;
        
        textRenderer->renderText(timeStr.str(), 130, 35, 0.6f, 0.2f, 1.0f, 1.0f);
        
        // Hits display
        std::stringstream statsStr;
        statsStr << "Hits: " << score << "/" << totalClicks;
        textRenderer->renderText(statsStr.str(), 330, 35, 0.5f, 0.4f, 1.0f, 0.4f);
        
        // Average speed
        double avgSpeed = 0.0;
        if (hitCount > 0) {
            avgSpeed = totalHitTime / hitCount;
        }
        
        std::stringstream speedStr;
        speedStr << "Speed: " << std::fixed << std::setprecision(2) << avgSpeed << " s";
        textRenderer->renderText(speedStr.str(), 480, 35, 0.45f, 1.0f, 0.8f, 0.3f);
        
        // Weapon mode
        std::string modeStr = (fireMode == FireMode::USP) ? "USP" : "AK-47";
        float modeR = (fireMode == FireMode::USP) ? 0.7f : 1.0f;
        float modeG = (fireMode == FireMode::USP) ? 0.7f : 0.5f;
        float modeB = (fireMode == FireMode::USP) ? 0.7f : 0.2f;
        textRenderer->renderText(modeStr, 630, 35, 0.4f, modeR, modeG, modeB);
        
        // Debug info
        std::stringstream debugStr;
        debugStr << "Targets: " << targets.size();
        textRenderer->renderText(debugStr.str(), 20, 100, 0.5f, 1.0f, 1.0f, 0.0f);
        
        // Console output
        float accuracy = 0.0f;
        if (totalClicks > 0) {
            accuracy = (static_cast<float>(score) / static_cast<float>(totalClicks)) * 100.0f;
        }
        
        std::cout << "Time: " << minutes << ":" << (seconds < 10 ? "0" : "") << seconds << ":" << (centiseconds < 10 ? "0" : "") << centiseconds
                  << " | Zivoti: " << lives << "/" << maxLives 
                  << " | Pogodaka: " << score << "/" << totalClicks << " (" << std::fixed << std::setprecision(1) << accuracy << "%)"
                  << " | Avg Speed: " << avgSpeed << "s         \r" << std::flush;
        
        // Student info (lower left)
        float infoWidth = 400.0f;
        float infoHeight = 200.0f;
        float infoX = 20;
        float infoY = windowHeight - infoHeight - 20;
        
        float padding = 10.0f;
        drawRect(infoX - padding, infoY - padding, infoWidth + 2 * padding, infoHeight + 2 * padding, 0.1f, 0.1f, 0.1f, 0.5f);
        
        drawTexture(infoX, infoY, infoWidth, infoHeight, studentInfoTexture, 1.0f);
        
        // Weapon display (lower right)
        float weaponWidth = 300.0f;
        float weaponHeight = 150.0f;
        float weaponX = windowWidth - weaponWidth - 20;
        float weaponY = windowHeight - weaponHeight - 20;
        
        unsigned int weaponTexture = (fireMode == FireMode::USP) ? uspTexture : akTexture;
        drawTexture(weaponX, weaponY, weaponWidth, weaponHeight, weaponTexture);
    } else {
        // GAME OVER - DISABLE ALL 3D RENDERING STATES
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // CLEAR DEPTH BUFFER to ensure Game Over is drawn on top
        glClear(GL_DEPTH_BUFFER_BIT);
        
        // Use rect shader with proper 2D projection
        glUseProgram(rectShaderProgram);
        
        float projection[16] = {
            2.0f / windowWidth, 0.0f, 0.0f, 0.0f,
            0.0f, -2.0f / windowHeight, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f, 1.0f
        };
        
        int projLoc = glGetUniformLocation(rectShaderProgram, "uProjection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
        
        // Dark overlay
        drawRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight), 0.0f, 0.0f, 0.0f, 0.7f);
        
        float boxWidth = 450;
        float boxHeight = 400;
        float boxX = (windowWidth - boxWidth) / 2;
        float boxY = (windowHeight - boxHeight) / 2;
        
        // Outer black box
        drawRect(boxX, boxY, boxWidth, boxHeight, 0.0f, 0.0f, 0.0f, 1.0f);
        // Inner gray box
        drawRect(boxX + 2, boxY + 2, boxWidth - 4, boxHeight - 4, 0.3f, 0.3f, 0.3f, 1.0f);
        
        // RESTART button (green)
        drawRect(restartButton.x, restartButton.y, restartButton.width, restartButton.height, 0.2f, 0.8f, 0.2f, 1.0f);
        
        // EXIT button (red)
        drawRect(exitButton.x, exitButton.y, exitButton.width, exitButton.height, 0.8f, 0.2f, 0.2f, 1.0f);
        
        // TEXT
        std::string gameOverText = "GAME OVER";
        float gameOverWidth = textRenderer->getTextWidth(gameOverText, 1.0f);
        float gameOverX = boxX + (boxWidth - gameOverWidth) / 2;
        textRenderer->renderText(gameOverText, gameOverX, boxY + 50, 1.0f, 1.0f, 0.2f, 0.2f);
        
        int survivalMinutes = static_cast<int>(survivalTime) / 60;
        int survivalSeconds = static_cast<int>(survivalTime) % 60;
        std::stringstream timeText;
        timeText << "Time: " << survivalMinutes << ":" << std::setfill('0') << std::setw(2) << survivalSeconds;
        
        float timeWidth = textRenderer->getTextWidth(timeText.str(), 0.5f);
        float timeX = boxX + (boxWidth - timeWidth) / 2;
        textRenderer->renderText(timeText.str(), timeX, boxY + 110, 0.5f, 0.8f, 0.8f, 1.0f);
        
        float accuracy = 0.0f;
        if (totalClicks > 0) {
            accuracy = (static_cast<float>(score) / static_cast<float>(totalClicks)) * 100.0f;
        }
        
        std::stringstream accuracyText;
        accuracyText << "Accuracy: " << std::fixed << std::setprecision(1) << accuracy << "%";
        float accuracyWidth = textRenderer->getTextWidth(accuracyText.str(), 0.5f);
        float accuracyX = boxX + (boxWidth - accuracyWidth) / 2;
        textRenderer->renderText(accuracyText.str(), accuracyX, boxY + 150, 0.5f, 0.4f, 1.0f, 0.4f);
        
        std::stringstream hitsText;
        hitsText << "Hits: " << score << " / " << totalClicks;
        float hitsWidth = textRenderer->getTextWidth(hitsText.str(), 0.45f);
        float hitsX = boxX + (boxWidth - hitsWidth) / 2;
        textRenderer->renderText(hitsText.str(), hitsX, boxY + 185, 0.45f, 0.9f, 0.9f, 0.9f);
        
        double avgSpeed = 0.0;
        if (hitCount > 0) {
            avgSpeed = totalHitTime / hitCount;
        }
        
        std::stringstream speedText;
        speedText << "Speed: " << std::fixed << std::setprecision(2) << avgSpeed << " s";
        float speedWidth = textRenderer->getTextWidth(speedText.str(), 0.45f);
        float speedX = boxX + (boxWidth - speedWidth) / 2;
        textRenderer->renderText(speedText.str(), speedX, boxY + 220, 0.45f, 1.0f, 0.8f, 0.3f);
        
        float restartTextWidth = textRenderer->getTextWidth("RESTART", 0.5f);
        float restartTextX = restartButton.x + (restartButton.width - restartTextWidth) / 2;
        textRenderer->renderText("RESTART", restartTextX, restartButton.y + 30, 0.5f, 1.0f, 1.0f, 1.0f);
        
        float exitTextWidth = textRenderer->getTextWidth("EXIT", 0.5f);
        float exitTextX = exitButton.x + (exitButton.width - exitTextWidth) / 2;
        textRenderer->renderText("EXIT", exitTextX, exitButton.y + 30, 0.5f, 1.0f, 1.0f, 1.0f);
        
        if (!gameOverPrintedOnce) {
            std::cout << "\n\n=== GAME OVER ===" << std::endl;
            std::cout << "Vreme prezivljanja: " << (int)survivalTime << "s" << std::endl;
            std::cout << "Ukupno pogodaka: " << score << std::endl;
            std::cout << "Prosecna brzina pogadjanja: " << avgHitSpeed << "s" << std::endl;
            std::cout << "\nPritisni 'R' za restart ili klikni na zeleno dugme" << std::endl;
            std::cout << "Pritisni 'ESC' za izlaz ili klikni na crveno dugme" << std::endl;
            std::cout << "================\n" << std::endl;
            gameOverPrintedOnce = true;
        }
    }
    
    // Re-enable depth test if it was enabled
    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
    
    // Draw crosshair (always on top, 2D overlay)
    if (!gameOver) {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glUseProgram(rectShaderProgram);
        
        float projection[16] = {
            2.0f / windowWidth, 0.0f, 0.0f, 0.0f,
            0.0f, -2.0f / windowHeight, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f, 1.0f
        };
        
        int projLoc = glGetUniformLocation(rectShaderProgram, "uProjection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
        
        float centerX = windowWidth / 2.0f;
        float centerY = windowHeight / 2.0f;
        float crosshairSize = 30.0f;
        float crosshairThickness = 6.0f;
        float crosshairGap = 12.0f;
        
        // Left line
        drawRect(centerX - crosshairSize - crosshairGap, centerY - crosshairThickness/2, crosshairSize, crosshairThickness, 0.0f, 1.0f, 0.0f, 1.0f);
        // Right line
        drawRect(centerX + crosshairGap, centerY - crosshairThickness/2, crosshairSize, crosshairThickness, 0.0f, 1.0f, 0.0f, 1.0f);
        // Top line
        drawRect(centerX - crosshairThickness/2, centerY - crosshairSize - crosshairGap, crosshairThickness, crosshairSize, 0.0f, 1.0f, 0.0f, 1.0f);
        // Bottom line
        drawRect(centerX - crosshairThickness/2, centerY + crosshairGap, crosshairThickness, crosshairSize, 0.0f, 1.0f, 0.0f, 1.0f);
        // Center dot
        drawRect(centerX - 4, centerY - 4, 8, 8, 1.0f, 0.0f, 0.0f, 1.0f);
        
        if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
        if (faceCullingEnabled) glEnable(GL_CULL_FACE);
    }
}

void AimTrainer::drawRect(float x, float y, float width, float height, float r, float g, float b, float alpha) {
    glUseProgram(rectShaderProgram);
    
    float projection[16] = {
        2.0f / windowWidth, 0.0f, 0.0f, 0.0f,
        0.0f, -2.0f / windowHeight, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f
    };
    
    int projLoc = glGetUniformLocation(rectShaderProgram, "uProjection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
    
    glBindVertexArray(textVAO);
    
    float vertices[] = {
        x, y,
        x + width, y,
        x + width, y + height,
        x, y,
        x + width, y + height,
        x, y + height
    };
    
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    
    int colorLoc = glGetUniformLocation(rectShaderProgram, "uColor");
    glUniform3f(colorLoc, r, g, b);
    
    int alphaLoc = glGetUniformLocation(rectShaderProgram, "uAlpha");
    glUniform1f(alphaLoc, alpha);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void AimTrainer::drawTexture(float x, float y, float width, float height, unsigned int texture, float alpha) {
    glUseProgram(textureShaderProgram);
    
    float projection[16] = {
        2.0f / windowWidth, 0.0f, 0.0f, 0.0f,
        0.0f, -2.0f / windowHeight, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f
    };
    
    int projLoc = glGetUniformLocation(textureShaderProgram, "uProjection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
    
    int alphaLoc = glGetUniformLocation(textureShaderProgram, "uAlpha");
    glUniform1f(alphaLoc, alpha);
    
    float vertices[] = {
        x, y, 0.0f, 1.0f,
        x + width, y, 1.0f, 1.0f,
        x + width, y + height, 1.0f, 0.0f,
        x, y, 0.0f, 1.0f,
        x + width, y + height, 1.0f, 0.0f,
        x, y + height, 0.0f, 0.0f
    };
    
    glBindVertexArray(textureVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    int texLoc = glGetUniformLocation(textureShaderProgram, "uTexture");
    glUniform1i(texLoc, 0);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void AimTrainer::handleMouseClick(double mouseX, double mouseY) {
    if (gameOver) {
        if (isPointInRect(static_cast<float>(mouseX), static_cast<float>(mouseY), 
                         restartButton.x, restartButton.y, restartButton.width, restartButton.height)) {
            restart();
        }
        else if (isPointInRect(static_cast<float>(mouseX), static_cast<float>(mouseY), 
                              exitButton.x, exitButton.y, exitButton.width, exitButton.height)) {
            exitRequested = true;
        }
        return;
    }
    
    totalClicks++;
    
    double currentTime = glfwGetTime();
    
    glm::vec3 rayOrigin = camera->getPosition();
    glm::vec3 rayDir = camera->getFront();
    
    bool hit = false;
    for (auto& target : targets) {
        if (target.active) {
            if (raySphereIntersection(rayOrigin, rayDir, target.position, target.radius)) {
                target.active = false;
                score++;
                hit = true;
                
                double timeSinceLastHit = currentTime - lastHitTime;
                totalHitTime += timeSinceLastHit;
                hitCount++;
                lastHitTime = currentTime;
                
                std::cout << "\nHIT! Pogodaka: " << score << std::endl;
                break;
            }
        }
    }
}

void AimTrainer::handleMousePress(double mouseX, double mouseY) {
    isMousePressed = true;
    lastShotTime = glfwGetTime();
    handleMouseClick(mouseX, mouseY);
}

void AimTrainer::handleMouseRelease() {
    isMousePressed = false;
}

void AimTrainer::setFireMode(FireMode mode) {
    fireMode = mode;
    if (mode == FireMode::USP) {
        std::cout << "\n[WEAPON] USP-S (Semi-Auto)" << std::endl;
    } else {
        std::cout << "\n[WEAPON] AK-47 (Full-Auto)" << std::endl;
    }
}

bool AimTrainer::shouldExit() const {
    return exitRequested;
}

bool AimTrainer::isPointInRect(float px, float py, float rx, float ry, float rw, float rh) {
    return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
}

void AimTrainer::initSphere() {
    const int segments = 32;
    const int rings = 16;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    for (int ring = 0; ring <= rings; ring++) {
        float phi = glm::pi<float>() * static_cast<float>(ring) / static_cast<float>(rings);
        for (int seg = 0; seg <= segments; seg++) {
            float theta = 2.0f * glm::pi<float>() * static_cast<float>(seg) / static_cast<float>(segments);
            
            float x = std::sin(phi) * std::cos(theta);
            float y = std::cos(phi);
            float z = std::sin(phi) * std::sin(theta);
            
            float u = static_cast<float>(seg) / static_cast<float>(segments);
            float v = static_cast<float>(ring) / static_cast<float>(rings);
            
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(u);
            vertices.push_back(v);
        }
    }
    
    for (int ring = 0; ring < rings; ring++) {
        for (int seg = 0; seg < segments; seg++) {
            int current = ring * (segments + 1) + seg;
            int next = current + segments + 1;
            
            indices.push_back(current);
            indices.push_back(next);
            indices.push_back(current + 1);
            
            indices.push_back(current + 1);
            indices.push_back(next);
            indices.push_back(next + 1);
        }
    }
    
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);
    
    glBindVertexArray(sphereVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
    
    std::cout << "3D Sphere initialized with " << indices.size() / 3 << " triangles" << std::endl;
}

void AimTrainer::drawSphere3D(const glm::vec3& position, float radius, unsigned int texture) {
    glUseProgram(sphere3DShaderProgram);
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(radius));
    
    glm::mat4 view = camera->getViewMatrix();
    
    float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    glm::mat4 projection = camera->getProjectionMatrix(aspect);
    
    int modelLoc = glGetUniformLocation(sphere3DShaderProgram, "uModel");
    int viewLoc = glGetUniformLocation(sphere3DShaderProgram, "uView");
    int projLoc = glGetUniformLocation(sphere3DShaderProgram, "uProjection");
    
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    glm::vec3 lightPos(5.0f, 5.0f, 5.0f);
    int lightPosLoc = glGetUniformLocation(sphere3DShaderProgram, "uLightPos");
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
    
    int viewPosLoc = glGetUniformLocation(sphere3DShaderProgram, "uViewPos");
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(camera->getPosition()));
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    int texLoc = glGetUniformLocation(sphere3DShaderProgram, "uTexture");
    glUniform1i(texLoc, 0);
    
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, (32 * 16 * 6), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void AimTrainer::initCylinder() {
    const int segments = 32;
    float radius = 1.0f;
    float halfDepth = 0.05f;
    
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // Front face - centar kruga
    vertices.insert(vertices.end(), {0.0f, 0.0f, halfDepth, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f});
    
    // Back face - centar kruga
    int backCenterIndex = 1;
    vertices.insert(vertices.end(), {0.0f, 0.0f, -halfDepth, 0.0f, 0.0f, -1.0f, 0.5f, 0.5f});
    
    int frontStartIndex = 2;
    
    // Front face vertices - pravilno texture mapiranje za krug
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
        float x = radius * std::cos(theta);
        float y = radius * std::sin(theta);
        
        // Texture koordinate za krug (mapiranje od centra ka ivici)
        float u = 0.5f + 0.5f * std::cos(theta);
        float v = 0.5f + 0.5f * std::sin(theta);
        
        // Front face vertex
        vertices.insert(vertices.end(), {x, y, halfDepth, 0.0f, 0.0f, 1.0f, u, v});
    }
    
    int backStartIndex = frontStartIndex + segments + 1;
    
    // Back face vertices - iste texture koordinate
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
        float x = radius * std::cos(theta);
        float y = radius * std::sin(theta);
        
        float u = 0.5f + 0.5f * std::cos(theta);
        float v = 0.5f + 0.5f * std::sin(theta);
        
        // Back face vertex
        vertices.insert(vertices.end(), {x, y, -halfDepth, 0.0f, 0.0f, -1.0f, u, v});
    }
    
    // Indices za front face
    for (int i = 0; i < segments; i++) {
        indices.push_back(0);
        indices.push_back(frontStartIndex + i);
        indices.push_back(frontStartIndex + i + 1);
    }
    
    // Indices za back face (obrnuti winding)
    for (int i = 0; i < segments; i++) {
        indices.push_back(backCenterIndex);
        indices.push_back(backStartIndex + i + 1);
        indices.push_back(backStartIndex + i);
    }
    
    // Side faces (tanki ivica - opciono, možemo ih izostaviti)
    int sideStartIndex = backStartIndex + segments + 1;
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
        float x = radius * std::cos(theta);
        float y = radius * std::sin(theta);
        
        // Normala za ivicu pokazuje radijalno prema spolja
        float nx = std::cos(theta);
        float ny = std::sin(theta);
        
        float u = static_cast<float>(i) / static_cast<float>(segments);
        
        // Front edge vertex
        vertices.insert(vertices.end(), {x, y, halfDepth, nx, ny, 0.0f, u, 0.0f});
        // Back edge vertex
        vertices.insert(vertices.end(), {x, y, -halfDepth, nx, ny, 0.0f, u, 1.0f});
    }
    
    // Indices za side faces
    for (int i = 0; i < segments; i++) {
        int current = sideStartIndex + i * 2;
        int next = sideStartIndex + (i + 1) * 2;
        
        indices.push_back(current);
        indices.push_back(next);
        indices.push_back(current + 1);
        
        indices.push_back(current + 1);
        indices.push_back(next);
        indices.push_back(next + 1);
    }
    
    glGenVertexArrays(1, &cylinderVAO);
    glGenBuffers(1, &cylinderVBO);
    glGenBuffers(1, &cylinderEBO);
    
    glBindVertexArray(cylinderVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinderEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
    
    std::cout << "Cylinder initialized with proper circular texture mapping!" << std::endl;
}

void AimTrainer::drawCylinder3D(const glm::vec3& position, float radius, float depth, unsigned int texture) {
    glUseProgram(sphere3DShaderProgram);
    
    // Cilindar uvek gleda ka kameri (billboard effect)
    glm::vec3 cameraPos = camera->getPosition();
    glm::vec3 direction = glm::normalize(cameraPos - position);
    
    // Kalkuliši rotaciju da gleda prema kameri
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(up, direction));
    glm::vec3 newUp = glm::cross(direction, right);
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    
    // Kreiraj rotation matrix da gleda ka kameri
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0] = glm::vec4(right, 0.0f);
    rotation[1] = glm::vec4(newUp, 0.0f);
    rotation[2] = glm::vec4(direction, 0.0f);
    
    model = model * rotation;
    model = glm::scale(model, glm::vec3(radius, radius, depth));
    
    glm::mat4 view = camera->getViewMatrix();
    float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    glm::mat4 projection = camera->getProjectionMatrix(aspect);
    
    int modelLoc = glGetUniformLocation(sphere3DShaderProgram, "uModel");
    int viewLoc = glGetUniformLocation(sphere3DShaderProgram, "uView");
    int projLoc = glGetUniformLocation(sphere3DShaderProgram, "uProjection");
    
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    glm::vec3 lightPos(0.0f, 4.0f, 0.0f);
    int lightPosLoc = glGetUniformLocation(sphere3DShaderProgram, "uLightPos");
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
    
    int viewPosLoc = glGetUniformLocation(sphere3DShaderProgram, "uViewPos");
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(camera->getPosition()));
    
    float currentTime = static_cast<float>(glfwGetTime());
    int timeLoc = glGetUniformLocation(sphere3DShaderProgram, "uTime");
    glUniform1f(timeLoc, currentTime);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    int texLoc = glGetUniformLocation(sphere3DShaderProgram, "uTexture");
    glUniform1i(texLoc, 0);
    
    glBindVertexArray(cylinderVAO);
    // Front face (32 triangles) + Back face (32 triangles) + Side faces (32*2 triangles) = 32 + 32 + 64 = 128 triangles = 384 indices
    glDrawElements(GL_TRIANGLES, 32 * 3 + 32 * 3 + 32 * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void AimTrainer::initRoom() {
    float roomWidth = 20.0f;
    float roomHeight = 10.0f;
    float roomDepth = 20.0f;
    
    float halfWidth = roomWidth / 2.0f;
    float halfHeight = roomHeight / 2.0f;
    float halfDepth = roomDepth / 2.0f;
    
    float texScaleW = 4.0f;
    float texScaleH = 2.0f;
    float texScaleD = 4.0f;
    
    std::vector<float> vertices = {
        // Prednji zid (0-3)
        -halfWidth, -halfHeight, -halfDepth,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         halfWidth, -halfHeight, -halfDepth,  0.0f, 0.0f, 1.0f,  texScaleW, 0.0f,
         halfWidth,  halfHeight, -halfDepth,  0.0f, 0.0f, 1.0f,  texScaleW, texScaleH,
        -halfWidth,  halfHeight, -halfDepth,  0.0f, 0.0f, 1.0f,  0.0f, texScaleH,
        
        // Zadnji zid (4-7)
         halfWidth, -halfHeight, halfDepth,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
        -halfWidth, -halfHeight, halfDepth,  0.0f, 0.0f, -1.0f,  texScaleW, 0.0f,
        -halfWidth,  halfHeight, halfDepth,  0.0f, 0.0f, -1.0f,  texScaleW, texScaleH,
         halfWidth,  halfHeight, halfDepth,  0.0f, 0.0f, -1.0f,  0.0f, texScaleH,
        
        // Levi zid (8-11)
        -halfWidth, -halfHeight, -halfDepth,  1.0f, 0.0f, 0.0f,  texScaleD, 0.0f,
        -halfWidth,  halfHeight, -halfDepth,  1.0f, 0.0f, 0.0f,  texScaleD, texScaleH,
        -halfWidth,  halfHeight,  halfDepth,  1.0f, 0.0f, 0.0f,  0.0f, texScaleH,
        -halfWidth, -halfHeight,  halfDepth,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        
        // Desni zid (12-15)
        halfWidth, -halfHeight, -halfDepth,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        halfWidth, -halfHeight,  halfDepth,  -1.0f, 0.0f, 0.0f,  texScaleD, 0.0f,
        halfWidth,  halfHeight,  halfDepth,  -1.0f, 0.0f, 0.0f,  texScaleD, texScaleH,
        halfWidth,  halfHeight, -halfDepth,  -1.0f, 0.0f, 0.0f,  0.0f, texScaleH,
        
        // Pod (16-19) - FIXED NORMAL to point UP
        -halfWidth, -halfHeight, -halfDepth,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         halfWidth, -halfHeight, -halfDepth,  0.0f, 1.0f, 0.0f,  texScaleW, 0.0f,
         halfWidth, -halfHeight,  halfDepth,  0.0f, 1.0f, 0.0f,  texScaleW, texScaleD,
        -halfWidth, -halfHeight,  halfDepth,  0.0f, 1.0f, 0.0f,  0.0f, texScaleD,
        
        // Plafon (20-23)
        -halfWidth, halfHeight, -halfDepth,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         halfWidth, halfHeight, -halfDepth,  0.0f, -1.0f, 0.0f,  texScaleW, 0.0f,
         halfWidth, halfHeight,  halfDepth,  0.0f, -1.0f, 0.0f,  texScaleW, texScaleD,
        -halfWidth, halfHeight,  halfDepth,  0.0f, -1.0f, 0.0f,  0.0f, texScaleD,
    };
    
    std::vector<unsigned int> indices = {
        // Prednji zid
        0, 1, 2, 0, 2, 3,
        // Zadnji zid
        4, 5, 6, 4, 6, 7,
        // Levi zid
        8, 9, 10, 8, 10, 11,
        // Desni zid
        12, 13, 14, 12, 14, 15,
        // Pod - REVERSED WINDING ORDER
        16, 18, 17, 16, 19, 18,
        // Plafon
        20, 21, 22, 20, 22, 23
    };
    
    glGenVertexArrays(1, &roomVAO);
    glGenBuffers(1, &roomVBO);
    glGenBuffers(1, &roomEBO);
    
    glBindVertexArray(roomVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, roomVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, roomEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
    
    std::cout << "Room initialized with FIXED floor winding order!" << std::endl;
}

void AimTrainer::drawRoom() {
    glUseProgram(roomShaderProgram);
    
    glm::mat4 view = camera->getViewMatrix();
    float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    glm::mat4 projection = camera->getProjectionMatrix(aspect);
    
    int viewLoc = glGetUniformLocation(roomShaderProgram, "uView");
    int projLoc = glGetUniformLocation(roomShaderProgram, "uProjection");
    int modelLoc = glGetUniformLocation(roomShaderProgram, "uModel");
    int wallColorLoc = glGetUniformLocation(roomShaderProgram, "uWallColor");
    int useTextureLoc = glGetUniformLocation(roomShaderProgram, "uUseTexture");
    int lightPosLoc = glGetUniformLocation(roomShaderProgram, "uLightPos");
    int viewPosLoc = glGetUniformLocation(roomShaderProgram, "uViewPos");
    int texLoc = glGetUniformLocation(roomShaderProgram, "uWallTexture");
    
    static bool debugPrinted = false;
    if (!debugPrinted) {
        std::cout << "=== Room Shader Uniforms ===" << std::endl;
        std::cout << "Wall texture ID: " << wallTexture << std::endl;
        std::cout << "Floor texture ID: " << floorTexture << std::endl;
        std::cout << "Ceiling texture ID: " << ceilingTexture << std::endl;
        debugPrinted = true;
    }
    
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    glm::vec3 lightPos(0.0f, 4.0f, 0.0f);
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(camera->getPosition()));
    
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    
    glBindVertexArray(roomVAO);
    
    // Svi zidovi - wall.png
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, wallTexture);
    glUniform1i(texLoc, 0);
    glm::vec3 wallColor(0.8f, 0.8f, 0.8f);
    glUniform3fv(wallColorLoc, 1, glm::value_ptr(wallColor));
    glUniform1i(useTextureLoc, 1);
    
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(6 * sizeof(unsigned int)));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(12 * sizeof(unsigned int)));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(18 * sizeof(unsigned int)));
    
    // Pod - floor.jpg
    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glUniform1i(texLoc, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(24 * sizeof(unsigned int)));
    
    // Plafon - ceiling.png
    glBindTexture(GL_TEXTURE_2D, ceilingTexture);
    glUniform1i(texLoc, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(30 * sizeof(unsigned int)));
    
    glBindVertexArray(0);
}

void AimTrainer::toggleDepthTest() {
    depthTestEnabled = !depthTestEnabled;
    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
        std::cout << "[DEBUG] Depth Test: ON" << std::endl;
    } else {
        glDisable(GL_DEPTH_TEST);
        std::cout << "[DEBUG] Depth Test: OFF" << std::endl;
    }
}

void AimTrainer::toggleFaceCulling() {
    faceCullingEnabled = !faceCullingEnabled;
    if (faceCullingEnabled) {
        glEnable(GL_CULL_FACE);
        std::cout << "[DEBUG] Face Culling: ON" << std::endl;
    } else {
        glDisable(GL_CULL_FACE);
        std::cout << "[DEBUG] Face Culling: OFF" << std::endl;
    }
}

void AimTrainer::processMouseMovement(float xoffset, float yoffset) {
    if (!gameOver && camera) {
        camera->processMouseMovement(xoffset, yoffset);
    }
}

bool AimTrainer::raySphereIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDir, 
                                        const glm::vec3& sphereCenter, float sphereRadius) {
    glm::vec3 oc = rayOrigin - sphereCenter;
    float a = glm::dot(rayDir, rayDir);
    float b = 2.0f * glm::dot(oc, rayDir);
    float c = glm::dot(oc, oc) - sphereRadius * sphereRadius;
    float discriminant = b * b - 4 * a * c;
    
    return discriminant >= 0.0f;
}
