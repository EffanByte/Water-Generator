#define STB_IMAGE_IMPLEMENTATION
#define M_PI   3.14159265358979323846264338327950288
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"
#include <shader_m.h>
#include <iostream>
#include <math.h>
#include <math.h>
#include "noise.h"
#include <vector>

// Callback to resize the viewport
void framebuffer_size_callback(GLFWwindow* window, int width, int length) {
    glViewport(0, 0, width, length);
}
// Vertex structure
struct Vertex {
    float x, y, z;        // Position
    float nx, ny, nz;     // Normal
};

// Function to generate a plane's vertices
std::vector<Vertex> generatePlane(int width, int length, float seaLevel) {
    std::vector<Vertex> vertices;
    for (int z = 0; z <= length; ++z) {
        for (int x = 0; x <= width; ++x) {
            float xPos = (x / static_cast<float>(width)) * width;
            float zPos = (z / static_cast<float>(length)) * length;

            Vertex vertex;
            vertex.x = xPos - width / 2.0f;
            vertex.y = seaLevel;
            vertex.z = zPos - length / 2.0f;
            vertex.nx = 0.0f;
            vertex.ny = 1.0f;
            vertex.nz = 0.0f;

            vertices.push_back(vertex);
        }
    }
    return vertices;
}

// Function to generate the plane's VAO and return VAO & VBO
GLuint generatePlaneVAO(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, GLuint& VBO) {
    // Generate indices for the plane
    int width = static_cast<int>(sqrt(vertices.size())) - 1;
    int length = static_cast<int>(vertices.size() / (width + 1)) - 1;

    for (int z = 0; z < length; ++z) {
        for (int x = 0; x < width; ++x) {
            GLuint topLeft = z * (width + 1) + x;
            GLuint topRight = topLeft + 1;
            GLuint bottomLeft = (z + 1) * (width + 1) + x;
            GLuint bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);

            indices.push_back(topLeft);
            indices.push_back(bottomRight);
            indices.push_back(topRight);
        }
    }

    // Generate VAO
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Generate VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Generate EBO
    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    // Define vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0); // Unbind VAO
    return VAO;
}

// Function to update sea level
void updateSeaLevel(std::vector<Vertex>& vertices, float seaLevel, GLuint VAO, GLuint VBO) {
    for (auto& vertex : vertices) {
        vertex.y = seaLevel;
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Light source sphere generation
struct LightSource {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    GLuint VAO;
    int vertices_count;
};

std::vector<float> generateSphere(float radius, int sectors, int stacks) {
    std::vector<float> vertices;
    float sectorStep = 2 * M_PI / sectors;
    float stackStep = M_PI / stacks;

    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = 3.14 / 2 - i * stackStep;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep;
            // Position
            vertices.push_back(xy * cosf(sectorAngle));  // x
            vertices.push_back(xy * sinf(sectorAngle));  // y
            vertices.push_back(z);                       // z
            // Normal
            vertices.push_back(xy * cosf(sectorAngle) / radius);
            vertices.push_back(xy * sinf(sectorAngle) / radius);
            vertices.push_back(z / radius);
        }
    }
    return vertices;
}

