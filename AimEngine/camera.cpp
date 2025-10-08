#include "camera.h"

Camera::Camera(glm::vec3 startPos) : Position(startPos), Yaw(-90.0f), Pitch(0.0f) {
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
}

// Update this function to use the member vectors
void Camera::ProcessKeyboard(bool* keys, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (keys['W']) Position += Front * velocity;
    if (keys['S']) Position -= Front * velocity;
    if (keys['A']) Position -= Right * velocity;
    if (keys['D']) Position += Right * velocity;
}


void Camera::ProcessMouseMovement(float xoffset, float yoffset) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch -= yoffset;

    if (Pitch > 89.0f) Pitch = 89.0f;
    if (Pitch < -89.0f) Pitch = -89.0f;

    updateCameraVectors(); // Update vectors whenever the mouse moves
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}