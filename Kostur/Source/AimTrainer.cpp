#include "../Header/AimTrainer.h"
#include "../Header/Util.h"
#include "../Header/OBJLoader.h"
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
    textRenderer(nullptr), camera(nullptr), exitRequested(false), totalClicks(0),
    fireMode(FireMode::USP), isMousePressed(false), lastShotTime(0.0), fireRate(0.1),
    depthTestEnabled(true), faceCullingEnabled(true), gameOverPrintedOnce(false),
    lastRecoilTime(0.0), recoilAmount(0.0f), recoilRecoverySpeed(8.0f)
{
    srand(static_cast<unsigned int>(time(nullptr)));

    rectShaderProgram = createShader("Shaders/rect.vert", "Shaders/rect.frag");
    textureShaderProgram = createShader("Shaders/texture.vert", "Shaders/texture.frag");
    freetypeShaderProgram = createShader("Shaders/freetype.vert", "Shaders/freetype.frag");
    cylinderShaderProgram = createShader("Shaders/sphere3d.vert", "Shaders/sphere3d.frag");
    roomShaderProgram = createShader("Shaders/room.vert", "Shaders/room.frag");
    lightShaderProgram = createShader("Shaders/light.vert", "Shaders/light.frag");
    weaponShaderProgram = createShader("Shaders/sphere3d.vert", "Shaders/sphere3d.frag");

    camera = new Camera(glm::vec3(0.0f, 0.0f, 3.0f));
    glm::vec3 spawnZoneCenter(0.0f, 0.0f, -6.5f);
    camera->lookAt(spawnZoneCenter);

    textRenderer = new TextRenderer(freetypeShaderProgram, windowWidth, windowHeight);
    if (!textRenderer->loadFont("C:/Windows/Fonts/arial.ttf", 48)) {
        std::cout << "Warning: Failed to load Arial font" << std::endl;
    }

    studentInfoTexture = loadImageToTexture("Resources/indeks.png");
    terroristTexture = loadImageToTexture("Resources/terrorist.png");
    counterTexture = loadImageToTexture("Resources/counter.png");
    heartTexture = loadImageToTexture("Resources/heart.png");
    emptyHeartTexture = loadImageToTexture("Resources/empty-heart.png");
    akTexture = loadImageToTexture("Resources/ak.png");
    uspTexture = loadImageToTexture("Resources/usp.png");
    wallTexture = loadImageToTexture("Resources/smooth-white-brick-wall.jpg");
    floorTexture = loadImageToTexture("Resources/floor.jpg");
    ceilingTexture = loadImageToTexture("Resources/ceiling.png");

    initBuffers();
    initCylinder();
    initRoom();
    initLight();
    initWallWeapons();

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
    glDeleteVertexArrays(1, &cylinderVAO);
    glDeleteBuffers(1, &cylinderVBO);
    glDeleteBuffers(1, &cylinderEBO);
    glDeleteVertexArrays(1, &roomVAO);
    glDeleteBuffers(1, &roomVBO);
    glDeleteBuffers(1, &roomEBO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &lightVBO);
    glDeleteBuffers(1, &lightEBO);
    glDeleteProgram(rectShaderProgram);
    glDeleteProgram(textureShaderProgram);
    glDeleteProgram(freetypeShaderProgram);
    glDeleteProgram(cylinderShaderProgram);
    glDeleteProgram(roomShaderProgram);
    glDeleteProgram(lightShaderProgram);
    glDeleteProgram(weaponShaderProgram);
    glDeleteTextures(1, &studentInfoTexture);
    glDeleteTextures(1, &terroristTexture);
    glDeleteTextures(1, &counterTexture);
    glDeleteTextures(1, &heartTexture);
    glDeleteTextures(1, &emptyHeartTexture);
    glDeleteTextures(1, &akTexture);
    glDeleteTextures(1, &uspTexture);
    glDeleteTextures(1, &wallTexture);
    glDeleteTextures(1, &floorTexture);
    glDeleteTextures(1, &ceilingTexture);

    for (auto& weapon : wallWeapons) {
        weapon.mesh.cleanup();
    }

    if (textRenderer) delete textRenderer;
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

    float roomWidth = 20.0f;
    float roomHeight = 10.0f;
    float roomDepth = 20.0f;

    float halfWidth = roomWidth / 2.0f;
    float halfHeight = roomHeight / 2.0f;
    float halfDepth = roomDepth / 2.0f;

    float marginX = target.radius * 3.5f;
    float marginY = target.radius * 3.5f;

    glm::vec3 cameraPos = camera->getPosition();
    glm::vec3 cameraFront = camera->getFront();
    glm::vec3 cameraRight = camera->getRight();
    glm::vec3 cameraUp = camera->getUp();

    int maxAttempts = 20;
    int attempt = 0;
    bool foundValidPosition = false;

    glm::vec3 targetPos;

    while (!foundValidPosition && attempt < maxAttempts) {
        attempt++;

        int wallChoice = rand() % 4;

        switch (wallChoice) {
        case 0:
            targetPos.x = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * (halfWidth - marginX);
            targetPos.y = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * (halfHeight - marginY);
            targetPos.z = -halfDepth + 0.5f;
            break;

        case 1:
            targetPos.x = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * (halfWidth - marginX);
            targetPos.y = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * (halfHeight - marginY);
            targetPos.z = halfDepth - 0.5f;
            break;

        case 2:
            targetPos.x = -halfWidth + 0.5f;
            targetPos.y = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * (halfHeight - marginY);
            targetPos.z = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * (halfDepth - marginX);
            break;

        case 3:
            targetPos.x = halfWidth - 0.5f;
            targetPos.y = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * (halfHeight - marginY);
            targetPos.z = (static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f) * (halfDepth - marginX);
            break;
        }

        glm::vec3 toTarget = glm::normalize(targetPos - cameraPos);
        float dotProduct = glm::dot(toTarget, cameraFront);

        if (dotProduct > 0.707f) {
            float rightOffset = glm::dot(toTarget, cameraRight);
            float upOffset = glm::dot(toTarget, cameraUp);

            if (std::abs(rightOffset) < 0.5f && std::abs(upOffset) < 0.4f) {
                foundValidPosition = true;
            }
        }
    }

    if (!foundValidPosition) {
        return;
    }

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
        drawRoom();
        drawLight();
        drawWallWeapons();

        for (const auto& target : targets) {
            if (target.active) {
                drawCylinder3D(target.position, target.radius, 0.15f, target.texture);
            }
        }
    }

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

        drawRect(10, 10, 700, 80, 0.0f, 0.0f, 0.0f, 0.7f);
        drawRect(12, 12, 696, 76, 0.2f, 0.2f, 0.2f, 0.8f);

        for (int i = 0; i < maxLives; i++) {
            if (i < lives) {
                drawTexture(20 + i * 35, 22, 28, 28, heartTexture);
            }
            else {
                drawTexture(20 + i * 35, 22, 28, 28, emptyHeartTexture);
            }
        }

        int minutes = static_cast<int>(elapsed) / 60;
        int seconds = static_cast<int>(elapsed) % 60;
        int centiseconds = static_cast<int>((elapsed - static_cast<int>(elapsed)) * 100) % 100;

        std::stringstream timeStr;
        timeStr << std::setfill('0') << std::setw(2) << minutes << ":"
            << std::setw(2) << seconds << ":"
            << std::setw(2) << centiseconds;

        textRenderer->renderText(timeStr.str(), 130, 35, 0.6f, 0.2f, 1.0f, 1.0f);

        std::stringstream statsStr;
        statsStr << "Hits: " << score << "/" << totalClicks;
        textRenderer->renderText(statsStr.str(), 330, 35, 0.5f, 0.4f, 1.0f, 0.4f);

        double avgSpeed = 0.0;
        if (hitCount > 0) {
            avgSpeed = totalHitTime / hitCount;
        }

        std::stringstream speedStr;
        speedStr << "Speed: " << std::fixed << std::setprecision(2) << avgSpeed << " s";
        textRenderer->renderText(speedStr.str(), 480, 35, 0.45f, 1.0f, 0.8f, 0.3f);

        std::string modeStr = (fireMode == FireMode::USP) ? "USP" : "AK-47";
        float modeR = (fireMode == FireMode::USP) ? 0.7f : 1.0f;
        float modeG = (fireMode == FireMode::USP) ? 0.7f : 0.5f;
        float modeB = (fireMode == FireMode::USP) ? 0.7f : 0.2f;
        textRenderer->renderText(modeStr, 630, 35, 0.4f, modeR, modeG, modeB);

        float infoWidth = 400.0f;
        float infoHeight = 200.0f;
        float infoX = 20;
        float infoY = windowHeight - infoHeight - 20;

        float padding = 10.0f;
        drawRect(infoX - padding, infoY - padding, infoWidth + 2 * padding, infoHeight + 2 * padding, 0.1f, 0.1f, 0.1f, 0.5f);

        drawTexture(infoX, infoY, infoWidth, infoHeight, studentInfoTexture, 1.0f);

        float weaponWidth = 300.0f;
        float weaponHeight = 150.0f;
        float weaponX = windowWidth - weaponWidth - 20;
        float weaponY = windowHeight - weaponHeight - 20;

        unsigned int weaponTexture = (fireMode == FireMode::USP) ? uspTexture : akTexture;
        drawTexture(weaponX, weaponY, weaponWidth, weaponHeight, weaponTexture);
    }
    else {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glClear(GL_DEPTH_BUFFER_BIT);

        glUseProgram(rectShaderProgram);

        float projection[16] = {
            2.0f / windowWidth, 0.0f, 0.0f, 0.0f,
            0.0f, -2.0f / windowHeight, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f, 1.0f
        };

        int projLoc = glGetUniformLocation(rectShaderProgram, "uProjection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);

        drawRect(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight), 0.0f, 0.0f, 0.0f, 0.7f);

        float boxWidth = 450;
        float boxHeight = 400;
        float boxX = (windowWidth - boxWidth) / 2;
        float boxY = (windowHeight - boxHeight) / 2;

        drawRect(boxX, boxY, boxWidth, boxHeight, 0.0f, 0.0f, 0.0f, 1.0f);
        drawRect(boxX + 2, boxY + 2, boxWidth - 4, boxHeight - 4, 0.3f, 0.3f, 0.3f, 1.0f);

        drawRect(restartButton.x, restartButton.y, restartButton.width, restartButton.height, 0.2f, 0.8f, 0.2f, 1.0f);
        drawRect(exitButton.x, exitButton.y, exitButton.width, exitButton.height, 0.8f, 0.2f, 0.2f, 1.0f);

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

        double avgSpeed = avgHitSpeed;

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

    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }

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

        double currentTime = glfwGetTime();
        double timeSinceRecoil = currentTime - lastRecoilTime;
        if (recoilAmount > 0.0f) {
            recoilAmount -= recoilRecoverySpeed * static_cast<float>(timeSinceRecoil);
            if (recoilAmount < 0.0f) recoilAmount = 0.0f;
            lastRecoilTime = currentTime;
        }

        float recoilExpansion = 1.0f + recoilAmount;

        float crosshairSize = 18.0f * recoilExpansion;
        float crosshairThickness = 4.0f;
        float crosshairGap = 6.0f * recoilExpansion;

        float greenIntensity = 0.8f + recoilAmount * 0.2f;
        if (greenIntensity > 1.0f) greenIntensity = 1.0f;

        drawRect(centerX - crosshairSize - crosshairGap, centerY - crosshairThickness / 2,
            crosshairSize, crosshairThickness, 0.0f, greenIntensity, 0.0f, 0.95f);
        drawRect(centerX + crosshairGap, centerY - crosshairThickness / 2,
            crosshairSize, crosshairThickness, 0.0f, greenIntensity, 0.0f, 0.95f);
        drawRect(centerX - crosshairThickness / 2, centerY - crosshairSize - crosshairGap,
            crosshairThickness, crosshairSize, 0.0f, greenIntensity, 0.0f, 0.95f);
        drawRect(centerX - crosshairThickness / 2, centerY + crosshairGap,
            crosshairThickness, crosshairSize, 0.0f, greenIntensity, 0.0f, 0.95f);

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

    lastRecoilTime = glfwGetTime();
    recoilAmount = (fireMode == FireMode::AK47) ? 1.5f : 1.0f;

    double currentTime = glfwGetTime();
    glm::vec3 rayOrigin = camera->getPosition();
    glm::vec3 rayDir = camera->getFront();

    // PRVO: Provjeri da li je pogođeno oružje na zidu (MNOGO MANJI hitbox)
    bool weaponPickedUp = false;
    for (const auto& weapon : wallWeapons) {
        // SMANJENI hitbox na samo 1.5% originalne veličine (bilo je 5%)
        glm::vec3 weaponMin = weapon.position - weapon.scale * 0.004f;
        glm::vec3 weaponMax = weapon.position + weapon.scale * 0.004f;

        if (rayAABBIntersection(rayOrigin, rayDir, weaponMin, weaponMax)) {
            if (weapon.isAK && fireMode != FireMode::AK47) {
                setFireMode(FireMode::AK47);
                std::cout << "✓ Picked up AK-47 from the wall!" << std::endl;
                weaponPickedUp = true;
                break;
            }
            else if (!weapon.isAK && fireMode != FireMode::USP) {
                setFireMode(FireMode::USP);
                std::cout << "✓ Picked up USP-S from the wall!" << std::endl;
                weaponPickedUp = true;
                break;
            }
            weaponPickedUp = true;
            break;
        }
    }

    if (weaponPickedUp) {
        return;
    }

    // DRUGO: Brojanje klika (samo ako nije oružje)
    totalClicks++;

    // TREĆE: Provjeri da li je pogođen target
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
    }
    else {
        std::cout << "\n[WEAPON] AK-47 (Full-Auto)" << std::endl;
    }
}

