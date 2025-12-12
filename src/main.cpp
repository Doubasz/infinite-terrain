
#include "glad/glad.h"
#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <vector>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "World.h"
#include <glm/gtc/type_ptr.hpp>


// Window resize callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

std::vector<int> getInputs(GLFWwindow* window);

int main(){

    //Simulation s = Simulation();

    // Initialise GLFW
    glfwInit();



    // Tell GLFW what version of OpenGL we are using
    // In this case we are using 3.3 core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
     
    // Tell GLFW we are using the CORE profile
    // So that means we are only using modern functions
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int width = 800;
    int height = 600;

    

    // Creates a GLFW window of 800x600 px
    GLFWwindow* window = glfwCreateWindow(800, 600, "Simulation", NULL, NULL);
    if(window == NULL){
        std::cerr << "Failed to create GLFWwindow" << std::endl;
        glfwTerminate();
        return -1;
    }

    
    // Introduce the window into the current context
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Disable V-Sync
    glfwSwapInterval(0);

    

    // Load glad so that it configures OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    World* w = new World();

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);

    static bool lockMouse = true;
    static bool escapePressed = false;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



    float lastFrame = 0.0f;

    
    Camera* camera = w->getCamera();
    
    double mouseX, mouseY;
    glm::vec2 mousePos;

    static double lastMouseX = 400.0;
    static double lastMouseY = 300.0;
    static bool firstMouse = true;

    glfwGetCursorPos(window, &mouseX, &mouseY);
    mousePos.x = mouseX;
    mousePos.y = mouseY;

    glm::vec2 mouseDelta;

    //camera->lastMousePos = mousePos;


    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);

    float hud_padding = 0.0f; // distance from edges
    ImVec2 window_pos = ImVec2(display_w - hud_padding, hud_padding);
    ImVec2 window_pos_pivot = ImVec2(1.0f, 0.0f); // align top-right


    while(!glfwWindowShouldClose(window)){
        
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;

        glfwGetCursorPos(window, &mouseX, &mouseY);
        mouseDelta.x = mouseX - lastMouseX;
        mouseDelta.y = lastMouseY - mouseY; // Reversed: y-coordinates go from bottom to top

        lastMouseX = mouseX;
        lastMouseY = mouseY;

        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
            if(!escapePressed) {  // Only toggle once per press
                lockMouse = !lockMouse;  // Use ! not ~
                
                if(lockMouse){
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                } else {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
                
                escapePressed = true;
            }
        } else {
            escapePressed = false;  // Reset when key is released
        }
        
        // Get all pressed keys
        std::vector<int> inputs = getInputs(window);
        
        // Process each input
        for(int input : inputs) {
            w->handleInput(input, mouseDelta, deltaTime);
        }
        
        // Always pass mouse movement
        w->handleInput(-1, mouseDelta, deltaTime);

        w->update(deltaTime);
        

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui window
        ImGui::Begin("Camera Settings");
        ImGui::SliderFloat("Speed", &camera->movementSpeed, 0.1f, 10.0f);
        ImGui::SliderFloat("Mouse Sensitivity", &camera->mouseSensitivity, 0.001f, 0.02f);
        //ImGui::SliderFloat("Camera Radius", &camera->radius, 1.0f, 50.0f);
        ImGui::Text("Use these sliders to tweak camera parameters in real-time!");
        ImGui::End();

        // FPS HUD window (top-left corner)
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot); // position
        ImGui::SetNextWindowBgAlpha(0.35f); // transparent background
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration |
                                        ImGuiWindowFlags_AlwaysAutoResize |
                                        ImGuiWindowFlags_NoSavedSettings |
                                        ImGuiWindowFlags_NoFocusOnAppearing |
                                        ImGuiWindowFlags_NoNav;

        ImGui::Begin("FPS Overlay", nullptr, window_flags);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Frame: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
        ImGui::End();

        w->render(deltaTime);

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    
    }


    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();

    return 0;
}


// Returns a vector of all currently pressed input keys
std::vector<int> getInputs(GLFWwindow* window){
    std::vector<int> inputs;
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        inputs.push_back(0);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        inputs.push_back(1);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        inputs.push_back(2);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        inputs.push_back(3);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        inputs.push_back(4);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        inputs.push_back(5);
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
        inputs.push_back(6);
    
    return inputs;
}