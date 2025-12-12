#ifndef TERRAIN_H
#define TERRAIN_H

#include "TerrainChunk.h"
#include "Camera.h"
#include <unordered_map>
#include <future>
#include <mutex>

struct ChunkKey {
    int x, z;

    bool operator==(const ChunkKey& other) const {
        return x == other.x && z == other.z;
    }
};

struct ChunkKeyHash {
    std::size_t operator()(const ChunkKey& k) const {
        // Fixed: proper 64-bit hash mixing
        std::size_t h1 = std::hash<int>{}(k.x);
        std::size_t h2 = std::hash<int>{}(k.z);
        return h1 ^ (h2 << 1);
    }
};

class Terrain{
public:
    Terrain(int chunksX, int chunksZ, int cellsPerSide, float worldScale, TerrainGenerator& generator);
    ~Terrain();

    void draw(const Frustum& f, glm::vec3 cameraPos, bool wireframe);
    ChunkKey worldToChunk(float worldX, float worldZ) const;
    void regenerateAround(int centerChunkX, int centerChunkZ, int radius);
    void update(float dt, const glm::vec3& cameraPos);
    void generateInitialTerrain(const glm::vec3 cameraPos);

    TerrainChunk* getChunk(int cx, int cz);

private:
    int chunksX, chunksZ;
    int cellsPerSide;
    float worldScale;
    TerrainGenerator& generator;
    int generateRadius;
    glm::vec3 lastCamPos;

    int updateFrameCounter = 0;  // Fixed: initialize to 0
    bool firstFrame;

    static constexpr float UNLOAD_DISTANCE = 1500.0f;
    static constexpr float MIN_MOVE_DISTANCE = 20.0f;
    static constexpr int UPDATE_INTERVAL = 8;

    std::unordered_map<ChunkKey, TerrainChunk*, ChunkKeyHash> chunks;
    std::unordered_map<ChunkKey, std::future<MeshData>, ChunkKeyHash> pendingFutures;
    std::unordered_map<ChunkKey, int, ChunkKeyHash> requestedLod;
    
    // CRITICAL: Mutex to protect generator access from multiple threads
    std::mutex generatorMutex;

    inline int indexFor(int cx, int cz) {return cz * chunksX + cx;} 
    int getLODForDistance(float distance);
    int lodCellsForIndex(int index);
    glm::vec3 getChunkCenterWorld(int cx, int cz);
    void unloadChunks(const glm::vec3 cameraPos);

    void requestChunkAsync(int cx, int cz, int lod);
    void finalizeReadyFutures();
};

#endif