bool AimTrainer::shouldExit() const {
    return exitRequested;
}

bool AimTrainer::isPointInRect(float px, float py, float rx, float ry, float rw, float rh) {
    return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
}

void AimTrainer::initCylinder() {
    const int segments = 32;
    float radius = 1.0f;
    float halfDepth = 0.05f;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    vertices.insert(vertices.end(), { 0.0f, 0.0f, halfDepth, 0.0f, 0.0f, 1.0f, 0.5f, 0.5f });

    int backCenterIndex = 1;
    vertices.insert(vertices.end(), { 0.0f, 0.0f, -halfDepth, 0.0f, 0.0f, -1.0f, 0.5f, 0.5f });

    int frontStartIndex = 2;

    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
        float x = radius * std::cos(theta);
        float y = radius * std::sin(theta);

        float u = 0.5f + 0.5f * std::cos(theta);
        float v = 0.5f + 0.5f * std::sin(theta);

        vertices.insert(vertices.end(), { x, y, halfDepth, 0.0f, 0.0f, 1.0f, u, v });
    }

    int backStartIndex = frontStartIndex + segments + 1;

    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
        float x = radius * std::cos(theta);
        float y = radius * std::sin(theta);

        float u = 0.5f + 0.5f * std::cos(theta);
        float v = 0.5f + 0.5f * std::sin(theta);

        vertices.insert(vertices.end(), { x, y, -halfDepth, 0.0f, 0.0f, -1.0f, u, v });
    }

    for (int i = 0; i < segments; i++) {
        indices.push_back(0);
        indices.push_back(frontStartIndex + i);
        indices.push_back(frontStartIndex + i + 1);
    }

    for (int i = 0; i < segments; i++) {
        indices.push_back(backCenterIndex);
        indices.push_back(backStartIndex + i + 1);
        indices.push_back(backStartIndex + i);
    }

    int sideStartIndex = backStartIndex + segments + 1;
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * glm::pi<float>() * static_cast<float>(i) / static_cast<float>(segments);
        float x = radius * std::cos(theta);
        float y = radius * std::sin(theta);

        float nx = std::cos(theta);
        float ny = std::sin(theta);

        float u = static_cast<float>(i) / static_cast<float>(segments);

        vertices.insert(vertices.end(), { x, y, halfDepth, nx, ny, 0.0f, u, 0.0f });
        vertices.insert(vertices.end(), { x, y, -halfDepth, nx, ny, 0.0f, u, 1.0f });
    }

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
}

