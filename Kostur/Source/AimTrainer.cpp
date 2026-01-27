#include "../Header/AimTrainer.h"
#include "../Header/Util.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>

AimTrainer::AimTrainer(int width, int height) 
    : score(0), lives(3), maxLives(3), gameOver(false), spawnTimer(0.0f), 
      spawnInterval(1.5f), initialSpawnInterval(1.5f), minSpawnInterval(0.3f),
      targetLifeTimeMultiplier(1.0f), minTargetLifeTime(0.4f),
      windowWidth(width), windowHeight(height), hitCount(0), totalHitTime(0.0),
      lastHitTime(0.0), gameOverTime(0.0), survivalTime(0.0), avgHitSpeed(0.0),
      textRenderer(nullptr), exitRequested(false), totalClicks(0),
      fireMode(FireMode::USP), isMousePressed(false), lastShotTime(0.0), fireRate(0.1)
{
    srand(static_cast<unsigned int>(time(nullptr)));
    
    rectShaderProgram = createShader("Shaders/rect.vert", "Shaders/rect.frag");
    textureShaderProgram = createShader("Shaders/texture.vert", "Shaders/texture.frag");
    freetypeShaderProgram = createShader("Shaders/freetype.vert", "Shaders/freetype.frag");
    texturedCircleShaderProgram = createShader("Shaders/textured_circle.vert", "Shaders/textured_circle.frag");
    
    textRenderer = new TextRenderer(freetypeShaderProgram, windowWidth, windowHeight);
    if (!textRenderer->loadFont("C:/Windows/Fonts/arial.ttf", 48)) {
        std::cout << "Warning: Failed to load Arial font" << std::endl;
    }
    
    studentInfoTexture = loadImageToTexture("Resources/indeks.png");
    backgroundTexture = loadImageToTexture("Resources/mirage.png");
    terroristTexture = loadImageToTexture("Resources/terrorist.png");
    counterTexture = loadImageToTexture("Resources/counter.png");
    heartTexture = loadImageToTexture("Resources/heart.png");
    emptyHeartTexture = loadImageToTexture("Resources/empty-heart.png");
    akTexture = loadImageToTexture("Resources/ak.png");
    uspTexture = loadImageToTexture("Resources/usp.png");
    
    initBuffers();
    
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
}

