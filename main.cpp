#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>
#include <vector>

#include "camera.h"
#include "collectible.h"
#include "font.h"
#include "imgui/imgui.h"
#include "imgui_glfw.h"
#include "imgui_opengl3.h"
#include "model.h"
#include "obstacle.h"
#include "player.h"
#include "shader.h"
#include "string.h"
#include "terrain.h"
#include "texture.h"
#include "wall.h"

#define DEMO 1

const unsigned int WIDTH  = 800;
const unsigned int HEIGHT = 600;

// TODO: move?
struct Frame {
    float deltaTime{0.0f};
    float lastFrame{0.0f};
};

struct Mouse {
    double lastX, lastY;
    bool firstMouse{true};
};

struct DirectionalLight {
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct Models {
    Model bunnyModel;
    Model sphereModel;
    Model cubeModel;
};

int waveNr{1};
#ifndef DEMO
int obstacleFactor{15};
int collectibleFactor{10};
glm::vec3 terrainScale{0.5f, 2.0f, 0.5f};
#else
int obstacleFactor{5};
int collectibleFactor{5};
glm::vec3 terrainScale{0.2f, 1.0f, 0.2f};
#endif

GLFWwindow* window;
Frame frame;
Shader shader;
Player player;
Camera camera;
Mouse mouse;
Terrain terrain;
Shader materialShader;
Shader terrainMaterialShader;
Shader skyboxShader;
Models models;
std::vector<Collectible> collectibles;
std::vector<Obstacle> obstacles;
std::vector<Wall> walls(4);
DirectionalLight dir_light;

#define POINT_LIGHT_COUNT 8
PointLight point_lights[POINT_LIGHT_COUNT];

Texture terrain_splatmap;
Texture terrain_textures[4];
Texture furTexture;
Texture collectibleTexture;
Texture rockTexture;
Texture lightRockTexture;
Texture skyboxTexture;

Model skybox;

Font menuFont;
Font interfaceFont;

bool inMainMenu;
bool inMenu{false};
bool isGameOver{false};

std::random_device r;
std::default_random_engine e1(r());

void continueGame() {
    inMenu           = false;
    mouse.firstMouse = true;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouseCallback(GLFWwindow* window, double xPos, double yPos) {
    if (mouse.firstMouse) {
        mouse.lastX      = xPos;
        mouse.lastY      = yPos;
        mouse.firstMouse = false;
    }

    double xOffset{xPos - mouse.lastX};
    double yOffset{yPos - mouse.lastY};
    mouse.lastX = xPos;
    mouse.lastY = yPos;

    if (!inMainMenu && !inMenu) {
        moveCamera(&camera, xOffset, yOffset);
    }
}

void keyCallback(GLFWwindow* window, int key, int scanCode, int action,
                 int mode) {
    // exit game
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        if (inMenu) {
            continueGame();
        } else if (inMainMenu) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            inMenu = true;
        }
    }

#ifndef DEMO
    // toggle cursor
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        glfwSetCursorPosCallback(window, NULL);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        glfwSetCursorPosCallback(window, mouseCallback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
#endif

    if (!inMainMenu && !inMenu) {
        // player movement
        if (key == GLFW_KEY_W) {
            if (action == GLFW_PRESS) player.moveDirection = 1.0f;
            if (action == GLFW_RELEASE) player.moveDirection = 0.0f;
        }
        if (key == GLFW_KEY_S) {
            if (action == GLFW_PRESS) player.moveDirection = -1.0f;
            if (action == GLFW_RELEASE) player.moveDirection = 0.0f;
        };
        if (key == GLFW_KEY_A) {
            if (action == GLFW_PRESS) player.turnDirection = 1.0f;
            if (action == GLFW_RELEASE) player.turnDirection = 0.0f;
        }
        if (key == GLFW_KEY_D) {
            if (action == GLFW_PRESS) player.turnDirection = -1.0f;
            if (action == GLFW_RELEASE) player.turnDirection = 0.0f;
        }

        // player jumping
        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
            if (player.canJump) {
                player.velocity.y = player.jumpPower;
                player.normal     = {0.0f, -1.0f, 0.0f};
                player.canJump    = false;
            }
        }
    }
}

