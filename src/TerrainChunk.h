#ifndef TERRAIN_CHUNK_H
#define TERRAIN_CHUNK_H

#include "Mesh.h"
#include "TerrainGenerator.h"
#include <unordered_map>
#include <vector>
#include <memory>

struct LodMeshInfo {
    MeshData data;      // CPU-side data (vertices + indices + skirt ranges)
    Mesh* mesh = nullptr; // GPU mesh (owns VBO/EBO/VAO) - will be deleted by TerrainChunk
    glm::vec3 minBounds = glm::vec3(0.0f);
    glm::vec3 maxBounds = glm::vec3(0.0f);

    // convenience
    bool valid() const { return mesh != nullptr && !data.vertices.empty(); }
};

class TerrainChunk {
public:
    int chunkX = 0;
    int chunkZ = 0;
    int baseCellsPerSide = 0; // the highest LOD cellsPerSide used when chunk was created
    float worldScale = 1.0f;

    bool lodReady[3];
    std::unordered_map<int, LodMeshInfo> lodMap;

    TerrainChunk(int cx, int cz, TerrainGenerator& gen, int cellsPerSide, float worldScale);
    TerrainChunk(int cx, int cz);
    ~TerrainChunk();

    void regenerate(TerrainGenerator& gen, int lodIndex, int cells);

    std::vector<float> exportHeights() const;

    void draw(int lodIndex, bool wireframe = false) const;

    static void computeBounds(const MeshData& data, glm::vec3& outMin, glm::vec3& outMax);

    bool hasLod(int lodIndex) const { return lodMap.find(lodIndex) != lodMap.end(); }
    const LodMeshInfo* getLodInfo(int lodIndex) const {
        auto it = lodMap.find(lodIndex);
        return (it == lodMap.end()) ? nullptr : &it->second;
    }

    void buildLodFromData(MeshData& m_data, int lodIndex);

    void setLodMesh(MeshData data, int lod);

    glm::vec3 getMin(int lod) const;
    glm::vec3 getMax(int lod) const;

private:
    LodMeshInfo buildLod(TerrainGenerator& gen, int cellsPerSide);
};

#endif
