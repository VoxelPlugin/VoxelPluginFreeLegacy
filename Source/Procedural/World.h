#pragma once
#include <vector>

class Chunk;

class World
{
public:
	World(int size);
	~World();

	const int Size;
	const int ChunkSize;
	float Level;
	std::vector<Chunk*> Chunks;

	void Randomize();
	void Sphere();

	float GetValue(int x, int y, int z);

private:
	std::vector<float> values;
};