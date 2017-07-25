#include "ChunkOctree.h"
#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "EngineGlobals.h"
#include "Engine.h"

ChunkOctree::ChunkOctree(int x, int y, int z, int depth) : X(x), Y(y), Z(z), Depth(depth)
{

}

inline bool ChunkOctree::operator==(ChunkOctree* other)
{
	return X == other->X && Y == other->Y && Z == other->Z;
}



bool ChunkOctree::CreateChilds()
{
	if (!IsLeaf())
	{
		int d = GetWidth() / 2;
		Childs[0] = new ChunkOctree(X, Y, Z, Depth - 1);
		Childs[1] = new ChunkOctree(X + d, Y, Z, Depth - 1);
		Childs[2] = new ChunkOctree(X, Y + d, Z, Depth - 1);
		Childs[3] = new ChunkOctree(X + d, Y + d, Z, Depth - 1);
		Childs[4] = new ChunkOctree(X, Y, Z + d, Depth - 1);
		Childs[5] = new ChunkOctree(X + d, Y, Z + d, Depth - 1);
		Childs[6] = new ChunkOctree(X, Y + d, Z + d, Depth - 1);
		Childs[7] = new ChunkOctree(X + d, Y + d, Z + d, Depth - 1);
		return true;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Error: Cannot create childs: IsLeaf"));
		return false;
	}
}

inline int ChunkOctree::GetWidth()
{
	return 16 << Depth;
}

void ChunkOctree::CreateTree(AVoxelWorld* world, FVector cameraPosition)
{
	float distanceToCamera = (world->GetTransform().TransformPosition(FVector(X, Y, Z)) - cameraPosition).Size();

	if (distanceToCamera > GetWidth() * 10000 || Depth == 0)
	{
		VoxelChunk = world->GetWorld()->SpawnActor<AVoxelChunk>(FVector::ZeroVector, FRotator::ZeroRotator);
		VoxelChunk->Init(X, Y, Z, Depth, world);
	}
	else
	{
		if (CreateChilds())
		{
			for (int i = 0; i < 8; i++)
			{
				Childs[i]->CreateTree(world, cameraPosition);
			}
		}
	}
}

void ChunkOctree::Update()
{
	if (IsLeaf())
	{
		VoxelChunk->Update();
	}
	else
	{
		for (int i = 0; i < 8; i++)
		{
			Childs[i]->Update();
		}
	}
}

ChunkOctree* ChunkOctree::GetLeaf(int x, int y, int z)
{
	if (IsLeaf())
	{
		return this;
	}
	else
	{
		// Ex: Child 6 -> position (0, 1, 1) -> 0b011 == 6
		return Childs[((x >= X + GetWidth() / 2) ? 1 : 0) + ((y >= Y + GetWidth() / 2) ? 2 : 0) + ((z >= Z + GetWidth() / 2) ? 4 : 0)]->GetLeaf(x, y, z);
	}
}


bool ChunkOctree::IsLeaf()
{
	return VoxelChunk != nullptr;
}