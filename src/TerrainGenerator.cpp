#include "TerrainGenerator.h"
#include <glm/gtc/noise.hpp>

TerrainGenerator::TerrainGenerator(const Params& p)
    : params(p), rng(p.seed), dist(0.0, 1.0)
{
}

void TerrainGenerator::setParams(const Params& p){
    params = p;
    rng.seed(params.seed);
}

const TerrainGenerator::Params& TerrainGenerator::getParams() const {
    return params;
}

float TerrainGenerator::fractalPerlin(float x, float z) const{
    float amplitude = 1.0f;
    float frequency = params.baseFrequency;
    float total = 0.0f;
    float maxAmp = 0.0f;

    for(int o = 0; o < params.octaves; ++o){
        // glm::perlin is thread-safe (pure function, no state)
        float n = glm::perlin(glm::vec2(x * frequency, z * frequency));
        
        total += n * amplitude;
        maxAmp += amplitude;
        amplitude *= params.persistence;
        frequency *= params.lacunarity;
    }
  
    if(maxAmp > 0.0f) {
        total /= maxAmp; // normalize to [-1,1]
    }
    return total;
}

float TerrainGenerator::getHeightAt(float worldX, float worldZ) const {
    float n = fractalPerlin(worldX, worldZ);
    return n * params.heightScale;
}

MeshData TerrainGenerator::generateChunk(int chunkX, int chunkZ, int cellsPerSide, float worldScale)
{
    MeshData out;

    const int fullCells = HIGH_LOD_CELLS;                   
    const float fullSize = (fullCells - 1) * worldScale;

    // Allocate memory upfront
    const size_t numVertices = size_t(cellsPerSide) * size_t(cellsPerSide);
    const size_t numIndices  = size_t(cellsPerSide - 1) * size_t(cellsPerSide - 1) * 6;

    out.vertices.resize(numVertices);
    out.indices.resize(numIndices);

    // LOD visualization colors
    glm::vec3 greenColor(0.15f, 0.2f, 0.12f);
    glm::vec3 lodColor = glm::mix(
        glm::vec3(1.0f, 0.0f, 0.0f), 
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::clamp((cellsPerSide - 17.0f) / (65.0f - 17.0f), 0.0f, 1.0f)
    );

    float chunkOriginX = chunkX * fullSize;
    float chunkOriginZ = chunkZ * fullSize;

    // ------------- Vertices ---------------------
    #pragma omp parallel for collapse(2)
    for(int row = 0; row < cellsPerSide; ++row)
    {
        for(int col = 0; col < cellsPerSide; ++col)
        {
            float u = col / float(cellsPerSide - 1);
            float v = row / float(cellsPerSide - 1);

            float wx = chunkOriginX + u * fullSize;
            float wz = chunkOriginZ + v * fullSize;

            float h = getHeightAt(wx, wz);

            Vertex vtx;
            vtx.position = glm::vec3(wx, h, wz);

            // Color based on height
            float t = glm::clamp((h / params.heightScale + 1.0f) * 0.5f, 0.0f, 1.0f);
            vtx.color = glm::mix(greenColor, lodColor, t);

            vtx.normal = glm::vec3(0.0f);  // Will be computed later
            vtx.texCoord = glm::vec2(u, v);

            size_t idx = row * cellsPerSide + col;
            out.vertices[idx] = vtx;
        }
    }

    // ------------- Indices ---------------------
    #pragma omp parallel for collapse(2)
    for(int row = 0; row < cellsPerSide - 1; ++row)
    {
        for(int col = 0; col < cellsPerSide - 1; ++col)
        {
            size_t idxBase = (row * (cellsPerSide - 1) + col) * 6;

            uint32_t tl = row * cellsPerSide + col;
            uint32_t tr = tl + 1;
            uint32_t bl = (row + 1) * cellsPerSide + col;
            uint32_t br = bl + 1;

            // Triangle 1
            out.indices[idxBase]     = tl;
            out.indices[idxBase + 1] = bl;
            out.indices[idxBase + 2] = tr;

            // Triangle 2
            out.indices[idxBase + 3] = tr;
            out.indices[idxBase + 4] = bl;
            out.indices[idxBase + 5] = br;
        }
    }

    // ------------- Normal Calculation ---------------------
    // Accumulate face normals, then normalize per-vertex
    std::vector<glm::vec3> tempNormals(numVertices, glm::vec3(0.0f));

    #pragma omp parallel for
    for(size_t i = 0; i < out.indices.size(); i += 3)
    {
        uint32_t ia = out.indices[i];
        uint32_t ib = out.indices[i + 1];
        uint32_t ic = out.indices[i + 2];

        glm::vec3 A = out.vertices[ia].position;
        glm::vec3 B = out.vertices[ib].position;
        glm::vec3 C = out.vertices[ic].position;

        glm::vec3 edge1 = B - A;
        glm::vec3 edge2 = C - A;
        glm::vec3 N = glm::cross(edge1, edge2);
        
        // Don't normalize yet - we want weighted average
        // (larger triangles contribute more)

        // Atomic operations for thread safety
        #pragma omp atomic
        tempNormals[ia].x += N.x;
        #pragma omp atomic
        tempNormals[ia].y += N.y;
        #pragma omp atomic
        tempNormals[ia].z += N.z;

        #pragma omp atomic
        tempNormals[ib].x += N.x;
        #pragma omp atomic
        tempNormals[ib].y += N.y;
        #pragma omp atomic
        tempNormals[ib].z += N.z;

        #pragma omp atomic
        tempNormals[ic].x += N.x;
        #pragma omp atomic
        tempNormals[ic].y += N.y;
        #pragma omp atomic
        tempNormals[ic].z += N.z;
    }

    // Normalize accumulated normals
    #pragma omp parallel for
    for(size_t i = 0; i < numVertices; ++i) {
        float len = glm::length(tempNormals[i]);
        if (len > 0.0f) {
            out.vertices[i].normal = tempNormals[i] / len;
        } else {
            out.vertices[i].normal = glm::vec3(0.0f, 1.0f, 0.0f);  // Default up
        }
    }

    return out;
}