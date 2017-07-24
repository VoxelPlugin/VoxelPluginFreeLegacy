#pragma once
#include <vector>

class Chunk;

class World
{
public:
	World(int size);
	~World();

	const int Size;
	const int SizeInChunks;
	std::vector<Chunk*> Chunks;

	void Randomize();
	void Sphere();
	void Plane();

	char GetValue(int x, int y, int z);
	void Add(int x, int y, int z);
	void Remove(int x, int y, int z);
	bool IsInWorld(int x, int y, int z);

private:
	std::vector<signed char> values;
};