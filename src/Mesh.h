#ifndef MESH_H
#define MESH_H


#include "glad/glad.h" 
#include <glm/glm.hpp>
#include <vector>
#include "MeshData.h"





class Mesh{
public:
    unsigned int VAO, VBO, EBO;
    unsigned int indexCount;


    Mesh(float* vertices, size_t vertSize, unsigned int* indices, size_t idxSize);
    Mesh(const MeshData& data);
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned>& indices);
    Mesh(const std::vector<float>& vertices, const std::vector<unsigned>& indices);
    ~Mesh();

    void draw();
    void drawSolid();
    void drawWireframe();
};

Mesh* createCubeMesh(float size);
Mesh* getRectangleMesh(float w, float h, float d, glm::vec3 color);
Mesh* generateTerrain(int width, int length, float scale, float heightScale);


#endif