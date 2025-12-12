#include "World.h"
#include <iostream>

World::World() {

    TextureManager::loadTexture("grass", "assets/textures/grass.jpg");
    TextureManager::loadTexture("rock",  "assets/textures/rock.jpg");
    TextureManager::loadTexture("snow",  "assets/textures/snow.jpg");

    // Initialize shaders
    terrainShader = new Shader("shaders/terrain.vert", "shaders/terrain.frag");
    skyShader = new Shader("shaders/sky.vert", "shaders/sky.frag");
    skybox = new SkyBox("shaders/skybox.vert", "shaders/skybox.frag");

    
    // Initialize camera
    camera = new Camera(glm::vec3(0.0f, 10.0f, 30.0f));
    camera->setMode(FREE_MODE);

    // Terrain generator params
    TerrainGenerator::Params params;
    params.baseFrequency = 0.004f;
    params.octaves = 5;
    params.persistence = 0.48f;
    params.lacunarity = 2.0f;
    params.heightScale = 100.0f;
    params.seed = 42;
    generator = TerrainGenerator(params);

    terrain = new Terrain(7, 7, 65, 1.0, generator);

    elapsedTime = 0.0f;
    growthTimer = 0.0f;
    timeOfDay = 0;
}



void World::render(float dt) {
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f); // light sky blue
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = camera->getViewMatrix();
    glm::mat4 projection = glm::perspective(
        glm::radians(camera->fov),
        800.0f / 600.0f,
        0.1f,
        2000000.0f
    );


    terrainShader->use();
    glm::mat4 model = glm::mat4(1.0f); // identity
    terrainShader->setMat4("model", model);
    terrainShader->setMat4("view", view);
    terrainShader->setMat4("projection", projection);

    Frustum f = extractFrustum(projection * view);

    terrain->draw(f, camera->getPosition(), false);

    skybox->draw(view, projection, false);

}

void World::update(float deltaTime) {
    elapsedTime += deltaTime;
    
    terrain->update(deltaTime, camera->getPosition());
}

void World::handleInput(int input, glm::vec2 mousePos, float dt) {
    static bool mouseProcessedThisFrame = false;
    
    if (input == -1 || !mouseProcessedThisFrame) {
        camera->processMouseMovement(mousePos.x, mousePos.y, true);
        mouseProcessedThisFrame = (input != -1);
    }
    
    if (input == -1) {
        mouseProcessedThisFrame = false;
    }

    if (input >= 0) {
        if (camera->getMode() == FREE_MODE) {
            camera->processKeyboard(input, dt);
        }
    }
}

glm::vec3 sunColorFromElevation(float elev) {
    elev = glm::clamp((elev + 0.1f) / 1.1f, 0.0f, 1.0f);
    glm::vec3 sunriseColor = glm::vec3(1.0, 0.45, 0.15);
    glm::vec3 noonColor = glm::vec3(1.0, 0.95, 0.85);
    return glm::mix(sunriseColor, noonColor, elev);
}