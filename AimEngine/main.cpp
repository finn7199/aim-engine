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
Renderer renderer;
TargetManager targetManager(TARGET_COUNT, TARGET_MIN_X, TARGET_MAX_X, TARGET_MIN_Y, TARGET_MAX_Y, TARGET_Z, TARGET_RADIUS);
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024];
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool mouseLeftClick = false;
bool spotLightEnabled = false;

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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //Makes mouse infinite, hides mouse thou. Add crosshair!!  
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

        renderer.SetViewPosition(camera.Position);
        //renderer.SetMaterial(
        //    glm::vec3(0.7f, 0.7f, 0.7f),  // AMBIENT (high reflectivity)
        //    glm::vec3(1.0f, 1.0f, 1.0f),  // DIFFUSE 
        //    glm::vec3(0.8f, 0.8f, 0.8f),  // SPECULAR
        //    64.0f                         // SHININESS (sharp reflections)
        //);

        glClearColor(0.1f, 0.15f, 0.3f, 1.0f); // Deep, muted navy
        glClear(GL_COLOR_BUFFER_BIT); // Fills the screen with the specified color
        // Directional light (sunlight)
        renderer.SetDirectionalLight(
            glm::vec3(0.5f, -1.0f, -0.5f),  // Direction (slightly tilted for realism)
            glm::vec3(0.6f, 0.6f, 0.6f),    // AMBIENT (bright ambient like real daylight)
            glm::vec3(1.5f, 1.5f, 1.3f),    // DIFFUSE (bright white with slight warmth)
            glm::vec3(1.2f, 1.2f, 1.2f)     // SPECULAR (strong highlights)
        );

        // Spotlight (flashlight)
        renderer.SetSpotLight(
            camera.Position,
            camera.Front,
            glm::cos(glm::radians(12.5f)),  // cutOff
            glm::cos(glm::radians(17.5f)),  // outerCutOff
            glm::vec3(0.0f, 0.0f, 0.0f),    // ambient
            glm::vec3(1.0f, 1.0f, 1.0f),    // diffuse
            glm::vec3(1.0f, 1.0f, 1.0f),    // specular
            1.0f,                           // constant
            0.09f,                          // linear
            0.032f                          // quadratic
        );
        renderer.ToggleSpotLight(spotLightEnabled);

        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        // Draw targets
        renderer.SetMaterial(glm::vec3(0.2f, 0.0f, 0.0f), glm::vec3(0.8f, 0.1f, 0.1f), glm::vec3(0.5f), 32.0f);
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

        // --- Create the Room ---
        // 1. Floor
        glm::mat4 floorModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0f));
        floorModel = glm::scale(floorModel, glm::vec3(20.0f, 0.1f, 20.0f));
        // Set a unique material for the floor before drawing
        renderer.SetMaterial(glm::vec3(0.1f), glm::vec3(0.5, 0.5, 0.6), glm::vec3(0.2f), 16.0f);
        renderer.DrawCube(floorModel, view, projection);


        // 2. Back Wall (where targets appear)
        glm::mat4 backWallModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 4.0f, -12.0f));
        backWallModel = glm::scale(backWallModel, glm::vec3(20.0f, 12.0f, 0.2f));
        // Set a unique material for the back wall
        renderer.SetMaterial(glm::vec3(0.1f), glm::vec3(0.4, 0.4, 0.5), glm::vec3(0.5f), 32.0f);
        renderer.DrawCube(backWallModel, view, projection);

        // 3. Ceiling
        glm::mat4 ceilingModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, 0.0f));
        ceilingModel = glm::scale(ceilingModel, glm::vec3(20.0f, 0.1f, 20.0f));
        // Use the same material as the floor for consistency
        renderer.SetMaterial(glm::vec3(0.1f), glm::vec3(0.5, 0.5, 0.6), glm::vec3(0.2f), 16.0f);
        renderer.DrawCube(ceilingModel, view, projection);

        // 4. Left Wall
        glm::mat4 leftWallModel = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 4.0f, 0.0f));
        leftWallModel = glm::scale(leftWallModel, glm::vec3(0.2f, 12.0f, 20.0f));
        // Set a unique material for the side walls
        renderer.SetMaterial(glm::vec3(0.1f), glm::vec3(0.6, 0.6, 0.6), glm::vec3(0.3f), 16.0f);
        renderer.DrawCube(leftWallModel, view, projection);

        // 5. Right Wall
        glm::mat4 rightWallModel = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 4.0f, 0.0f));
        rightWallModel = glm::scale(rightWallModel, glm::vec3(0.2f, 12.0f, 20.0f));
        // Use the same material as the left wall
        renderer.SetMaterial(glm::vec3(0.1f), glm::vec3(0.6, 0.6, 0.6), glm::vec3(0.3f), 16.0f);
        renderer.DrawCube(rightWallModel, view, projection);

        renderer.DrawCrosshair();

        // Handle mouse click
        if (mouseLeftClick)
        {
            mouseLeftClick = false;
            glm::vec3 rayOrigin = camera.Position;
            glm::vec3 rayDirection = camera.Front; // The ray is the camera's forward direction

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
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            keys[key] = true;
            if (key == GLFW_KEY_E) {
                spotLightEnabled = !spotLightEnabled;
                renderer.ToggleSpotLight(spotLightEnabled);
            }
        }
        else if (action == GLFW_RELEASE) {
            keys[key] = false;
        }
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

