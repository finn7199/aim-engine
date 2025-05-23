#pragma once
#include <glm/glm.hpp>

class Renderer {
public:
    void Init();
    void BeginFrame();
    void DrawCube(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& color = glm::vec3(0.3f, 0.3f, 1.0f));
    void DrawSphere(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
    void EndFrame();

private:
    unsigned int shaderProgram;
    unsigned int sphereVAO, sphereVBO;
    int sphereVerticesCount;
    unsigned int cubeVAO, cubeVBO, cubeEBO; // Buffers for the cube
    int cubeVerticesCount; // Number of vertices for the cube (using indices)
};