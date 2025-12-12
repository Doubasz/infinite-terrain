#include "TerrainChunk.h"
#include <limits>

// Full constructor - no longer generates LODs automatically
TerrainChunk::TerrainChunk(int cx, int cz, TerrainGenerator& gen, int cellsPerSide, float m_worldScale)
    : chunkX(cx), chunkZ(cz), baseCellsPerSide(cellsPerSide), worldScale(m_worldScale)
{
    for(int i = 0; i < 3; i++) {
        lodReady[i] = false;
    }
    
    // NOTE: We no longer auto-generate LODs here. 
    // The Terrain class will call buildLodFromData() when mesh data is ready.
}

// Minimal constructor for async chunk creation
TerrainChunk::TerrainChunk(int cx, int cz)
    : chunkX(cx), chunkZ(cz), baseCellsPerSide(0), worldScale(1.0f)
{
    for(int i = 0; i < 3; ++i){
        lodReady[i] = false;
    }
}

TerrainChunk::~TerrainChunk()
{
    // Delete GPU meshes we own
    for (auto &p : lodMap) {
        if (p.second.mesh) {
            delete p.second.mesh;
            p.second.mesh = nullptr;
        }
    }
    lodMap.clear();
}

LodMeshInfo TerrainChunk::buildLod(TerrainGenerator& gen, int cellsPerSide)
{
    LodMeshInfo info;
    info.data = gen.generateChunk(chunkX, chunkZ, cellsPerSide, worldScale);
    computeBounds(info.data, info.minBounds, info.maxBounds);
    info.mesh = new Mesh(info.data);
    return info;
}

void TerrainChunk::buildLodFromData(MeshData& m_data, int lodIndex){
    // IMPORTANT: Don't rebuild if this LOD already exists
    // This prevents overwriting LODs that are still being used
    if (lodReady[lodIndex] && lodMap.count(lodIndex) > 0) {
        // Optional: You could update it, but be careful with threading
        delete lodMap[lodIndex].mesh;
    }
    
    LodMeshInfo info;
    info.data = std::move(m_data);  // Use move semantics for efficiency
    computeBounds(info.data, info.minBounds, info.maxBounds);
    info.mesh = new Mesh(info.data);

    lodMap[lodIndex] = std::move(info);
    lodReady[lodIndex] = true;
}

void TerrainChunk::regenerate(TerrainGenerator& gen, int lodIndex, int cells)
{
    auto it = lodMap.find(lodIndex);
    if (it != lodMap.end()) {
        // Delete old mesh
        if (it->second.mesh) {
            delete it->second.mesh;
            it->second.mesh = nullptr;
        }
        
        // Generate new data
        it->second.data = gen.generateChunk(chunkX, chunkZ, cells, worldScale);
        computeBounds(it->second.data, it->second.minBounds, it->second.maxBounds);
        it->second.mesh = new Mesh(it->second.data);
        lodReady[lodIndex] = true;
    } else {
        // Create new LOD
        lodMap[lodIndex] = buildLod(gen, cells);
        lodReady[lodIndex] = true;
    }
}

std::vector<float> TerrainChunk::exportHeights() const
{
    std::vector<float> heights;
    
    // Prefer highest LOD (index 0)
    const LodMeshInfo* info = getLodInfo(0);
    if (!info && !lodMap.empty()) {
        info = &lodMap.begin()->second;
    }
    if (!info) return heights;

    heights.reserve(info->data.vertices.size());
    for (const auto &v : info->data.vertices) {
        heights.push_back(v.position.y);
    }
    return heights;
}

void TerrainChunk::draw(int lodIndex, bool wireframe) const
{
    if (wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    auto it = lodMap.find(lodIndex);
    if (it == lodMap.end()) {
        // Fallback: try to find ANY available LOD
        for (int fallbackLod = 0; fallbackLod < 3; ++fallbackLod) {
            it = lodMap.find(fallbackLod);
            if (it != lodMap.end() && it->second.mesh) {
                break;
            }
        }
        
        if (it == lodMap.end()) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            return;  // No LOD available
        }
    }

    const LodMeshInfo &info = it->second;
    if (info.mesh) {
        info.mesh->draw();
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void TerrainChunk::computeBounds(const MeshData& data, glm::vec3& minBounds, glm::vec3& maxBounds)
{
    if (data.vertices.empty()) {
        minBounds = maxBounds = glm::vec3(0.0f);
        return;
    }

    minBounds = glm::vec3(std::numeric_limits<float>::max());
    maxBounds = glm::vec3(std::numeric_limits<float>::lowest());

    for (const auto &v : data.vertices) {
        minBounds.x = std::min(minBounds.x, v.position.x);
        minBounds.y = std::min(minBounds.y, v.position.y);
        minBounds.z = std::min(minBounds.z, v.position.z);

        maxBounds.x = std::max(maxBounds.x, v.position.x);
        maxBounds.y = std::max(maxBounds.y, v.position.y);
        maxBounds.z = std::max(maxBounds.z, v.position.z);
    }
}

void TerrainChunk::setLodMesh(MeshData data, int lodIndex){
    if(lodIndex < 0 || lodIndex > 2) return;
    
    // Clean up existing mesh if any
    auto it = lodMap.find(lodIndex);
    if (it != lodMap.end() && it->second.mesh) {
        delete it->second.mesh;
    }
    
    LodMeshInfo info;
    info.data = std::move(data);
    computeBounds(info.data, info.minBounds, info.maxBounds);
    info.mesh = new Mesh(info.data);
    
    lodMap[lodIndex] = std::move(info);
    lodReady[lodIndex] = true;
}

glm::vec3 TerrainChunk::getMin(int lod) const {
    auto it = lodMap.find(lod);
    if (it != lodMap.end())
        return it->second.minBounds;
    return glm::vec3(0.0f);
}

glm::vec3 TerrainChunk::getMax(int lod) const {
    auto it = lodMap.find(lod);
    if (it != lodMap.end())
        return it->second.maxBounds;
    return glm::vec3(0.0f);
}