void AimTrainer::drawCylinder3D(const glm::vec3& position, float radius, float depth, unsigned int texture) {
    glUseProgram(cylinderShaderProgram);

    glm::vec3 cameraPos = camera->getPosition();
    glm::vec3 direction = glm::normalize(cameraPos - position);

    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(up, direction));
    glm::vec3 newUp = glm::cross(direction, right);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);

    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0] = glm::vec4(right, 0.0f);
    rotation[1] = glm::vec4(newUp, 0.0f);
    rotation[2] = glm::vec4(direction, 0.0f);

    model = model * rotation;
    model = glm::scale(model, glm::vec3(radius, radius, depth));

    glm::mat4 view = camera->getViewMatrix();
    float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    glm::mat4 projection = camera->getProjectionMatrix(aspect);

    int modelLoc = glGetUniformLocation(cylinderShaderProgram, "uModel");
    int viewLoc = glGetUniformLocation(cylinderShaderProgram, "uView");
    int projLoc = glGetUniformLocation(cylinderShaderProgram, "uProjection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glm::vec3 lightPos(0.0f, 4.0f, 0.0f);
    int lightPosLoc = glGetUniformLocation(cylinderShaderProgram, "uLightPos");
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

    int viewPosLoc = glGetUniformLocation(cylinderShaderProgram, "uViewPos");
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(camera->getPosition()));

    float currentTime = static_cast<float>(glfwGetTime());
    int timeLoc = glGetUniformLocation(cylinderShaderProgram, "uTime");
    glUniform1f(timeLoc, currentTime);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    int texLoc = glGetUniformLocation(cylinderShaderProgram, "uTexture");
    glUniform1i(texLoc, 0);

    glBindVertexArray(cylinderVAO);
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
        -halfWidth, -halfHeight, -halfDepth,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
         halfWidth, -halfHeight, -halfDepth,  0.0f, 0.0f, 1.0f,  texScaleW, 0.0f,
         halfWidth,  halfHeight, -halfDepth,  0.0f, 0.0f, 1.0f,  texScaleW, texScaleH,
        -halfWidth,  halfHeight, -halfDepth,  0.0f, 0.0f, 1.0f,  0.0f, texScaleH,

         halfWidth, -halfHeight, halfDepth,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
        -halfWidth, -halfHeight, halfDepth,  0.0f, 0.0f, -1.0f,  texScaleW, 0.0f,
        -halfWidth,  halfHeight, halfDepth,  0.0f, 0.0f, -1.0f,  texScaleW, texScaleH,
         halfWidth,  halfHeight, halfDepth,  0.0f, 0.0f, -1.0f,  0.0f, texScaleH,

        -halfWidth, -halfHeight, -halfDepth,  1.0f, 0.0f, 0.0f,  texScaleD, 0.0f,
        -halfWidth,  halfHeight, -halfDepth,  1.0f, 0.0f, 0.0f,  texScaleD, texScaleH,
        -halfWidth,  halfHeight,  halfDepth,  1.0f, 0.0f, 0.0f,  0.0f, texScaleH,
        -halfWidth, -halfHeight,  halfDepth,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,

        halfWidth, -halfHeight, -halfDepth,  -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        halfWidth, -halfHeight,  halfDepth,  -1.0f, 0.0f, 0.0f,  texScaleD, 0.0f,
        halfWidth,  halfHeight,  halfDepth,  -1.0f, 0.0f, 0.0f,  texScaleD, texScaleH,
        halfWidth,  halfHeight, -halfDepth,  -1.0f, 0.0f, 0.0f,  0.0f, texScaleH,

        -halfWidth, -halfHeight, -halfDepth,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
         halfWidth, -halfHeight, -halfDepth,  0.0f, 1.0f, 0.0f,  texScaleW, 0.0f,
         halfWidth, -halfHeight,  halfDepth,  0.0f, 1.0f, 0.0f,  texScaleW, texScaleD,
        -halfWidth, -halfHeight,  halfDepth,  0.0f, 1.0f, 0.0f,  0.0f, texScaleD,

        -halfWidth, halfHeight, -halfDepth,  0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         halfWidth, halfHeight, -halfDepth,  0.0f, -1.0f, 0.0f,  texScaleW, 0.0f,
         halfWidth, halfHeight,  halfDepth,  0.0f, -1.0f, 0.0f,  texScaleW, texScaleD,
        -halfWidth, halfHeight,  halfDepth,  0.0f, -1.0f, 0.0f,  0.0f, texScaleD,
    };

    std::vector<unsigned int> indices = {
        0, 1, 2, 0, 2, 3,       // Front wall
        4, 5, 6, 4, 6, 7,       // Back wall
        8, 9, 10, 8, 10, 11,    // Left wall
        12, 13, 14, 12, 14, 15, // Right wall
        16, 18, 17, 16, 19, 18, // Floor (reversed winding)
        20, 21, 22, 20, 22, 23  // Ceiling
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

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glm::vec3 lightPos(0.0f, 4.0f, 0.0f);
    glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(camera->getPosition()));

    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glBindVertexArray(roomVAO);

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

    glBindTexture(GL_TEXTURE_2D, floorTexture);
    glUniform1i(texLoc, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(24 * sizeof(unsigned int)));

    glBindTexture(GL_TEXTURE_2D, ceilingTexture);
    glUniform1i(texLoc, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(30 * sizeof(unsigned int)));

    glBindVertexArray(0);
}

