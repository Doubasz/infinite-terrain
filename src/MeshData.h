#ifndef MESH_DATA_H
#define MESH_DATA_H

#include <glm/glm.hpp>
#include <vector>

struct Vertex{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texCoord;
};


class MeshData{
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    void clear();

    size_t verticesCount() const;
    size_t indicesCount() const;
};

#endif