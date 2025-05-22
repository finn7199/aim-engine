#include "camera.h"

Camera::Camera(glm::vec3 startPos) {
    Position = startPos;
    Yaw = -90.0f;
    Pitch = 0.0f;
}

glm::mat4 Camera::GetViewMatrix() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front = glm::normalize(front);
    return glm::lookAt(Position, Position + front, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::ProcessKeyboard(bool* keys, float deltaTime) {
    glm::vec3 front = glm::normalize(glm::vec3(
        cos(glm::radians(Yaw)),
        0.0f,
        sin(glm::radians(Yaw))
    ));
    glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));

    float velocity = MovementSpeed * deltaTime;
    if (keys['W']) Position += front * velocity;
    if (keys['S']) Position -= front * velocity;
    if (keys['A']) Position -= right * velocity;
    if (keys['D']) Position += right * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch -= yoffset;

    if (Pitch > 89.0f) Pitch = 89.0f;
    if (Pitch < -89.0f) Pitch = -89.0f;
}
