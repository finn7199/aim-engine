#include "renderer.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <cmath>
#include <iostream>
#include "stb_image.h"

Renderer::~Renderer() {
	delete lightingShader;
	delete crosshairShader;
	delete skyboxShader;
}

//function to generate sphere vertices
void GenerateSphereVertices(std::vector<float>& vertices, float radius, int sectors, int stacks) {
	const float PI = 3.1415926f;
	std::vector<float> tmpVertices;
	std::vector<float> tmpNormals;
	std::vector<unsigned int> indices;

	float sectorStep = 2 * PI / sectors;
	float stackStep = PI / stacks;

	// Generate vertices and normals
	for (int i = 0; i <= stacks; ++i) {
		float stackAngle = PI / 2 - i * stackStep;
		float xy = radius * cosf(stackAngle);
		float z = radius * sinf(stackAngle);

		for (int j = 0; j <= sectors; ++j) {
			float sectorAngle = j * sectorStep;
			float x = xy * cosf(sectorAngle);
			float y = xy * sinf(sectorAngle);

			// Vertex position
			tmpVertices.push_back(x);
			tmpVertices.push_back(y);
			tmpVertices.push_back(z);

			// Normal (normalized vertex position)
			float length = sqrt(x * x + y * y + z * z);
			tmpNormals.push_back(x / length);
			tmpNormals.push_back(y / length);
			tmpNormals.push_back(z / length);
		}
	}

	// Generate indices
	for (int i = 0; i < stacks; ++i) {
		int k1 = i * (sectors + 1);
		int k2 = k1 + sectors + 1;

		for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
			if (i != 0) {
				indices.push_back(k1);
				indices.push_back(k2);
				indices.push_back(k1 + 1);
			}

			if (i != (stacks - 1)) {
				indices.push_back(k1 + 1);
				indices.push_back(k2);
				indices.push_back(k2 + 1);
			}
		}
	}

	// Create interleaved vertex data
	for (unsigned int i = 0; i < indices.size(); ++i) {
		unsigned int index = indices[i];

		// Position
		vertices.push_back(tmpVertices[index * 3]);
		vertices.push_back(tmpVertices[index * 3 + 1]);
		vertices.push_back(tmpVertices[index * 3 + 2]);

		// Normal
		vertices.push_back(tmpNormals[index * 3]);
		vertices.push_back(tmpNormals[index * 3 + 1]);
		vertices.push_back(tmpNormals[index * 3 + 2]);
	}
}

float skyboxVertices[] = {
	// positions for skybox
	-1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
};

