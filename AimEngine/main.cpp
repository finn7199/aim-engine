#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "renderer.h"
#include "camera.h"
#include "target_manager.h"

// Function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
glm::vec3 GetRayFromMouse(float mouseX, float mouseY, const glm::mat4& projection, const glm::mat4& view);

// Screen dimensions
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// Target parameters
const float TARGET_MIN_X = -5.0f;
const float TARGET_MAX_X = 5.0f;
const float TARGET_MIN_Y = 1.0f;
const float TARGET_MAX_Y = 6.0f;
const float TARGET_Z = -10.0f;
const float TARGET_RADIUS = 0.25f;
const int TARGET_COUNT = 10;

// Globals
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool mouseLeftClick = false;
TargetManager targetManager(TARGET_COUNT, TARGET_MIN_X, TARGET_MAX_X, TARGET_MIN_Y, TARGET_MAX_Y, TARGET_Z, TARGET_RADIUS);

int main()
{
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Aim Trainer - OpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);

    // Load GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    // Setup renderer
    Renderer renderer;
    renderer.Init();

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        camera.ProcessKeyboard(keys, deltaTime);

        renderer.BeginFrame();

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        // Draw targets
        for (auto& target : targetManager.targets)
        {
            if (!target.hit)
            {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, target.position);
                model = glm::scale(model, glm::vec3(target.radius));
                renderer.DrawSphere(model, view, projection);
            }
        }

        // Draw the Cube (as ground)
        glm::mat4 groundModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.5f, 0.0f));
        groundModel = glm::scale(groundModel, glm::vec3(10.0f, 0.1f, 10.0f)); // Scale to a plane
        renderer.DrawCube(groundModel, view, projection, glm::vec3(0.3f, 0.3f, 1.0f));

        // Draw a wall
        glm::mat4 wallModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.75f, 5.0f));
        wallModel = glm::scale(wallModel, glm::vec3(10.0f, 5.0f, 0.2f));
        renderer.DrawCube(wallModel, view, projection, glm::vec3(0.8f, 0.2f, 0.2f)); // Red wall

        // Handle mouse click
        if (mouseLeftClick)
        {
            mouseLeftClick = false;

            // Get ray from camera to mouse position
            glm::vec3 rayOrigin = camera.Position;
            glm::vec3 rayDirection = GetRayFromMouse(lastX, lastY, projection, view);

            // Check for hits
            if (targetManager.CheckHits(rayOrigin, rayDirection))
            {
                targetManager.ResetHitTargets(TARGET_MIN_X, TARGET_MAX_X, TARGET_MIN_Y, TARGET_MAX_Y, TARGET_Z);
            }
        }

        renderer.EndFrame();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}


// Handle input
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Resize callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)(xpos - lastX);
    float yoffset = (float)(ypos - lastY);
    lastX = (float)xpos;
    lastY = (float)ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        mouseLeftClick = true;
    }
}
// Function to create a ray from mouse position
glm::vec3 GetRayFromMouse(float mouseX, float mouseY, const glm::mat4& projection, const glm::mat4& view) {
    // Convert mouse coordinates to normalized device coordinates
    float x = (2.0f * mouseX) / SCR_WIDTH - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / SCR_HEIGHT;
    float z = 1.0f;

    // Create ray in clip space
    glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);

    // Convert to eye space
    glm::vec4 rayEye = glm::inverse(projection) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    // Convert to world space
    glm::vec3 rayWorld = glm::vec3(glm::inverse(view) * rayEye);
    return glm::normalize(rayWorld);
}