void AimTrainer::initLight() {
    float lightWidth = 2.0f;
    float lightHeight = 0.3f;
    float lightDepth = 2.0f;

    float hw = lightWidth / 2.0f;
    float hh = lightHeight / 2.0f;
    float hd = lightDepth / 2.0f;

    std::vector<float> vertices = {
        -hw, -hh, -hd,  0.0f, -1.0f, 0.0f,
         hw, -hh, -hd,  0.0f, -1.0f, 0.0f,
         hw, -hh,  hd,  0.0f, -1.0f, 0.0f,
        -hw, -hh,  hd,  0.0f, -1.0f, 0.0f,

        -hw, hh, -hd,  0.0f, 1.0f, 0.0f,
         hw, hh, -hd,  0.0f, 1.0f, 0.0f,
         hw, hh,  hd,  0.0f, 1.0f, 0.0f,
        -hw, hh,  hd,  0.0f, 1.0f, 0.0f,

        -hw, -hh, -hd,  0.0f, 0.0f, -1.0f,
         hw, -hh, -hd,  0.0f, 0.0f, -1.0f,
         hw,  hh, -hd,  0.0f, 0.0f, -1.0f,
        -hw,  hh, -hd,  0.0f, 0.0f, -1.0f,

        -hw, -hh, hd,  0.0f, 0.0f, 1.0f,
         hw, -hh, hd,  0.0f, 0.0f, 1.0f,
         hw,  hh, hd,  0.0f, 0.0f, 1.0f,
        -hw,  hh, hd,  0.0f, 0.0f, 1.0f,

        -hw, -hh, -hd,  -1.0f, 0.0f, 0.0f,
        -hw,  hh, -hd,  -1.0f, 0.0f, 0.0f,
        -hw,  hh,  hd,  -1.0f, 0.0f, 0.0f,
        -hw, -hh,  hd,  -1.0f, 0.0f, 0.0f,

        hw, -hh, -hd,  1.0f, 0.0f, 0.0f,
        hw,  hh, -hd,  1.0f, 0.0f, 0.0f,
        hw,  hh,  hd,  1.0f, 0.0f, 0.0f,
        hw, -hh,  hd,  1.0f, 0.0f, 0.0f,
    };

    std::vector<unsigned int> indices = {
        0, 1, 2, 0, 2, 3,       // Front wall
        4, 5, 6, 4, 6, 7,       // Back wall
        8, 9, 10, 8, 10, 11,    // Left wall
        12, 13, 14, 12, 14, 15, // Right wall
        16, 18, 17, 16, 19, 18, // Floor (reversed winding)
        20, 21, 22, 20, 22, 23  // Ceiling
    };

    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);
    glGenBuffers(1, &lightEBO);

    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void AimTrainer::initWallWeapons() {
    std::cout << "=== INITIALIZING WALL WEAPONS ===" << std::endl;

    WallWeapon testAK;
    if (OBJLoader::loadOBJ("obj/ak47.obj", testAK.mesh)) {
        OBJLoader::setupMesh(testAK.mesh);
        testAK.mesh.texture = loadImageToTexture("obj/weapon_rif_ak47.png");

        testAK.position = glm::vec3(7.0f, -3.5f, -9.5f);
        testAK.rotation = glm::vec3(0.0f, glm::radians(180.0f), glm::radians(90.0f));
        testAK.scale = glm::vec3(150.0f, 150.0f, 150.0f);
        testAK.isAK = true;
        wallWeapons.push_back(testAK);

        std::cout << "✓ AK-47 mounted on BOTTOM RIGHT corner!" << std::endl;
        std::cout << "  Position: (" << testAK.position.x << ", " << testAK.position.y << ", " << testAK.position.z << ")" << std::endl;
    }
    else {
        std::cout << "✗ FAILED to load AK-47 model!" << std::endl;
    }

    WallWeapon testUSP;
    if (OBJLoader::loadOBJ("obj2/usp.obj", testUSP.mesh)) {
        OBJLoader::setupMesh(testUSP.mesh);
        testUSP.mesh.texture = loadImageToTexture("obj2/weapon_pist_usp_silencer.png");

        testUSP.position = glm::vec3(-8.5f, -3.5f, -9.5f);
        testUSP.rotation = glm::vec3(0.0f, glm::radians(180.0f), glm::radians(90.0f));
        testUSP.scale = glm::vec3(150.0f, 150.0f, 150.0f);
        testUSP.isAK = false;
        wallWeapons.push_back(testUSP);

        std::cout << "✓ USP-S mounted on BOTTOM LEFT corner!" << std::endl;
        std::cout << "  Position: (" << testUSP.position.x << ", " << testUSP.position.y << ", " << testUSP.position.z << ")" << std::endl;
    }
    else {
        std::cout << "✗ FAILED to load USP model!" << std::endl;
    }

    std::cout << "Total wall weapons: " << wallWeapons.size() << std::endl;
    std::cout << "==================================" << std::endl;
}