LightSource createLightSource(glm::vec3 position, glm::vec3 color, float intensity) {
    LightSource light;
    light.position = position;
    light.color = color;
    light.intensity = intensity;

    // Generate sphere mesh for light source
    std::vector<float> sphereVertices = generateSphere(20.0f, 50, 50);
    light.vertices_count = sphereVertices.size() / 6;  // 6 floats per vertex (pos + normal)

    glGenVertexArrays(1, &light.VAO);
    glBindVertexArray(light.VAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return light;
}


unsigned int loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, length, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &length, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, length, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

bool isGuiOpen = false;  // Tracks whether the GUI menu is open

void initImGui(GLFWwindow* window) {
    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Set ImGui style
    ImGui::StyleColorsDark();
}

// ImGui variables //most from older terrain code, will clear up later
int octaves = 4;
float persistence = 0.5f;
int width = 256;
int length = 256;
float frequency = 2.0f;
int scale = 50;
float lengthScale = 10.0f;
float lacunarity = 2.0f;
float seaLevel = -10.0f;
bool renderingMode = true;
float testVar = 0.5;
glm::vec3 lightDir = glm::vec3(0.5, 0.5, testVar);

// Water variables (Change them to uniforms later?)
bool updateSea = false;
float seaFrequency = 0.2f; //median value
float seaAmplitude = 0.5f; // median value
float waveSpeed = 1.0f;    // median value
int waveCount = 4;
float u_time;

void renderImGuiMenu() {
    if (!isGuiOpen) return;  // Don't render if menu is closed

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    float oldSeaLevel = seaLevel;
    float oldSeaFrequency = seaFrequency;
    float oldSeaAmplitude = seaAmplitude;
    float oldWaveSpeed = waveSpeed;
    int oldWaveCount = waveCount;

    ImGui::Begin("Sea Settings");

    ImGui::SliderFloat("Sea Level", &seaLevel, -25.0f, 10.0f);
    ImGui::SliderFloat("Sea Frequency", &seaFrequency, 0.0f, 1.0f);
    ImGui::SliderFloat("Sea Amplitude", &seaAmplitude, 0.0f, 2.0f);
    ImGui::SliderFloat("Wave Speed", &waveSpeed, 0.0f, 0.01f);
    ImGui::SliderInt("Wave Count", &waveCount, 1, 10);
    ImGui::SliderFloat("Light Dir", &testVar, -1.0, 1.0);
    ImGui::Checkbox("Rendering Mode", &renderingMode);

    if (oldSeaLevel != seaLevel || oldSeaAmplitude != seaAmplitude || oldSeaFrequency != seaFrequency || oldWaveSpeed != waveSpeed
        || oldWaveCount != waveCount)
    {
        updateSea = true;
    }
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// Camera variables
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f); // Camera position
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // Camera facing direction
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); // Camera up direction
float yaw = -90.0f; // Initial yaw
float pitch = 0.0f; // Initial pitch
float lastX = 400, lastY = 300; // Mouse position
bool firstMouse = true; // First mouse movement flag;

float deltaTime = 0.0f; // Time between frames
float lastFrame = 0.0f; // Time of last frame

bool isMouseOverImGui = false;  // Flag to check if ImGui has mouse focus
bool cameraControlEnabled = true;

void processInput(GLFWwindow* window) {
    static bool eKeyPressed = false;  // Track E key state

    // Toggle GUI with E key
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        if (!eKeyPressed) {  // Only toggle once per press
            isGuiOpen = !isGuiOpen;
            if (isGuiOpen) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            eKeyPressed = true;
        }
    }
    else {
        eKeyPressed = false;
    }

    // Only process camera movement if GUI is closed
    if (!isGuiOpen) {
        float cameraSpeed = 40.0f * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
}



