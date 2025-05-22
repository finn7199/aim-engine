#include "renderer.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include <iostream>

// [Vertex shader]
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 uMVP;

void main()
{
    gl_Position = uMVP * vec4(aPos, 1.0);
}
)";

// [Fragment shader]
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 uColor;

void main()
{
    FragColor = vec4(uColor, 1.0);
}
)";

//function to generate sphere vertices
void GenerateSphereVertices(std::vector<float>& vertices, float radius, int sectors, int stacks) {
    const float PI = 3.1415926f;
    std::vector<float> tmpVertices;

    float sectorStep = 2 * PI / sectors;
    float stackStep = PI / stacks;

    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = PI / 2 - i * stackStep;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep;
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            tmpVertices.push_back(x);
            tmpVertices.push_back(y);
            tmpVertices.push_back(z);
        }
    }

    // Generate sphere triangle vertices
    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;

        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            // Two triangles per sector
            if (i != 0) {
                // Triangle 1
                vertices.push_back(tmpVertices[k1 * 3]);
                vertices.push_back(tmpVertices[k1 * 3 + 1]);
                vertices.push_back(tmpVertices[k1 * 3 + 2]);

                vertices.push_back(tmpVertices[k2 * 3]);
                vertices.push_back(tmpVertices[k2 * 3 + 1]);
                vertices.push_back(tmpVertices[k2 * 3 + 2]);

                vertices.push_back(tmpVertices[(k1 + 1) * 3]);
                vertices.push_back(tmpVertices[(k1 + 1) * 3 + 1]);
                vertices.push_back(tmpVertices[(k1 + 1) * 3 + 2]);
            }

            if (i != (stacks - 1)) {
                // Triangle 2
                vertices.push_back(tmpVertices[(k1 + 1) * 3]);
                vertices.push_back(tmpVertices[(k1 + 1) * 3 + 1]);
                vertices.push_back(tmpVertices[(k1 + 1) * 3 + 2]);

                vertices.push_back(tmpVertices[k2 * 3]);
                vertices.push_back(tmpVertices[k2 * 3 + 1]);
                vertices.push_back(tmpVertices[k2 * 3 + 2]);

                vertices.push_back(tmpVertices[(k2 + 1) * 3]);
                vertices.push_back(tmpVertices[(k2 + 1) * 3 + 1]);
                vertices.push_back(tmpVertices[(k2 + 1) * 3 + 2]);
            }
        }
    }
}

void Renderer::Init() {
    // Compile shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Check for vertex shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL); // Fixed: was using vertexShaderSource here
    glCompileShader(fragmentShader);

    // Check for fragment shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Setup sphere VAO
    std::vector<float> sphereVertices;
    GenerateSphereVertices(sphereVertices, 1.0f, 36, 18);
    sphereVerticesCount = static_cast<int>(sphereVertices.size()) / 3;

    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);

    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    // Setup cube VAO
    float cubeVertices[] = {
        // Positions
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f
    };

    unsigned int cubeIndices[] = {
        0, 1, 3, 1, 2, 3,  // Front
        1, 5, 2, 5, 6, 2,  // Right
        5, 4, 6, 4, 7, 6,  // Back
        4, 0, 7, 0, 3, 7,  // Left
        4, 5, 0, 5, 1, 0,  // Bottom
        3, 2, 7, 2, 6, 7   // Top
    };

    cubeVerticesCount = 36; // 12 triangles * 3 vertices

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void Renderer::BeginFrame() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::DrawCube(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(shaderProgram);
    glm::mat4 mvp = projection * view * model;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMVP"), 1, GL_FALSE, glm::value_ptr(mvp));
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), 0.3f, 0.3f, 1.0f); // Example color for the cube
    glBindVertexArray(cubeVAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Renderer::DrawSphere(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(shaderProgram);
    glm::mat4 mvp = projection * view * model;
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "uMVP"), 1, GL_FALSE, glm::value_ptr(mvp));
    glUniform3f(glGetUniformLocation(shaderProgram, "uColor"), 1.0f, 0.3f, 0.3f);
    glBindVertexArray(sphereVAO);
    glDrawArrays(GL_TRIANGLES, 0, sphereVerticesCount);
    glBindVertexArray(0);
}

void Renderer::EndFrame() {
    // Handled in main: glfwSwapBuffers
}