void AimTrainer::drawWallWeapons() {
    if (wallWeapons.empty()) {
        return;
    }

    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(weaponShaderProgram);

    glm::mat4 view = camera->getViewMatrix();
    float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

    int viewLoc = glGetUniformLocation(weaponShaderProgram, "uView");
    int projLoc = glGetUniformLocation(weaponShaderProgram, "uProjection");

    if (viewLoc == -1 || projLoc == -1) {
        std::cout << "ERROR: Shader uniforms not found!" << std::endl;
        return;
    }

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glm::vec3 lightPos(0.0f, 4.0f, 0.0f);
    int lightPosLoc = glGetUniformLocation(weaponShaderProgram, "uLightPos");
    if (lightPosLoc != -1) {
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
    }

    int viewPosLoc = glGetUniformLocation(weaponShaderProgram, "uViewPos");
    if (viewPosLoc != -1) {
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(camera->getPosition()));
    }

    float currentTime = static_cast<float>(glfwGetTime());
    int timeLoc = glGetUniformLocation(weaponShaderProgram, "uTime");
    if (timeLoc != -1) {
        glUniform1f(timeLoc, currentTime);
    }

    for (const auto& weapon : wallWeapons) {
        drawWeaponMesh(weapon);
    }

    if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
    if (faceCullingEnabled) glEnable(GL_CULL_FACE);
}

