#pragma once
#include <glm/glm.hpp>
#include "shader.h" 

class Renderer {
public:
    ~Renderer();

    void Init();
    void BeginFrame();
    void DrawCube(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
    void DrawSphere(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection);
    void DrawCrosshair();
    void EndFrame();

    void SetViewPosition(const glm::vec3& position);
    void SetMaterial(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess);
    void SetDirectionalLight(const glm::vec3& direction, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular);
    void SetSpotLight(const glm::vec3& position, const glm::vec3& direction,float cutOff, float outerCutOff, const glm::vec3& ambient, const glm::vec3& diffuse,
                      const glm::vec3& specular, float constant, float linear, float quadratic);
    void ToggleSpotLight(bool enabled);

private:
    Shader* lightingShader;
    Shader* crosshairShader;

    unsigned int sphereVAO, sphereVBO;
    int sphereVerticesCount;
    unsigned int cubeVAO, cubeVBO, cubeEBO; // Buffers for the cube
    int cubeVerticesCount; // Number of vertices for the cube (using indices)
    unsigned int crosshairVAO, crosshairVBO;
};