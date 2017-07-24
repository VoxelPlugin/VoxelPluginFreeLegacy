#pragma once

#include <utility>
#include <forward_list>
#include <vector>
#include <unordered_map>
#include "CoreMinimal.h"

class World;


class Chunk
{
public:
	Chunk(World* world, int x, int y, int z);
	const int X;
	const int Y;
	const int Z;
	World* const world;

	TArray<FVector> Vertices;
	TArray<int> Triangles;

	void Process();

private:
	std::forward_list<FVector> vertices;
	std::forward_list<int> triangles;

	int cache1[16][16][4];
	int cache2[16][16][4];
	bool newCacheIs1;

	int verticesCount;
	int trianglesCount;


	void Polygonise(int x, int y, int z);
	char GetValue(int x, int y, int z);
};