void AimTrainer::drawWeaponMesh(const WallWeapon& weapon) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, weapon.position);
    model = glm::rotate(model, weapon.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, weapon.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, weapon.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, weapon.scale);

    int modelLoc = glGetUniformLocation(weaponShaderProgram, "uModel");
    if (modelLoc == -1) {
        return;
    }

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    if (weapon.mesh.texture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, weapon.mesh.texture);
        int texLoc = glGetUniformLocation(weaponShaderProgram, "uTexture");
        if (texLoc != -1) {
            glUniform1i(texLoc, 0);
        }
    }

    glBindVertexArray(weapon.mesh.VAO);
    glDrawElements(GL_TRIANGLES, weapon.mesh.indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void AimTrainer::drawLight() {
    glUseProgram(lightShaderProgram);

    glm::vec3 lightPosition(0.0f, 4.5f, 0.0f);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, lightPosition);

    glm::mat4 view = camera->getViewMatrix();
    float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    glm::mat4 projection = camera->getProjectionMatrix(aspect);

    int modelLoc = glGetUniformLocation(lightShaderProgram, "uModel");
    int viewLoc = glGetUniformLocation(lightShaderProgram, "uView");
    int projLoc = glGetUniformLocation(lightShaderProgram, "uProjection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glm::vec3 lightColor(1.0f, 0.95f, 0.8f);
    int colorLoc = glGetUniformLocation(lightShaderProgram, "uLightColor");
    glUniform3fv(colorLoc, 1, glm::value_ptr(lightColor));

    float intensity = 2.0f;
    int intensityLoc = glGetUniformLocation(lightShaderProgram, "uIntensity");
    glUniform1f(intensityLoc, intensity);

    glBindVertexArray(lightVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void AimTrainer::toggleDepthTest() {
    depthTestEnabled = !depthTestEnabled;
    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
    else {
        glDisable(GL_DEPTH_TEST);
    }
}

void AimTrainer::toggleFaceCulling() {
    faceCullingEnabled = !faceCullingEnabled;
    if (faceCullingEnabled) {
        glEnable(GL_CULL_FACE);
    }
    else {
        glDisable(GL_CULL_FACE);
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

bool AimTrainer::rayAABBIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
    const glm::vec3& boxMin, const glm::vec3& boxMax) {
    glm::vec3 invDir = glm::vec3(1.0f) / rayDir;

    glm::vec3 t0 = (boxMin - rayOrigin) * invDir;
    glm::vec3 t1 = (boxMax - rayOrigin) * invDir;

    glm::vec3 tmin = glm::min(t0, t1);
    glm::vec3 tmax = glm::max(t0, t1);

    float tNear = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
    float tFar = glm::min(glm::min(tmax.x, tmax.y), tmax.z);

    return tNear <= tFar && tFar >= 0.0f;
}