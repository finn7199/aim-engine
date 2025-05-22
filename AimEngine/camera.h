#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    glm::vec3 Position;
    float Yaw, Pitch;

    float MovementSpeed = 5.0f;
    float MouseSensitivity = 0.1f;

    Camera(glm::vec3 startPos);

    glm::mat4 GetViewMatrix();
    void ProcessKeyboard(bool* keys, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset);
};
