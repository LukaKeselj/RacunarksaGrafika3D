#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
private:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;
    
    float yaw;
    float pitch;
    float movementSpeed;
    float mouseSensitivity;
    float zoom;
    
    void updateCameraVectors();
    
public:
    Camera(glm::vec3 startPosition = glm::vec3(0.0f, 0.0f, 3.0f),
           glm::vec3 startUp = glm::vec3(0.0f, 1.0f, 0.0f),
           float startYaw = -90.0f,
           float startPitch = 0.0f);
    
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspect, float nearPlane = 0.1f, float farPlane = 100.0f) const;
    
    void processKeyboard(CameraMovement direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void processMouseScroll(float yoffset);
    
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getFront() const { return front; }
    glm::vec3 getRight() const { return right; }
    glm::vec3 getUp() const { return up; }
    float getZoom() const { return zoom; }
    
    // Za postavljanje kamere da gleda prema odre?enoj ta?ki
    void lookAt(const glm::vec3& target);
    
    // Za raycasting (3D picking)
    glm::vec3 getRayDirection(float mouseX, float mouseY, int screenWidth, int screenHeight) const;
};
