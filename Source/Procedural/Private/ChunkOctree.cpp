#include "ChunkOctree.h"
#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "EngineGlobals.h"
#include "Engine.h"

ChunkOctree::ChunkOctree(FIntVector position, int depth) : Position(position), Depth(depth)
{

}

bool ChunkOctree::operator==(ChunkOctree* other)
{
	return Position == other->Position && Depth == other->Depth;
}



bool ChunkOctree::CreateChilds()
{
	if (!IsLeaf())
	{
		int d = GetWidth() / 4;
		Childs[0] = new ChunkOctree(Position + FIntVector(-d, -d, -d), Depth - 1);
		Childs[1] = new ChunkOctree(Position + FIntVector(+d, -d, -d), Depth - 1);
		Childs[2] = new ChunkOctree(Position + FIntVector(-d, +d, -d), Depth - 1);
		Childs[3] = new ChunkOctree(Position + FIntVector(+d, +d, -d), Depth - 1);
		Childs[4] = new ChunkOctree(Position + FIntVector(-d, -d, +d), Depth - 1);
		Childs[5] = new ChunkOctree(Position + FIntVector(+d, -d, +d), Depth - 1);
		Childs[6] = new ChunkOctree(Position + FIntVector(-d, +d, +d), Depth - 1);
		Childs[7] = new ChunkOctree(Position + FIntVector(+d, +d, +d), Depth - 1);
		return true;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Error: Cannot create childs: IsLeaf"));
		return false;
	}
}

int ChunkOctree::GetWidth()
{
	return 16 << Depth;
}

void ChunkOctree::CreateTree(AVoxelWorld* world, FVector cameraPosition)
{
	float distanceToCamera = (world->GetTransform().TransformPosition(FVector(Position.X, Position.Y, Position.Z)) - cameraPosition).Size();

	if (distanceToCamera > GetWidth() * world->GetActorScale3D().Size() * 10000 || Depth == 0)
	{
		VoxelChunk = world->GetWorld()->SpawnActor<AVoxelChunk>(FVector::ZeroVector, FRotator::ZeroRotator);
		int w = GetWidth() / 2;
		VoxelChunk->Init(Position - FIntVector(1, 1, 1) * GetWidth() / 2, Depth, world);
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

ChunkOctree* ChunkOctree::GetLeaf(FIntVector position)
{
	if (IsLeaf())
	{
		return this;
	}
	else
	{
		// Ex: Child 6 -> position (0, 1, 1) -> 0b011 == 6
		int d = GetWidth() / 2;
		return Childs[(position.X >= Position.X ? 1 : 0) + (position.Y >= Position.Y ? 2 : 0) + (position.Z >= Position.Z ? 4 : 0)]->GetLeaf(position);
	}
}


bool ChunkOctree::IsLeaf()
{
	return VoxelChunk != nullptr;
}