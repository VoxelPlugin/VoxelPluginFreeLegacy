#include "ChunkOctree.h"
#include "VoxelChunk.h"
#include "VoxelCollisionChunk.h"
#include "VoxelWorld.h"
#include "EngineGlobals.h"
#include "Engine.h"

ChunkOctree::ChunkOctree(FIntVector position, int depth) : Position(position), Depth(depth), bHasChilds(false), bHasChunk(false)
{

}

ChunkOctree::~ChunkOctree()
{
	if (bHasChunk)
	{
		Unload();
	}
	if (bHasChilds)
	{
		for (int i = 0; i < 8; i++)
		{
			delete Childs[i];
		}
	}
}


bool ChunkOctree::operator==(ChunkOctree* other)
{
	return Position == other->Position && Depth == other->Depth;
}



int ChunkOctree::GetWidth()
{
	return 16 << Depth;
}

void ChunkOctree::CreateTree(AVoxelWorld* world, FVector cameraPosition)
{
	float distanceToCamera = (world->GetTransform().TransformPosition(FVector(Position.X, Position.Y, Position.Z)) - cameraPosition).Size();

	if (distanceToCamera > GetWidth() * world->GetActorScale3D().Size() * 2 || Depth == 0)
	{
		if (bHasChilds)
		{
			DeleteChilds();
		}
		if (!bHasChunk)
		{
			Load(world);
		}
	}
	else
	{
		if (bHasChunk)
		{
			Unload();
		}
		if (!bHasChilds)
		{
			CreateChilds();
		}
		for (int i = 0; i < 8; i++)
		{
			Childs[i]->CreateTree(world, cameraPosition);
		}
	}
}

void ChunkOctree::Update()
{
	if (bHasChunk)
	{
		VoxelChunk->Update();
	}
	else if (bHasChilds)
	{
		for (int i = 0; i < 8; i++)
		{
			Childs[i]->Update();
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Error: Cannot update: !bHasChunk && !bHasChilds"));
	}
}

ChunkOctree* ChunkOctree::GetChunk(FIntVector position)
{
	if (bHasChunk)
	{
		return this;
	}
	else
	{
		// Ex: Child 6 -> position (0, 1, 1) -> 0b011 == 6
		int d = GetWidth() / 2;
		return Childs[(position.X >= Position.X ? 1 : 0) + (position.Y >= Position.Y ? 2 : 0) + (position.Z >= Position.Z ? 4 : 0)]->GetChunk(position);
	}
}




void ChunkOctree::Load(AVoxelWorld* world)
{
	if (!bHasChunk)
	{
		VoxelChunk = world->GetWorld()->SpawnActor<AVoxelChunk>(FVector::ZeroVector, FRotator::ZeroRotator);
		int w = GetWidth() / 2;
		VoxelChunk->Init(Position - FIntVector(1, 1, 1) * GetWidth() / 2, Depth, world);
		VoxelChunk->Update();
		bHasChunk = true;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Error: Cannot load: bHasChunk"));
	}
}

void ChunkOctree::Unload()
{
	if (bHasChunk)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Start chunk destruction"));

		VoxelChunk->Unload();

		VoxelChunk = nullptr;
		bHasChunk = false;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Error: Cannot unload: !bHasChunk"));
	}
}

void ChunkOctree::CreateChilds()
{
	if (!bHasChilds)
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
		bHasChilds = true;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Error: Cannot create childs: bHasChilds"));
	}
}

void ChunkOctree::DeleteChilds()
{
	if (bHasChilds)
	{
		for (int i = 0; i < 8; i++)
		{
			delete Childs[i];
		}
		bHasChilds = false;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Error: Cannot delete childs: !bHasChilds"));
	}
}