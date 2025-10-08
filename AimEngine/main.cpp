#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "renderer.h"
#include "camera.h"
#include "target_manager.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

glm::vec3 g_lightDirection(-0.5f, -1.0f, -0.5f); //debug starting guess

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

        // Set the viewport back to the screen size after the cubemap conversion
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

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
            g_lightDirection,  // Direction (slightly tilted for realism) glm::vec3(-0.798228f, -0.412418f, -0.439026f);
            glm::vec3(5.0f, 5.0f, 5.0f) // higher value makes the light more intense.
        );

        // Spotlight (flashlight)
        renderer.SetSpotLight(
            camera.Position,
            camera.Front,
            glm::vec3(10.0f, 10.0f, 10.0f), // A bright white flashlight color
            glm::cos(glm::radians(12.5f)),  // cutOff
            glm::cos(glm::radians(17.5f)),  // outerCutOff
            1.0f,                           // constant
            0.09f,                          // linear
            0.032f                          // quadratic
        );
        renderer.ToggleSpotLight(spotLightEnabled);

        // Draw skybox first
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        renderer.DrawSkybox(view, projection);

        // Draw targets with a red, matte plastic material (albedo, metal, rough)
        renderer.SetMaterial(glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, 0.5f);
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

        // Create the Room
        // 1. Floor
        glm::mat4 floorModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0f));
        floorModel = glm::scale(floorModel, glm::vec3(20.0f, 0.1f, 20.0f));
        renderer.SetMaterial(glm::vec3(0.2f, 0.2f, 0.25f), 0.0f, 0.8f);
        renderer.DrawCube(floorModel, view, projection);

        // 2. Back Wall (where targets appear)
        glm::mat4 backWallModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 4.0f, -12.0f));
        backWallModel = glm::scale(backWallModel, glm::vec3(20.0f, 12.0f, 0.2f));
        renderer.SetMaterial(glm::vec3(0.15f, 0.15f, 0.18f), 0.0f, 0.9f);
        renderer.DrawCube(backWallModel, view, projection);

        // 3. Ceiling
        glm::mat4 ceilingModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, 0.0f));
        ceilingModel = glm::scale(ceilingModel, glm::vec3(20.0f, 0.1f, 20.0f));
        renderer.SetMaterial(glm::vec3(0.8f, 0.8f, 0.8f), 0.0f, 0.9f);
        renderer.DrawCube(ceilingModel, view, projection);

        // 4. Left Wall
        glm::mat4 leftWallModel = glm::translate(glm::mat4(1.0f), glm::vec3(-10.0f, 4.0f, 0.0f));
        leftWallModel = glm::scale(leftWallModel, glm::vec3(0.2f, 12.0f, 20.0f));
        renderer.SetMaterial(glm::vec3(0.5f, 0.5f, 0.5f), 0.0f, 0.4f);
        renderer.DrawCube(leftWallModel, view, projection);

        // 5. Right Wall
        glm::mat4 rightWallModel = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 4.0f, 0.0f));
        rightWallModel = glm::scale(rightWallModel, glm::vec3(0.2f, 12.0f, 20.0f));
        renderer.SetMaterial(glm::vec3(0.5f, 0.5f, 0.5f), 0.0f, 0.4f);
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

            //  DEBUG LOGIC 
            float step = 0.1f; // How much to change the vector by each key press
            if (key == GLFW_KEY_KP_8) g_lightDirection.y += step; // Numpad 8: Y+
            if (key == GLFW_KEY_KP_2) g_lightDirection.y -= step; // Numpad 2: Y-
            if (key == GLFW_KEY_KP_4) g_lightDirection.x -= step; // Numpad 4: X-
            if (key == GLFW_KEY_KP_6) g_lightDirection.x += step; // Numpad 6: X+
            if (key == GLFW_KEY_KP_7) g_lightDirection.z += step; // Numpad 7: Z+
            if (key == GLFW_KEY_KP_9) g_lightDirection.z -= step; // Numpad 9: Z-

            if (key == GLFW_KEY_P) { // 'P' to Print
                g_lightDirection = glm::normalize(g_lightDirection); // Normalize before printing
                std::cout << "Final Light Direction: glm::vec3("
                    << g_lightDirection.x << "f, "
                    << g_lightDirection.y << "f, "
                    << g_lightDirection.z << "f);" << std::endl;
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

