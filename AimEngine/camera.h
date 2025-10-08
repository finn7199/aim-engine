#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    float Yaw, Pitch;
    float MovementSpeed = 5.0f;
    float MouseSensitivity = 0.1f;

    Camera(glm::vec3 startPos);

    glm::mat4 GetViewMatrix();
    void ProcessKeyboard(bool* keys, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset);

private:
    void updateCameraVectors();
};
