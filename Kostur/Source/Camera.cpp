#include "../Header/Camera.h"
#include <algorithm>

Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch)
    : position(startPosition), worldUp(startUp), yaw(startYaw), pitch(startPitch),
    front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(5.0f), mouseSensitivity(0.1f), zoom(45.0f)
{
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

// NOVI OVERLOAD - 1 parametar (samo aspect)
glm::mat4 Camera::getProjectionMatrix(float aspect) const {
    return glm::perspective(glm::radians(zoom), aspect, 0.1f, 100.0f);
}

// Postojeći 3-parametarski
glm::mat4 Camera::getProjectionMatrix(float aspect, float nearPlane, float farPlane) const {
    return glm::perspective(glm::radians(zoom), aspect, nearPlane, farPlane);
}

void Camera::processKeyboard(CameraMovement direction, float deltaTime) {
    float velocity = movementSpeed * deltaTime;

    if (direction == CameraMovement::FORWARD)
        position += front * velocity;
    if (direction == CameraMovement::BACKWARD)
        position -= front * velocity;
    if (direction == CameraMovement::LEFT)
        position -= right * velocity;
    if (direction == CameraMovement::RIGHT)
        position += right * velocity;
    if (direction == CameraMovement::UP)
        position += up * velocity;
    if (direction == CameraMovement::DOWN)
        position -= up * velocity;
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (constrainPitch) {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::processMouseScroll(float yoffset) {
    zoom -= yoffset;
    if (zoom < 1.0f)
        zoom = 1.0f;
    if (zoom > 45.0f)
        zoom = 45.0f;
}

void Camera::updateCameraVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);

    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

glm::vec3 Camera::getRayDirection(float mouseX, float mouseY, int screenWidth, int screenHeight) const {
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight;

    glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);

    glm::mat4 projection = getProjectionMatrix((float)screenWidth / (float)screenHeight);
    glm::vec4 rayEye = glm::inverse(projection) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    glm::mat4 view = getViewMatrix();
    glm::vec3 rayWorld = glm::vec3(glm::inverse(view) * rayEye);
    rayWorld = glm::normalize(rayWorld);

    return rayWorld;
}

void Camera::lookAt(const glm::vec3& target) {
    glm::vec3 direction = glm::normalize(target - position);

    yaw = glm::degrees(atan2(direction.z, direction.x));
    pitch = glm::degrees(asin(direction.y));

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    updateCameraVectors();
}