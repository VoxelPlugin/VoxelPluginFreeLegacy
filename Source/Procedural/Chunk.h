#pragma once

#include <utility>
#include <forward_list>
#include <vector>
#include <unordered_map>
#include "CoreMinimal.h"

namespace std
{
	template <>
	struct hash < pair<int, int >>
	{
		size_t operator()(pair<int, int> const& p) const
		{
			auto h1 = std::hash<int>{}(p.first);
			auto h2 = std::hash<int>{}(p.second);

			// Mainly for demonstration purposes, i.e. works but is overly simple
			// In the real world, use sth. like boost.hash_combine
			return h1 ^ h2;
		}
	};
};

class World;


class Chunk
{
public:
	Chunk(World* world, int x, int y, int z);
	static const int Size = 16;
	const int X;
	const int Y;
	const int Z;
	World* const world;

	TArray<FVector> Vertices;
	TArray<FVector> Normals;
	TArray<int> Triangles;

	void Process();

private:

	std::unordered_map<std::pair<int, int>, int> interpolationDict;
	std::vector<FVector> vertices;
	std::vector<FVector> normals;
	std::forward_list<int> triangles;
	std::vector<bool> keepVertices;

	int verticesCount;
	int trianglesCount;


	int Interpolate(int verticeIndex1, int verticeIndex2);
	void Polygonise(int grid[8]);

	float GetLevel();
	float GetValue(int verticeIndex);
	int GetIndex(int x, int y, int z);
	std::pair<int, int> CreatePair(int a, int b);
};