#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../Header/Util.h"
#include "../Header/AimTrainer.h"
#include "../Header/Crosshair.h"

AimTrainer* game = nullptr;
bool firstMouse = true;
double lastMouseX = 0.0;
double lastMouseY = 0.0;

void mouseMovementCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }
    
    double xoffset = xpos - lastMouseX;
    double yoffset = lastMouseY - ypos;
    
    lastMouseX = xpos;
    lastMouseY = ypos;
    
    if (game) {
        game->processMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
    }
}

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
    
    if (key == GLFW_KEY_D && action == GLFW_PRESS && game) {
        game->toggleDepthTest();
    }
    
    if (key == GLFW_KEY_F && action == GLFW_PRESS && game) {
        game->toggleFaceCulling();
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
    
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "3D Aim Trainer", monitor, NULL);
    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    glfwSetCursorPos(window, WINDOW_WIDTH / 2.0, WINDOW_HEIGHT / 2.0);
    lastMouseX = WINDOW_WIDTH / 2.0;
    lastMouseY = WINDOW_HEIGHT / 2.0;

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, mouseMovementCallback);
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
            
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}