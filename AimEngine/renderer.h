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
    void DrawSkybox(const glm::mat4& view, const glm::mat4& projection);
    void RenderQuad();
    void EndFrame();

    void SetViewPosition(const glm::vec3& position);
    void SetMaterial(const glm::vec3& albedo, float metallic, float roughness);
    void SetDirectionalLight(const glm::vec3& direction, const glm::vec3& color);
    void SetSpotLight(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color, float cutOff, float outerCutOff, float constant, float linear, float quadratic);
    void ToggleSpotLight(bool enabled);

private:
    Shader* pbrShader;
    unsigned int sphereVAO, sphereVBO;
    int sphereVerticesCount;
    unsigned int cubeVAO, cubeVBO, cubeEBO; // Buffers for the cube
    int cubeVerticesCount; // Number of vertices for the cube (using indices)

    Shader* crosshairShader;
    unsigned int crosshairVAO, crosshairVBO;

    Shader* skyboxShader;
    unsigned int cubemapTexture;
    unsigned int skyboxVAO, skyboxVBO;

    // IBL textures
    unsigned int irradianceMap;
    unsigned int prefilterMap;
    unsigned int brdfLUTTexture;

    unsigned int quadVAO, quadVBO;
};