void Renderer::Init() {
	// Compile shaders
	lightingShader = new Shader("Shaders/lighting.vert", "Shaders/lighting.frag");
	crosshairShader = new Shader("Shaders/crosshair.vert", "Shaders/crosshair.frag");
	skyboxShader = new Shader("Shaders/skybox.vert", "Shaders/skybox.frag");

	// Setup sphere VAO
	std::vector<float> sphereVertices;
	GenerateSphereVertices(sphereVertices, 1.0f, 36, 18);
	sphereVerticesCount = static_cast<int>(sphereVertices.size()) / 6; // position + normal

	glGenVertexArrays(1, &sphereVAO);
	glGenBuffers(1, &sphereVBO);

	glBindVertexArray(sphereVAO);
	glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
	glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Setup cube VAO
	float cubeVertices[] = {
		// Positions          // Normals
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f
	};

	unsigned int cubeIndices[] = {
		0, 1, 3, 1, 2, 3,  // Front
		4, 5, 7, 5, 6, 7,  // Back
		8, 9, 11, 9, 10, 11, // Left
		12, 13, 15, 13, 14, 15, // Right
		16, 17, 19, 17, 18, 19, // Bottom
		20, 21, 23, 21, 22, 23  // Top
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

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// Setup Crosshair Geometry
	/*
	const unsigned int SCR_WIDTH = 1920;
	const unsigned int SCR_HEIGHT = 1080;
	float aspectRatio = (float)SCR_WIDTH / (float)SCR_HEIGHT;
	*/
	float aspectRatio = 1.777; // 1920x1080 aspect ratio
	float crosshairLength = 0.025f; // visual size for the crosshair.

	float crosshairVertices[] = {
		// Horizontal line (squished by the aspect ratio)
		-crosshairLength / aspectRatio,  0.0f,
		 crosshairLength / aspectRatio,  0.0f,
		 // Vertical line
		  0.0f, -crosshairLength,
		  0.0f,  crosshairLength
	};
	glGenVertexArrays(1, &crosshairVAO);
	glGenBuffers(1, &crosshairVBO);
	glBindVertexArray(crosshairVAO);
	glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), &crosshairVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Setup Skybox VAO
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// HDR EQUIRECTANGULAR TO CUBEMAP CONVERSION
	Shader equirectangularShader("Shaders/equirectangular.vert", "Shaders/equirectangular.frag");
	// 1. Create a framebuffer to render to the cubemap faces
	unsigned int captureFBO, captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	// 2. Load the HDR equirectangular map
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	float* data = stbi_loadf("Skybox/partly_cloudy_sky.hdr", &width, &height, &nrComponents, 0);
	unsigned int hdrTexture;
	if (data) {
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(data);
	}
	else {
		std::cout << "Failed to load HDR image." << std::endl;
	}

	// 3. Create an empty cubemap texture to render to
	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	for (unsigned int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 4. Define projection and view matrices for the 6 faces
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] = {
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	// 5. Render to each face of the cubemap
	equirectangularShader.use();
	equirectangularShader.setInt("uEquirectangularMap", 0);
	equirectangularShader.setMat4("uProjection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	glViewport(0, 0, 512, 512); // Configure the viewport to the capture dimensions
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i) {
		equirectangularShader.setMat4("uView", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemapTexture, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Render the cube
		glBindVertexArray(skyboxVAO); // We can reuse the skybox cube
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Resize the viewport back to the screen dimensions in your main loop
}

void Renderer::BeginFrame() {
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::DrawCube(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) {
	lightingShader->use();
	lightingShader->setMat4("uProjection", projection);
	lightingShader->setMat4("uView", view);
	lightingShader->setMat4("uModel", model);
	glBindVertexArray(cubeVAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Renderer::DrawSphere(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) {
	lightingShader->use();
	lightingShader->setMat4("uProjection", projection);
	lightingShader->setMat4("uView", view);
	lightingShader->setMat4("uModel", model);
	glBindVertexArray(sphereVAO);
	glDrawArrays(GL_TRIANGLES, 0, sphereVerticesCount);
	glBindVertexArray(0);
}

void Renderer::DrawCrosshair() {
	glDisable(GL_DEPTH_TEST);
	crosshairShader->use(); // Use the crosshair shader
	glBindVertexArray(crosshairVAO);
	glDrawArrays(GL_LINES, 0, 4);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
}

void Renderer::DrawSkybox(const glm::mat4& view, const glm::mat4& projection) {
	glDepthFunc(GL_LEQUAL); // Change depth function so depth test passes when values are equal to depth buffer's content
	skyboxShader->use();
	skyboxShader->setMat4("uView", view);
	skyboxShader->setMat4("uProjection", projection);

	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	skyboxShader->setInt("uSkybox", 0);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // Set depth function back to default
}

void Renderer::EndFrame() {
	// Handled in main: glfwSwapBuffers
}

void Renderer::SetViewPosition(const glm::vec3& position) {
	lightingShader->use();
	lightingShader->setVec3("uViewPos", position);
}

void Renderer::SetMaterial(const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float shininess) {
	lightingShader->use();
	lightingShader->setVec3("uMaterial.ambient", ambient);
	lightingShader->setVec3("uMaterial.diffuse", diffuse);
	lightingShader->setVec3("uMaterial.specular", specular);
	lightingShader->setFloat("uMaterial.shininess", shininess);
}

void Renderer::SetDirectionalLight(const glm::vec3& direction, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular) {
	lightingShader->use();
	lightingShader->setVec3("uDirLight.direction", direction);
	lightingShader->setVec3("uDirLight.ambient", ambient);
	lightingShader->setVec3("uDirLight.diffuse", diffuse);
	lightingShader->setVec3("uDirLight.specular", specular);
}

void Renderer::SetSpotLight(const glm::vec3& position, const glm::vec3& direction, float cutOff, float outerCutOff, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, float constant, float linear, float quadratic) {
	lightingShader->use();
	lightingShader->setVec3("uSpotLight.position", position);
	lightingShader->setVec3("uSpotLight.direction", direction);
	lightingShader->setFloat("uSpotLight.cutOff", cutOff);
	lightingShader->setFloat("uSpotLight.outerCutOff", outerCutOff);
	lightingShader->setVec3("uSpotLight.ambient", ambient);
	lightingShader->setVec3("uSpotLight.diffuse", diffuse);
	lightingShader->setVec3("uSpotLight.specular", specular);
	lightingShader->setFloat("uSpotLight.constant", constant);
	lightingShader->setFloat("uSpotLight.linear", linear);
	lightingShader->setFloat("uSpotLight.quadratic", quadratic);
}

void Renderer::ToggleSpotLight(bool enabled) {
	lightingShader->use();
	lightingShader->setBool("uSpotLightEnabled", enabled);
}