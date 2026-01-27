#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../Header/Util.h"
#include "../Header/AimTrainer.h"
#include "../Header/Crosshair.h"

AimTrainer* game = nullptr;

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && game) {
        if (action == GLFW_PRESS) {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            game->handleMousePress(mouseX, mouseY);
        } else if (action == GLFW_RELEASE) {
            game->handleMouseRelease();
        }
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    
    if (key == GLFW_KEY_R && action == GLFW_PRESS && game) {
        if (game->isGameOver()) {
            game->restart();
        }
    }
    
    if (key == GLFW_KEY_1 && action == GLFW_PRESS && game) {
        game->setFireMode(FireMode::AK47);
    }
    
    if (key == GLFW_KEY_2 && action == GLFW_PRESS && game) {
        game->setFireMode(FireMode::USP);
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    
    const int WINDOW_WIDTH = mode->width;
    const int WINDOW_HEIGHT = mode->height;
    
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Aim Trainer", monitor, NULL);
    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLFWcursor* crosshairCursor = createCrosshairCursor();
    if (crosshairCursor) {
        glfwSetCursor(window, crosshairCursor);
    }

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyCallback);

    glClearColor(0.15f, 0.15f, 0.2f, 1.0f);

    game = new AimTrainer(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    double lastTime = glfwGetTime();
    const double targetFPS = 75.0;
    const double targetFrameTime = 1.0 / targetFPS;

    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        
        if (deltaTime >= targetFrameTime) {
            lastTime = currentTime;
            
            glClear(GL_COLOR_BUFFER_BIT);

            game->update(static_cast<float>(deltaTime));
            game->render();

            if (game->shouldExit()) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }

            glfwSwapBuffers(window);
        }
        
        glfwPollEvents();
    }

    delete game;
    
    if (crosshairCursor) {
        glfwDestroyCursor(crosshairCursor);
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}