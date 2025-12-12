#include "Terrain.h"
#include <thread>
#include <vector>
#include <chrono>

Terrain::Terrain(int chunksX_, int chunksZ_, int cellsPerSide_, float worldScale_, TerrainGenerator& generator_)
    : chunksX(chunksX_), chunksZ(chunksZ_), cellsPerSide(cellsPerSide_), worldScale(worldScale_), generator(generator_)
{
    generateRadius = 16;
    lastCamPos = glm::vec3(0.0f);
    firstFrame = true;
    updateFrameCounter = 0;  // Initialize properly

    generateInitialTerrain(glm::vec3(0));
}

Terrain::~Terrain(){
    // Wait for all pending futures to complete before destroying
    for(auto& it : pendingFutures){
        if(it.second.valid()){
            it.second.wait();
        }
    }
    
    for(auto& it : chunks){
        delete it.second;
    }
    chunks.clear();
}

void Terrain::generateInitialTerrain(const glm::vec3 cameraPos){
    ChunkKey camKey = worldToChunk(cameraPos.x, cameraPos.z);
    int cx0 = camKey.x;
    int cz0 = camKey.z;

    for (int dz = -generateRadius; dz <= generateRadius; ++dz) {
        for (int dx = -generateRadius; dx <= generateRadius; ++dx) {
            int cx = dx + cx0;
            int cz = dz + cz0;
            ChunkKey key{cx, cz};

            // FIXED: Lock generator for synchronous initial generation
            MeshData data;
            {
                std::lock_guard<std::mutex> lock(generatorMutex);
                data = generator.generateChunk(cx, cz, lodCellsForIndex(0), worldScale);
            }

            TerrainChunk* chunk = new TerrainChunk(cx, cz, generator, cellsPerSide, worldScale);
            chunks[key] = chunk;
            chunk->buildLodFromData(data, 0);
        }
    }
}

void Terrain::draw(const Frustum& f, glm::vec3 cameraPos, bool wireframe)
{
    for (auto& it : chunks) {
        TerrainChunk* c = it.second;
        if (!c) continue;

        // Check if chunk has at least one LOD ready
        if (!c->lodReady[0] && !c->lodReady[1] && !c->lodReady[2]) {
            continue;  // Skip chunks with no LOD ready
        }

        glm::vec3 center = getChunkCenterWorld(it.first.x, it.first.z);
        float distance = glm::distance(cameraPos, center);
        int lod = getLODForDistance(distance);

        // Fallback to available LOD if requested one isn't ready
        if (!c->lodReady[lod]) {
            // Try LOD 0, then 1, then 2
            if (c->lodReady[0]) lod = 0;
            else if (c->lodReady[1]) lod = 1;
            else if (c->lodReady[2]) lod = 2;
            else continue;  // No LOD available
        }

        glm::vec3 minB = c->getMin(lod);
        glm::vec3 maxB = c->getMax(lod);

        if (!isInFrustum(f, minB, maxB))
            continue;

        c->draw(lod, wireframe);
    }
}

ChunkKey Terrain::worldToChunk(float worldX, float worldZ) const{
    int cx = static_cast<int>(std::floor(worldX / ((cellsPerSide - 1) * worldScale)));
    int cz = static_cast<int>(std::floor(worldZ / ((cellsPerSide - 1) * worldScale)));
    ChunkKey key{cx, cz};
    return key;
}

void Terrain::regenerateAround(int centerChunkX, int centerChunkZ, int radius){
    int minX = std::max(0, centerChunkX - radius);
    int maxX = std::min(chunksX - 1, centerChunkX + radius);
    int minZ = std::max(0, centerChunkZ - radius);
    int maxZ = std::min(chunksZ - 1, centerChunkZ + radius);

    for (int cz = minZ; cz <= maxZ; ++cz) {
        for (int cx = minX; cx <= maxX; ++cx) {
            ChunkKey key{cx, cz};
            auto it = chunks.find(key);
            
            if (it != chunks.end() && it->second) {
                // FIXED: Lock generator access
                std::lock_guard<std::mutex> lock(generatorMutex);
                it->second->regenerate(generator, 0, 65);
            }
        }
    }
}

TerrainChunk* Terrain::getChunk(int cx, int cz){
    ChunkKey key{cx, cz};
    auto it = chunks.find(key);
    
    if(it == chunks.end()){
        return nullptr;
    }
    
    return it->second;
}

int Terrain::getLODForDistance(float distance) {
    if(distance < 150) return 0; 
    if(distance < 300) return 1; 
    return 2;                     
}

