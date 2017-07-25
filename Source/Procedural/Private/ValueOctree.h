#pragma once
#include <cmath>
#include "CoreMinimal.h"
#include "Engine.h"
#include <vector>

class ValueOctree
{
public:
	ValueOctree(int x, int y, int z, int depth);
	~ValueOctree();

	const int X;
	const int Y;
	const int Z;
	const int Depth;



	inline int GetWidth();

	void CreateTree(FVector cameraPosition);

	ValueOctree* GetLeaf(int x, int y, int z);

	bool IsLeaf();

	bool IsDirty();

	signed char GetValue(int x, int y, int z);

	void SetValue(int x, int y, int z, signed char value);

	bool IsInChunk(int x, int y, int z);

protected:
	/*
	bottom      top
	-----> y
	| 0 | 2    4 | 6
	v 1 | 3    5 | 7
	x

	*/
	ValueOctree* Childs[8];

	std::vector<signed char> Values;
	bool bIsDirty;
	bool bIsLeaf;

	bool CreateChilds();
};