#ifndef TERRAIN_GENERATOR_H
#define TERRAIN_GENERATOR_H

#include "MeshData.h"
#include <random>


#define HIGH_LOD_CELLS 65
class TerrainGenerator{
public:
	struct Params{
		float baseFrequency = 0.01f;
		int octaves = 4;
		float persistence = 0.5f;
		float lacunarity = 2.0f;
		float heightScale = 20.0f;
		unsigned int seed = 1337;
	};

	TerrainGenerator() = default;
	explicit TerrainGenerator(const Params& p);

	MeshData generateChunk(int chunkX, int chunkZ, int cellPerSide, float worldScale);

	float getHeightAt(float worldX, float worldZ) const;

	void setParams(const Params& p);
	const Params& getParams() const;

private:
	Params params;
	std::mt19937 rng;
	std::uniform_real_distribution<float> dist;

	float fractalPerlin(float x, float z) const;
};

#endif