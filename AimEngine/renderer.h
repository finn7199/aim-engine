#pragma once
#include <glm/glm.hpp>

class Renderer {
public:
    void Init();
    void BeginFrame();
    void DrawCube(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
    void DrawSphere(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
    void EndFrame();

private:
    unsigned int shaderProgram;
    unsigned int sphereVAO, sphereVBO;
    int sphereVerticesCount;
    unsigned int cubeVAO, cubeVBO, cubeEBO; // Buffers for the cube
    int cubeVerticesCount; // Number of vertices for the cube (using indices)
};