// Mouse callback for camera orientation
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (isGuiOpen) return;  // Don't process mouse movement when GUI is open

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version to 3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Terrain Renderer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Load OpenGL functions using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set the viewport and callback
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);


    // Shader setup (place the shaders in the same directory)
    Shader skyshader("skyshader.vs", "skyshader.fs");
  //  Shader seashader("seashader.vs", "seashader.fs", "seashader.gs");
  //  Shader normalshader("normalshader.vs", "normalshader.fs", "normalshader.gs");
   Shader seashader("seashadernogs.vs", "seashader.fs");
    Shader lightshader("lightshader.vs", "lightshader.fs");

    // Cube vertices
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // load textures
    // -------------
    std::vector<std::string> faces
    {
        "./textures/skybox/right.jpg",
        "./textures/skybox/left.jpg",
        "./textures/skybox/top.jpg",
        "./textures/skybox/bottom.jpg",
        "./textures/skybox/front.jpg",
        "./textures/skybox/back.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    skyshader.use();
    skyshader.setInt("skybox", 0);


    float planeWidth = width;
    float planelength = length;
    std::vector<Vertex> planeVertices = generatePlane(planeWidth, planelength, seaLevel);
    std::vector<GLuint> planeIndices;
    GLuint planeVBO;

    GLuint planeVAO = generatePlaneVAO(planeVertices, planeIndices, planeVBO);

    lightshader.use();

    LightSource light = createLightSource(glm::vec3(100,100,100), glm::vec3(0.5, 0.5, 0.5), 5.0f);

    // Background color     
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Register mouse callback
    glfwSetCursorPosCallback(window, mouse_callback);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    initImGui(window);

    // Render loop
    while (!glfwWindowShouldClose(window)) {

        // Calculate deltaTime
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input (camera movement)
        processInput(window);

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skyshader.use();
        // View matrix (camera position and orientation)
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        GLint viewLoc = glGetUniformLocation(skyshader.ID, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // Projection matrix (perspective projection)
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 1.0f, 500.0f);
        GLint projLoc = glGetUniformLocation(skyshader.ID, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        // In the render loop, before drawing:
        glm::mat4 model = glm::mat4(1.0f); // Identity matrix
        GLint modelLoc = glGetUniformLocation(skyshader.ID, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content

        lightshader.use();
        glm::mat4 lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel, light.position);
        glUniformMatrix4fv(glGetUniformLocation(lightshader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(lightshader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightshader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
        glUniform3fv(glGetUniformLocation(lightshader.ID, "lightColor"), 1, glm::value_ptr(light.color));

        glBindVertexArray(light.VAO);
        glDrawArrays(GL_TRIANGLES, 0, light.vertices_count);

        seashader.use();
        seashader.setVec3("lightDir", glm::vec3(0.5,0.7,0.3)); // Light position
        seashader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f)); // White light
        seashader.setFloat("lightIntensity", 1.0f);

        seashader.setVec3("viewPos", cameraPos); // Pass camera position

        // Material properties
        seashader.setVec3("objectColor", glm::vec3(0.2f, 0.6f, 0.9f)); // Water color
        seashader.setFloat("ambientStrength", 0.3f);
        seashader.setFloat("diffuseStrength", 0.6f);
        seashader.setFloat("specularStrength", 0.3f);
        seashader.setFloat("shininess", 64.0f);

        u_time = glfwGetTime();
        glUniform1f(glGetUniformLocation(seashader.ID, "u_time"), u_time);
        glUniform3fv(glGetUniformLocation(seashader.ID, "lightPos"), 1, glm::value_ptr(light.position));
        glUniform3fv(glGetUniformLocation(seashader.ID, "lightColor"), 1, glm::value_ptr(light.color));
        glUniform1f(glGetUniformLocation(seashader.ID, "lightIntensity"), light.intensity);
        glUniform3fv(glGetUniformLocation(seashader.ID, "viewPos"), 1, glm::value_ptr(view));
        glUniform1f(glGetUniformLocation(seashader.ID, "Dir"), testVar);
        glUniform1i(glGetUniformLocation(seashader.ID, "u_showNormals"), renderingMode);
        glm::vec3 planeColor(0.2f, 0.6f, 0.9f);
        glUniform3fv(glGetUniformLocation(seashader.ID, "objectColor"), 1, glm::value_ptr(planeColor));
            // Re-apply sea generation uniforms
            glUniform1f(glGetUniformLocation(seashader.ID, "seaLevel"), seaLevel);
            glUniform1f(glGetUniformLocation(seashader.ID, "seafrequency"), seaFrequency);
            glUniform1f(glGetUniformLocation(seashader.ID, "wave_speed"), waveSpeed);
            glUniform1f(glGetUniformLocation(seashader.ID, "sea_amplitude"), seaAmplitude);
            glUniform1f(glGetUniformLocation(seashader.ID, "wave_count"), waveCount);

            // Update sea level vertices
            updateSeaLevel(planeVertices, seaLevel, planeVAO, planeVBO);

        seashader.use();
        // Bind the VAO and draw the plane
        glBindVertexArray(planeVAO);
        glDrawElements(GL_TRIANGLES, planeIndices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        seashader.setMat4("model", model);
        seashader.setMat4("view", view);
        seashader.setMat4("projection", projection);

        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyshader.use();
        view = glm::mat4(glm::mat3(view)); // remove translation from the view matrix
        skyshader.setMat4("projection", projection);
        skyshader.setMat4("view", view);
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default


        // Only render ImGui if menu is open
        if (isGuiOpen) {
            renderImGuiMenu();
        }

        // Check if ImGui is hovered and manage camera control
        isMouseOverImGui = ImGui::IsAnyItemHovered() || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
        cameraControlEnabled = !isMouseOverImGui;

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
