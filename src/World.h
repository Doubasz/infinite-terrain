#ifndef WORLD_H
#define WORLD_H


#include "Camera.h"
#include "SkyBox.h"
#include "Terrain.h"
#include "TextureManager.h"
#include <glm/gtc/type_ptr.hpp>
#include <vector>

class World {
private:
    
    // Shaders
    Shader* skyShader;
    Shader* terrainShader;

    // Scene objects
    Camera* camera;
    SkyBox* skybox;
    Terrain* terrain;
    
    // Terrain generation
    TerrainGenerator generator;

    // Time tracking
    float elapsedTime;
    float growthTimer;
    float timeOfDay;


public:
    World();
    ~World();

    // Main loop methods
    void handleInput(int input, glm::vec2 mousePos, float dt);
    void update(float deltaTime);
    void render(float dt);

    // Accessors
    Camera* getCamera() const { return camera; }
};

// Utility function
glm::vec3 sunColorFromElevation(float elev);

#endif // WORLD_H