void initGLFW() {
    // Init GLFW
    glfwInit();

    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a GLFWwindow object that we can use for GLFW's functions
    window =
        glfwCreateWindow(WIDTH, HEIGHT, "TSBK07 Project", nullptr, nullptr);
    if (window == nullptr) {
        error("Failed to create GLFW window\n");
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetKeyCallback(window, keyCallback);
}

void initGLEW() {
    // set this to true so GLEW knows to use a modern approach to retrieving
    // function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL function pointers
    if (GLEW_OK != glewInit()) {
        error("Failed to initialize GLEW\n");
        exit(-1);
    }
}

void loadModels() {
    loadModel(&models.sphereModel, "models/groundsphere.obj");
    loadModel(&models.bunnyModel, "models/bunnyplus.obj");
    loadModel(&models.cubeModel, "models/cube.obj");
}

void spawnPlayer() {
    TerrainPosition pos;
    terrainGetPosition(&terrain, 0.5f * terrain.width * terrain.scale.x,
                       0.5f * terrain.height * terrain.scale.z, &pos);
    player.initialize(pos.position, pos.normal, models.bunnyModel.radius, 5);
}

void spawnObstacles() {
    float terrWidth{terrain.width * terrain.scale.x};
    float terrHeight{terrain.height * terrain.scale.z};
    std::uniform_real_distribution<float> uniformDistX(2, terrWidth - 2);
    std::uniform_real_distribution<float> uniformDistZ(2, terrHeight - 2);

    for (int i = obstacles.size(); i < waveNr * obstacleFactor; i++) {
        float x = uniformDistX(e1);
        float z = uniformDistZ(e1);
        TerrainPosition pos;
        terrainGetPosition(&terrain, x, z, &pos);

        Obstacle obstacle{pos.position, pos.normal, models.sphereModel.radius};
        obstacles.push_back(obstacle);
    }
}

void spawnCollectibles() {
    float terrWidth{terrain.width * terrain.scale.x};
    float terrHeight{terrain.height * terrain.scale.z};
    std::uniform_real_distribution<float> uniformDistX(2, terrWidth - 2);
    std::uniform_real_distribution<float> uniformDistZ(2, terrHeight - 2);

    for (int i = 0; i < waveNr * collectibleFactor; i++) {
        float x{uniformDistX(e1)};
        float z{uniformDistZ(e1)};
        TerrainPosition pos;
        terrainGetPosition(&terrain, x, z, &pos);

        Collectible collectible{pos.position, models.sphereModel.radius};
        collectibles.push_back(collectible);
    }
}

void spawnWalls() {
    float terrWidth{terrain.width * terrain.scale.x};
    float terrHeight{terrain.height * terrain.scale.z};

    // south wall
    walls.at(0).position = {terrWidth / 2, 0.0f, 0.0f};
    walls.at(0).scale    = {terrWidth, walls.at(0).height, walls.at(0).width};

    // east wall
    walls.at(1).position = {0.0f, 0.0f, terrHeight / 2};
    walls.at(1).scale    = {walls.at(1).width, walls.at(1).height, terrHeight};

    // north wall
    walls.at(2).position = {terrWidth / 2, 0.0f, terrHeight};
    walls.at(2).scale    = {terrWidth, walls.at(2).height, walls.at(2).width};

    // west wall
    walls.at(3).position = {terrWidth, 0.0f, terrHeight / 2};
    walls.at(3).scale    = {walls.at(3).width, walls.at(3).height, terrHeight};
}

void restartGame() {
    isGameOver       = false;
    inMenu           = false;
    inMainMenu       = false;
    mouse.firstMouse = true;
    waveNr           = 1;
    obstacles.clear();
    collectibles.clear();
    spawnPlayer();
    spawnObstacles();
    spawnCollectibles();
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

bool button_draw(Font* font, float x, float y, const char* msg) {
    auto width  = font_width(font, msg);
    auto inside = false;
    auto click  = false;
    if (mouse.lastX > x - width * 0.5f && mouse.lastX < x + width * 0.5f) {
        if (mouse.lastY > HEIGHT - (y + 40) && mouse.lastY < HEIGHT - y) {
            int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            if (mouseState == GLFW_PRESS) {
                click = true;
            }
            inside = true;
        }
    }

    auto color = glm::vec3{1, 1, 1};
    if (inside) color = glm::vec3{1, 0, 0};
    font_draw(&menuFont, x - width * 0.5f, y, color, msg);

    return click;
}

void init() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable
    // Keyboard Controls io.ConfigFlags |=
    // ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glEnable(GL_TEXTURE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    shaderCompile(materialShader, "shaders/material.vert",
                  "shaders/material.frag");
    shaderCompile(terrainMaterialShader, "shaders/material.vert",
                  "shaders/terrain_material.frag");
    shaderCompile(shader, "shaders/main.vert", "shaders/main.frag");
    shaderCompile(skyboxShader, "shaders/skybox.vert", "shaders/skybox.frag");

    glm::mat4 projectionMatrix{glm::perspective(
        glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f)};
    shaderBind(materialShader);
    shaderSetMat4(materialShader, "projection", projectionMatrix);
    shaderBind(terrainMaterialShader);
    shaderSetMat4(terrainMaterialShader, "projection", projectionMatrix);
    shaderBind(shader);
    shaderSetMat4(shader, "projection", projectionMatrix);
    shaderBind(skyboxShader);
    shaderSetMat4(skyboxShader, "projection", projectionMatrix);

    auto terrain_texture = texture_load("textures/heightmap.png");
    terrainCreate(&terrain, &terrain_texture, terrainScale);

    terrain_splatmap    = texture_load("textures/terrain_splatmap.png");
    terrain_textures[0] = texture_load("textures/terrain_texture_01.png");
    terrain_textures[1] = texture_load("textures/terrain_texture_02.jpg");
    terrain_textures[2] = texture_load("textures/grass.png");
    terrain_textures[3] = texture_load("textures/grass2.png");

    loadModels();
    loadModel(&skybox, "models/skybox.obj");
    skyboxTexture = texture_load("textures/SkyBox512.png");

    spawnPlayer();
    spawnWalls();

    furTexture         = texture_load("textures/fur_texture.jpg");
    collectibleTexture = texture_load("textures/gold_texture.jpg");
    rockTexture        = texture_load("textures/rock_texture.jpg");
    lightRockTexture   = texture_load("textures/light_rock_texture.jpg");

    dir_light.direction = glm::vec3{-0.2f, -1.0f, -1.0f};
    dir_light.ambient   = glm::vec3{0.2f, 0.2f, 0.2f};
    dir_light.diffuse   = glm::vec3{0.5f, 0.5f, 0.5f};
    dir_light.specular  = glm::vec3{0.5f, 0.5f, 0.5f};

    for (auto i = 0; i < POINT_LIGHT_COUNT; i++) {
        point_lights[i].constant  = 1.0f;
        point_lights[i].quadratic = 1.0f;
    }

    point_lights[0].position = glm::vec3(4, 2, 4);
    point_lights[0].diffuse  = glm::vec3(1, 0, 0);

    point_lights[1].position  = glm::vec3(4, 2, 4);
    point_lights[1].diffuse   = glm::vec3(1, 1, 1);
    point_lights[1].quadratic = 1.0f;

    font_init();
    font_load(&menuFont, "fonts/zorque.ttf", 48);
    font_load(&interfaceFont, "fonts/zorque.ttf", 32);

    inMainMenu = true;
}

