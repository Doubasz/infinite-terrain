#ifndef TERRAIN_CACHE_H
#define TERRAIN_CACHE_H

#include <string>
#include <vector>


namespace TerrainCache{
    std::string filename(int cx, int cz);
    bool loadHeightMap(int cx, int cz, int cellsPerSide, std::vector<float>& outHeights);
    bool saveHeightMap(int cx, int cz, int cellsPerSide, const std::vector<float>& heights);
};


#endif