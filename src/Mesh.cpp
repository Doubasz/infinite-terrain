
#include "Mesh.h"
#include <glm/gtc/noise.hpp>



Mesh::Mesh(float* vertices, size_t vertSize, unsigned int* indices, size_t idxSize){
    indexCount = idxSize / sizeof(unsigned int);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertSize, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxSize, indices, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    // Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}


Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices){
    indexCount = indices.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    // Color
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(1);

    // Texture
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}



Mesh::Mesh(const std::vector<float>& vertices, const std::vector<unsigned>& indices){
    indexCount = indices.size();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    GLsizei stride = 8 * sizeof(float); // 3 pos + 3 normal + 2 texcoords

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // Texcoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}


Mesh::Mesh(const MeshData& meshData){
    indexCount = meshData.indicesCount();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);



    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, meshData.verticesCount() * sizeof(Vertex), meshData.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshData.indicesCount() * sizeof(unsigned int), meshData.indices.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    // Color
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);

    // Texture
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
}


Mesh::~Mesh(){
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}


void Mesh::draw(){

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::drawSolid(){
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


void Mesh::drawWireframe(){
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_CULL_FACE);       // optional: see all edges
    glLineWidth(1.0f);             // change thickness

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glEnable(GL_CULL_FACE);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


Mesh* createCubeMesh(float size){
    // Cube vertices scaled by size
    float cubeVertices[] = {
        -0.5f*size, -0.5f*size, -0.5f*size, 0.125f, 0.798f, 0.961f,
         0.5f*size, -0.5f*size, -0.5f*size, 0.125f, 0.798f, 0.961f,
         0.5f*size,  0.5f*size, -0.5f*size, 0.125f, 0.798f, 0.961f,
        -0.5f*size,  0.5f*size, -0.5f*size, 0.125f, 0.798f, 0.961f,
        -0.5f*size, -0.5f*size,  0.5f*size, 0.125f, 0.798f, 0.961f,
         0.5f*size, -0.5f*size,  0.5f*size, 0.125f, 0.798f, 0.961f,
         0.5f*size,  0.5f*size,  0.5f*size, 0.125f, 0.798f, 0.961f,
        -0.5f*size,  0.5f*size,  0.5f*size, 0.125f, 0.798f, 0.961f
    };

    unsigned int cubeIndices[] = {
        0,1,2,2,3,0,
        4,5,6,6,7,4,
        0,4,7,7,3,0,
        1,5,6,6,2,1,
        3,2,6,6,7,3,
        0,1,5,5,4,0
    };

    // Create mesh
    Mesh* mesh = new Mesh(cubeVertices, sizeof(cubeVertices), cubeIndices, sizeof(cubeIndices));
    return mesh;
}


// Creates a rectangle (box) centered at origin
// x = width, y = height, z = depth
Mesh* getRectangleMesh(float w, float h, float d, glm::vec3 color) {
    float hx = w * 0.5f; // half-width
    float hy = h * 0.5f; // half-height
    float hz = d * 0.5f; // half-depth

    // 8 vertices with colors
    std::vector<Vertex> vertices = {
        {{-hx, -hy, -hz}, {color.x, color.y, color.z}}, // 0 - red
        {{ hx, -hy, -hz}, {color.x, color.y, color.z}}, // 1 - green
        {{ hx, -hy,  hz}, {color.x, color.y, color.z}}, // 2 - blue
        {{-hx, -hy,  hz}, {color.x, color.y, color.z}}, // 3 - yellow
        {{-hx,  hy, -hz}, {color.x, color.y, color.z}}, // 4 - magenta
        {{ hx,  hy, -hz}, {color.x, color.y, color.z}}, // 5 - cyan
        {{ hx,  hy,  hz}, {color.x, color.y, color.z}}, // 6 - white
        {{-hx,  hy,  hz}, {color.x, color.y, color.z}}, // 7 - gray
    };

    // 12 triangles (36 indices) for a cube
    std::vector<unsigned int> indices = {
        // bottom
        0, 1, 2,  2, 3, 0,
        // top
        4, 5, 6,  6, 7, 4,
        // front
        3, 2, 6,  6, 7, 3,
        // back
        0, 1, 5,  5, 4, 0,
        // left
        0, 3, 7,  7, 4, 0,
        // right
        1, 2, 6,  6, 5, 1
    };

    Mesh* mesh = new Mesh(vertices, indices);
    return mesh;
}


Mesh* generateTerrain(int width, int length, float scale, float heightScale) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // Generate vertices
    for (int z = 0; z < length; ++z) {
        for (int x = 0; x < width; ++x) {
            // glm::perlin takes vec2 input for 2D noise and returns [-1, 1]
            glm::vec2 pos(x * 0.1f, z * 0.1f);
            float y = glm::perlin(pos) * heightScale; // [-heightScale, heightScale]

            Vertex v;
            v.position = glm::vec3(x * scale, y, z * scale);
            v.color = glm::vec3(0.2f + (y + heightScale) / (2.0f * heightScale) * 0.8f, 0.5f, 0.2f); // green gradient
            v.normal = glm::vec3(0.0f, 1.0f, 0.0f); // temporary normal
            vertices.push_back(v);
        }
    }

    // Generate indices (grid -> triangles)
    for (int z = 0; z < length - 1; ++z) {
        for (int x = 0; x < width - 1; ++x) {
            int topLeft = z * width + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * width + x;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    // Compute normals
    for (size_t i = 0; i < indices.size(); i += 3) {
        Vertex &v0 = vertices[indices[i]];
        Vertex &v1 = vertices[indices[i + 1]];
        Vertex &v2 = vertices[indices[i + 2]];

        glm::vec3 edge1 = v1.position - v0.position;
        glm::vec3 edge2 = v2.position - v0.position;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        v0.normal += normal;
        v1.normal += normal;
        v2.normal += normal;
    }

    // Normalize normals
    for (auto &v : vertices) {
        v.normal = glm::normalize(v.normal);
    }

    return new Mesh(vertices, indices);
}