void Terrain::update(float dt, const glm::vec3& cameraPos){
    if(firstFrame){
        firstFrame = false;
    }
    else{
        float dist = glm::distance(cameraPos, lastCamPos);
        if(updateFrameCounter++ < UPDATE_INTERVAL && dist < MIN_MOVE_DISTANCE){
            return;
        }
    }

    lastCamPos = cameraPos;
    updateFrameCounter = 0;

    // IMPORTANT: Finalize ready futures BEFORE checking for new requests
    finalizeReadyFutures();

    ChunkKey camKey = worldToChunk(cameraPos.x, cameraPos.z);
    int cx0 = camKey.x;
    int cz0 = camKey.z;

    const int MAX_NEW_REQUESTS_PER_FRAME = 8;
    int newRequestsThisFrame = 0;

    // Request chunks in a spiral pattern (better than nested loops)
    for(int dz = -generateRadius; dz <= generateRadius; ++dz){
        for(int dx = -generateRadius; dx <= generateRadius; ++dx){
            if (newRequestsThisFrame >= MAX_NEW_REQUESTS_PER_FRAME) break;

            int cx = dx + cx0;
            int cz = dz + cz0;
            ChunkKey key{cx, cz};

            glm::vec3 chunkCenter = getChunkCenterWorld(cx, cz);
            float distance = glm::distance(cameraPos, chunkCenter);
            int lod = getLODForDistance(distance);

            // Skip if already pending
            if (pendingFutures.count(key) > 0) continue;

            // Check if chunk exists and LOD is ready
            auto it = chunks.find(key);
            if (it != chunks.end()) {
                TerrainChunk* c = it->second;
                if (c && c->lodReady[lod]) continue;
            }

            // Request this LOD asynchronously
            requestChunkAsync(cx, cz, lod);
            requestedLod[key] = lod;
            ++newRequestsThisFrame;
        }
        if (newRequestsThisFrame >= MAX_NEW_REQUESTS_PER_FRAME) break;
    }

    unloadChunks(cameraPos);
}

void Terrain::unloadChunks(const glm::vec3 cameraPos){
    for(auto it = chunks.begin(); it != chunks.end();){
        glm::vec3 center = getChunkCenterWorld(it->first.x, it->first.z);
        float distance = glm::distance(cameraPos, center);

        if(distance > UNLOAD_DISTANCE){
            delete it->second;
            it = chunks.erase(it);
        }
        else{
            ++it;
        }
    }
}

glm::vec3 Terrain::getChunkCenterWorld(int cx, int cz){
    float chunkSize = (cellsPerSide - 1) * worldScale;
    float ox = cx * chunkSize;
    float oz = cz * chunkSize;

    ox += chunkSize * 0.5f;
    oz += chunkSize * 0.5f;

    return glm::vec3(ox, 0.0f, oz);
}

void Terrain::requestChunkAsync(int cx, int cz, int lod){
    ChunkKey key{cx, cz};

    if (pendingFutures.count(key) > 0) return;

    // CRITICAL FIX: Capture generator by reference, protect with mutex
    pendingFutures[key] = std::async(std::launch::async, 
        [this, cx, cz, lod]() -> MeshData {
            int cells = lodCellsForIndex(lod);
            
            // Lock the generator while generating mesh data
            std::lock_guard<std::mutex> lock(generatorMutex);
            MeshData data = this->generator.generateChunk(cx, cz, cells, this->worldScale);
            
            return data;
        });
}

int Terrain::lodCellsForIndex(int index){
    switch(index){
        case 0: return 65;
        case 1: return 33;
        case 2: return 17;
        default: return 17;  // Default to lowest LOD
    }
}

void Terrain::finalizeReadyFutures(){
    using namespace std::chrono_literals;
    std::vector<ChunkKey> finishedKeys;

    for(auto it = pendingFutures.begin(); it != pendingFutures.end(); ++it){
        ChunkKey key = it->first;
        auto &fut = it->second;

        // Check if future is valid and ready
        if(fut.valid() && fut.wait_for(0ms) == std::future_status::ready){
            MeshData data = fut.get();
            int lod = requestedLod[key];

            // Get or create chunk
            TerrainChunk* chunk = nullptr;
            auto chIt = chunks.find(key);
            if (chIt != chunks.end()){
                chunk = chIt->second;
            } else {
                // Create empty chunk (no initial generation)
                chunk = new TerrainChunk(key.x, key.z);
                chunk->baseCellsPerSide = cellsPerSide;
                chunk->worldScale = worldScale;
                chunks[key] = chunk;
            }

            // Build this specific LOD
            chunk->buildLodFromData(data, lod);
            finishedKeys.push_back(key);
        }
    }

    // Cleanup finished futures
    for (auto &k : finishedKeys) {
        pendingFutures.erase(k);
        requestedLod.erase(k);
    }
}