void display() {
    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

#ifndef DEMO
    // Imgui Windows
    {
        ImGui::Begin("Hello, world!");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::Text("Score: %d", player.score);
        ImGui::Text("Health: %d", player.health);
        ImGui::SliderFloat("Jump power", &player.jumpPower, 0.0f, 1.0f);
        ImGui::SliderFloat("Gravity", &player.gravity, -10.0f, 0.0f);

        if (ImGui::CollapsingHeader("Directional Light")) {
            ImGui::ColorEdit3("Ambient", glm::value_ptr(dir_light.ambient));
            ImGui::ColorEdit3("Diffuse", glm::value_ptr(dir_light.diffuse));
            ImGui::ColorEdit3("Specular", glm::value_ptr(dir_light.specular));
        }

        for (auto i = 0; i < POINT_LIGHT_COUNT; i++) {
            ImGui::PushID(i);
            char buffer[32];
            sprintf(buffer, "Point Light[%i]", i);
            if (ImGui::CollapsingHeader(buffer)) {
                ImGui::DragFloat3("Position",
                                  glm::value_ptr(point_lights[i].position),
                                  0.01f, -10.0f, 10.0f);
                ImGui::ColorEdit3("Ambient",
                                  glm::value_ptr(point_lights[i].ambient));
                ImGui::ColorEdit3("Diffuse",
                                  glm::value_ptr(point_lights[i].diffuse));
                ImGui::ColorEdit3("Specular",
                                  glm::value_ptr(point_lights[i].specular));

                ImGui::SliderFloat("Constant", &point_lights[i].constant, 0.0f,
                                   8.0f);
                ImGui::SliderFloat("Linear", &point_lights[i].linear, 0.0f,
                                   32.0f);
                ImGui::SliderFloat("Quadratic", &point_lights[i].quadratic,
                                   0.0f, 32.0f);
            }
            ImGui::PopID();
        }

        ImGui::End();
    }
#endif

    // Logic for each frame
    double currentFrame{glfwGetTime()};
    frame.deltaTime = currentFrame - frame.lastFrame;
    frame.lastFrame = currentFrame;

    if (!inMainMenu && !inMenu) {
        player.update(&terrain, walls, obstacles, &collectibles,
                      frame.deltaTime);

        for (auto& obstacle : obstacles)
            obstacle.update(&terrain, obstacles, walls, frame.deltaTime);

        if (collectibles.size() == 0) {
            waveNr++;
            spawnCollectibles();
            spawnObstacles();
        };

        if (player.health == 0) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            isGameOver       = true;
            inMainMenu       = true;
            player.highScore = player.score;
        }
    }
    cameraUpdatePosition(&camera, &player, walls);

    auto dark_scale = 1.0f;
    if (waveNr >= 3) dark_scale = 0.5f;
    if (waveNr >= 5) dark_scale = 0.1f;

    dir_light.direction = glm::vec3{-0.2f, -1.0f, -1.0f};
    dir_light.ambient   = glm::vec3{0.5f, 0.5f, 0.5f} * dark_scale;
    dir_light.diffuse   = glm::vec3{0.5f, 0.5f, 0.5f} * dark_scale;
    dir_light.specular  = glm::vec3{0.5f, 0.5f, 0.5f};

    point_lights[0].position = player.position + glm::vec3{0, 1, 0};

    // Move point-light to collectables
    auto pli = 1;
    for (auto i = pli; i < POINT_LIGHT_COUNT; i++) {
        point_lights[i].position = glm::vec3{0, 0, 0};
        point_lights[i].diffuse  = glm::vec3{0, 0, 0};
    }

    for (auto& c : collectibles) {
        if (pli < POINT_LIGHT_COUNT) {
            point_lights[pli].position = c.position + glm::vec3{0, 0.4f, 0};
            point_lights[pli].ambient  = glm::vec3{0, 0, 0};
            point_lights[pli].diffuse  = glm::vec3{2, 2, 0};
            point_lights[pli].specular = glm::vec3{1, 1, 0};
            pli++;
        }
    }

    // Render
    ImGui::Render();
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    {
        // Skybox
        shaderBind(skyboxShader);

        glm::mat4 view{getViewMatrix(&camera, &player)};
        shaderSetMat4(skyboxShader, "view", view);
        glDisable(GL_DEPTH_TEST);

        texture_bind(&skyboxTexture, 0);
        shaderSetTexture(skyboxShader, "tex", 0);

        // Follow the player around
        auto modelMatrix = glm::translate(glm::mat4(1.0f), player.position);
        modelMatrix *= glm::scale(glm::vec3{10, 10, 10});
        shaderSetMat4(skyboxShader, "model", modelMatrix);
        drawModel(&skybox);

        glEnable(GL_DEPTH_TEST);
    }

    // Setup MaterialShader for rendering
    shaderBind(materialShader);

    glm::mat4 view{getViewMatrix(&camera, &player)};
    shaderSetMat4(materialShader, "view", view);
    shaderSetVec3(materialShader, "view_position", camera.position);

    shaderSetVec3(materialShader, "dir_light.direction", dir_light.direction);
    shaderSetVec3(materialShader, "dir_light.ambient", dir_light.ambient);
    shaderSetVec3(materialShader, "dir_light.diffuse", dir_light.diffuse);
    shaderSetVec3(materialShader, "dir_light.specular", dir_light.specular);

    for (auto i = 0; i < POINT_LIGHT_COUNT; i++) {
        shaderSetVec3(materialShader, "point_light", i, "position",
                      point_lights[i].position);
        shaderSetVec3(materialShader, "point_light", i, "ambient",
                      point_lights[i].ambient);
        shaderSetVec3(materialShader, "point_light", i, "diffuse",
                      point_lights[i].diffuse);
        shaderSetVec3(materialShader, "point_light", i, "specular",
                      point_lights[i].specular);
        shaderSetFloat(materialShader, "point_light", i, "constant",
                       point_lights[i].constant);
        shaderSetFloat(materialShader, "point_light", i, "linear",
                       point_lights[i].linear);
        shaderSetFloat(materialShader, "point_light", i, "quadratic",
                       point_lights[i].quadratic);
    }

    {
        // Player
        texture_bind(&furTexture, 0);
        glm::mat4 modelMatrix{player.getMatrix()};
        shaderSetMat4(materialShader, "model", modelMatrix);
        shaderSetTexture(materialShader, "tex", 0);
        shaderSetVec3(materialShader, "material.diffuse", glm::vec3{1, 1, 1});
        shaderSetVec3(materialShader, "material.specular",
                      glm::vec3{0.1, 0.1, 0.1});
        shaderSetFloat(materialShader, "material.shininess", 32.0f);
        drawModel(&models.bunnyModel);
    }

    {
        // collectible
        texture_bind(&collectibleTexture, 0);
        shaderSetTexture(materialShader, "tex", 0);
        shaderSetVec3(materialShader, "material.diffuse", glm::vec3{1, 1, 1});
        shaderSetVec3(materialShader, "material.specular",
                      glm::vec3{0.1, 0.1, 0.1});
        shaderSetFloat(materialShader, "material.shininess", 32.0f);

        for (auto& collectible : collectibles) {
            glm::mat4 modelMatrix{collectible.getMatrix()};
            shaderSetMat4(materialShader, "model", modelMatrix);
            drawModel(&models.sphereModel);
        }
    }

    {
        // obstacle
        texture_bind(&rockTexture, 0);
        shaderSetTexture(materialShader, "tex", 0);
        shaderSetVec3(materialShader, "material.diffuse", glm::vec3{1, 1, 1});
        shaderSetVec3(materialShader, "material.specular",
                      glm::vec3{0.1, 0.1, 0.1});
        shaderSetFloat(materialShader, "material.shininess", 32.0f);
        for (auto& obstacle : obstacles) {
            glm::mat4 modelMatrix{obstacle.getMatrix()};
            shaderSetMat4(materialShader, "model", modelMatrix);
            drawModel(&models.sphereModel);
        }
    }

    {
        // walls
        texture_bind(&lightRockTexture, 0);
        for (auto& wall : walls) {
            glm::mat4 modelMatrix{getWallMatrix(&wall)};
            shaderSetMat4(materialShader, "model", modelMatrix);
            shaderSetTexture(materialShader, "tex", 0);
            shaderSetVec3(materialShader, "material.diffuse",
                          glm::vec3{1, 1, 1});
            shaderSetVec3(materialShader, "material.specular",
                          glm::vec3{0.1, 0.1, 0.1});
            shaderSetFloat(materialShader, "material.shininess", 32.0f);
            drawModel(&models.cubeModel);
        }
    }

    {
        shaderBind(terrainMaterialShader);
        glm::mat4 view{getViewMatrix(&camera, &player)};
        shaderSetMat4(terrainMaterialShader, "view", view);
        shaderSetVec3(terrainMaterialShader, "view_position", camera.position);

        shaderSetVec3(terrainMaterialShader, "dir_light.direction",
                      dir_light.direction);
        shaderSetVec3(terrainMaterialShader, "dir_light.ambient",
                      dir_light.ambient);
        shaderSetVec3(terrainMaterialShader, "dir_light.diffuse",
                      dir_light.diffuse);
        shaderSetVec3(terrainMaterialShader, "dir_light.specular",
                      dir_light.specular);

        for (auto i = 0; i < POINT_LIGHT_COUNT; i++) {
            shaderSetVec3(terrainMaterialShader, "point_light", i, "position",
                          point_lights[i].position);
            shaderSetVec3(terrainMaterialShader, "point_light", i, "ambient",
                          point_lights[i].ambient);
            shaderSetVec3(terrainMaterialShader, "point_light", i, "diffuse",
                          point_lights[i].diffuse);
            shaderSetVec3(terrainMaterialShader, "point_light", i, "specular",
                          point_lights[i].specular);
            shaderSetFloat(terrainMaterialShader, "point_light", i, "constant",
                           point_lights[i].constant);
            shaderSetFloat(terrainMaterialShader, "point_light", i, "linear",
                           point_lights[i].linear);
            shaderSetFloat(terrainMaterialShader, "point_light", i, "quadratic",
                           point_lights[i].quadratic);
        }

        texture_bind(&terrain_splatmap, 0);
        texture_bind(&terrain_textures[0], 1);
        texture_bind(&terrain_textures[1], 2);
        texture_bind(&terrain_textures[2], 3);
        texture_bind(&terrain_textures[3], 4);

        auto modelMatrix = glm::mat4(1);
        shaderSetMat4(terrainMaterialShader, "model", modelMatrix);
        shaderSetTexture(terrainMaterialShader, "splatmap", 0);
        shaderSetTexture(terrainMaterialShader, "textures[0]", 1);
        shaderSetTexture(terrainMaterialShader, "textures[1]", 2);
        shaderSetTexture(terrainMaterialShader, "textures[2]", 3);
        shaderSetTexture(terrainMaterialShader, "textures[3]", 4);

        shaderSetVec3(terrainMaterialShader, "material.diffuse",
                      glm::vec3{1, 1, 1});
        shaderSetVec3(terrainMaterialShader, "material.specular",
                      glm::vec3{0.0, 0.0, 0.0});
        shaderSetFloat(terrainMaterialShader, "material.shininess", 32.0f);
        drawModel(&terrain.model);
    }

    {
        // Text
        font_projection(WIDTH, HEIGHT);

        if (inMainMenu) {
            if (isGameOver) {
                auto game_over_width = font_width(&menuFont, "Game Over");
                font_draw(&menuFont, WIDTH / 2 - game_over_width / 2,
                          HEIGHT - HEIGHT / 4, "Game Over");
            } else {
                auto title_width = font_width(&menuFont, "TSBK07 Project");
                font_draw(&menuFont, WIDTH / 2 - title_width / 2,
                          HEIGHT - HEIGHT / 4, "TSBK07 Project");
            }

            font_drawf(&interfaceFont, 10, HEIGHT - 40, "High score: %i",
                       player.highScore);

            if (button_draw(&menuFont, WIDTH / 2, HEIGHT - 2 * HEIGHT / 4,
                            "Play")) {
                restartGame();
            }
            if (button_draw(&menuFont, WIDTH / 2, HEIGHT - 3 * HEIGHT / 4,
                            "Exit")) {
                glfwSetWindowShouldClose(window, GL_TRUE);
            }
        } else if (inMenu) {
            auto pause_width = font_width(&menuFont, "Paused");
            font_draw(&menuFont, WIDTH / 2 - pause_width / 2,
                      HEIGHT - HEIGHT / 5, "Paused");

            if (button_draw(&menuFont, WIDTH / 2, HEIGHT - 2 * HEIGHT / 5,
                            "Continue")) {
                continueGame();
            }
            if (button_draw(&menuFont, WIDTH / 2, HEIGHT - 3 * HEIGHT / 5,
                            "Restart")) {
                restartGame();
            }
            if (button_draw(&menuFont, WIDTH / 2, HEIGHT - 4 * HEIGHT / 5,
                            "Exit")) {
                glfwSetWindowShouldClose(window, GL_TRUE);
            }
        } else {
            font_drawf(&interfaceFont, 10, HEIGHT - 40, "Health: %i",
                       player.health);

            font_drawf(&interfaceFont, 10, HEIGHT - 70, "Score: %i (%zi left)",
                       player.score, collectibles.size());

            font_drawf(&interfaceFont, 10, HEIGHT - 100, "Wave: %i", waveNr);
        }
    }

    // Render ImGui frame
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
}

int main() {
    // Initialize program
    initGLFW();
    initGLEW();
    init();

    // Game loop
    while (!glfwWindowShouldClose(window)) display();

    // Clean up data
    // NOTE: If we're exiting cleaning up stuff is not required as the OS will
    // do it for us.
    cleanUpModel(&models.bunnyModel);
    cleanUpModel(&models.sphereModel);
    cleanUpModel(&models.cubeModel);
    cleanUpModel(&terrain.model);
    cleanUpModel(&skybox);

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}