AimTrainer::~AimTrainer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);
    glDeleteVertexArrays(1, &textureVAO);
    glDeleteBuffers(1, &textureVBO);
    glDeleteProgram(rectShaderProgram);
    glDeleteProgram(textureShaderProgram);
    glDeleteProgram(freetypeShaderProgram);
    glDeleteProgram(texturedCircleShaderProgram);
    glDeleteTextures(1, &studentInfoTexture);
    glDeleteTextures(1, &backgroundTexture);
    glDeleteTextures(1, &terroristTexture);
    glDeleteTextures(1, &counterTexture);
    glDeleteTextures(1, &heartTexture);
    glDeleteTextures(1, &emptyHeartTexture);
    glDeleteTextures(1, &akTexture);
    glDeleteTextures(1, &uspTexture);
    if (textRenderer) delete textRenderer;
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
    
    // VAO za circle (textured circle shader)
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // VAO za rect (samo pozicija, 2D)
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, NULL, GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // VAO za texture (pozicija + UV koordinate)
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
    target.radius = 50.0f;
    
    target.x = target.radius + static_cast<float>(rand()) / RAND_MAX * (windowWidth - 2 * target.radius);
    target.y = target.radius + static_cast<float>(rand()) / RAND_MAX * (windowHeight - 2 * target.radius - 100);
    target.y += 50;
    
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
    
    targets.clear();
    
    startTime = glfwGetTime();
    lastHitTime = startTime;
    
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
    drawTexture(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight), backgroundTexture);
    
    for (const auto& target : targets) {
        if (target.active) {
            drawCircle(target.x, target.y, target.radius, target.texture);
        }
    }
    
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
        
        drawRect(10, 10, 700, 80, 0.0f, 0.0f, 0.0f);
        drawRect(12, 12, 696, 76, 0.2f, 0.2f, 0.2f);
        
        for (int i = 0; i < maxLives; i++) {
            if (i < lives) {
                drawTexture(20 + i * 35, 22, 28, 28, heartTexture);
            } else {
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
        
        textRenderer->renderText(timeStr.str(), 130, 35, 0.6f, 1.0f, 1.0f, 1.0f);
        
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
        
        float accuracy = 0.0f;
        if (totalClicks > 0) {
            accuracy = (static_cast<float>(score) / static_cast<float>(totalClicks)) * 100.0f;
        }
        
        std::cout << "Time: " << minutes << ":" << (seconds < 10 ? "0" : "") << seconds << ":" << (centiseconds < 10 ? "0" : "") << centiseconds
                  << " | Zivoti: " << lives << "/" << maxLives 
                  << " | Pogodaka: " << score << "/" << totalClicks << " (" << std::fixed << std::setprecision(1) << accuracy << "%)"
                  << " | Avg Speed: " << avgSpeed << "s         \r" << std::flush;
        
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
    } else {
        float boxWidth = 450;
        float boxHeight = 400;
        float boxX = (windowWidth - boxWidth) / 2;
        float boxY = (windowHeight - boxHeight) / 2;
        
        drawRect(boxX, boxY, boxWidth, boxHeight, 0.0f, 0.0f, 0.0f);
        drawRect(boxX + 2, boxY + 2, boxWidth - 4, boxHeight - 4, 0.3f, 0.3f, 0.3f);
        
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
        
        glUseProgram(rectShaderProgram);
        
        float projection[16] = {
            2.0f / windowWidth, 0.0f, 0.0f, 0.0f,
            0.0f, -2.0f / windowHeight, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f, 1.0f
        };
        
        int projLoc = glGetUniformLocation(rectShaderProgram, "uProjection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
        
        drawRect(restartButton.x, restartButton.y, restartButton.width, restartButton.height, 0.2f, 0.8f, 0.2f);
        
        float restartTextWidth = textRenderer->getTextWidth("RESTART", 0.5f);
        float restartTextX = restartButton.x + (restartButton.width - restartTextWidth) / 2;
        textRenderer->renderText("RESTART", restartTextX, restartButton.y + 30, 0.5f, 1.0f, 1.0f, 1.0f);
        
        drawRect(exitButton.x, exitButton.y, exitButton.width, exitButton.height, 0.8f, 0.2f, 0.2f);
        
        float exitTextWidth = textRenderer->getTextWidth("EXIT", 0.5f);
        float exitTextX = exitButton.x + (exitButton.width - exitTextWidth) / 2;
        textRenderer->renderText("EXIT", exitTextX, exitButton.y + 30, 0.5f, 1.0f, 1.0f, 1.0f);
        
        static bool printedOnce = false;
        if (!printedOnce) {
            std::cout << "\n\n=== GAME OVER ===" << std::endl;
            std::cout << "Vreme prezivljanja: " << (int)survivalTime << "s" << std::endl;
            std::cout << "Ukupno pogodaka: " << score << std::endl;
            std::cout << "Prosecna brzina pogadjanja: " << avgHitSpeed << "s" << std::endl;
            std::cout << "\nPritisni 'R' za restart ili klikni na zeleno dugme" << std::endl;
            std::cout << "Pritisni 'ESC' za izlaz ili klikni na crveno dugme" << std::endl;
            std::cout << "================\n" << std::endl;
            printedOnce = true;
        }
    }
}

void AimTrainer::drawCircle(float x, float y, float radius, unsigned int texture) {
    glUseProgram(texturedCircleShaderProgram);
    
    float projection[16] = {
        2.0f / windowWidth, 0.0f, 0.0f, 0.0f,
        0.0f, -2.0f / windowHeight, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f
    };
    
    int projLoc = glGetUniformLocation(texturedCircleShaderProgram, "uProjection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection);
    
    glBindVertexArray(VAO);
    
    float model[16] = {
        radius, 0.0f, 0.0f, 0.0f,
        0.0f, radius, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        x, y, 0.0f, 1.0f
    };
    
    int modelLoc = glGetUniformLocation(texturedCircleShaderProgram, "uModel");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    int texLoc = glGetUniformLocation(texturedCircleShaderProgram, "uTexture");
    glUniform1i(texLoc, 0);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void AimTrainer::drawRect(float x, float y, float width, float height, float r, float g, float b, float alpha) {
    glUseProgram(rectShaderProgram);
    
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
    
    bool hit = false;
    for (auto& target : targets) {
        if (target.active) {
            float dx = static_cast<float>(mouseX) - target.x;
            float dy = static_cast<float>(mouseY) - target.y;
            float distance = std::sqrt(dx * dx + dy * dy);
            
            if (distance <= target.radius) {
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
