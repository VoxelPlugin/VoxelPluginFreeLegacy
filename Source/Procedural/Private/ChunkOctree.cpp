#include "ChunkOctree.h"
#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "EngineGlobals.h"
#include "Engine.h"

DEFINE_LOG_CATEGORY(ChunkOctreeLog)

ChunkOctree::ChunkOctree(FIntVector position, int depth) : Position(position), Depth(depth), bHasChilds(false), bHasChunk(false), VoxelChunk(nullptr)
{
	check(Depth >= 0);
}

ChunkOctree::~ChunkOctree()
{

}


bool ChunkOctree::operator==(const ChunkOctree& other)
{
	return Position == other.Position && Depth == other.Depth;
}



void ChunkOctree::Delete()
{
	if (bHasChunk)
	{
		Unload();
	}
	if (bHasChilds)
	{
		DeleteChilds();
	}
}

int ChunkOctree::GetWidth()
{
	return 16 << Depth;
}

void ChunkOctree::CreateTree(AVoxelWorld* world, FVector cameraPosition)
{
	check(world);
	check(bHasChunk == (VoxelChunk != nullptr));
	check(bHasChilds == Childs[0].IsValid());
	check(!(bHasChilds && bHasChunk));

	if (Depth == 0)
	{
		// Always create
		if (!bHasChunk)
		{
			Load(world);
		}
		return;
	}

	const float DistanceToCamera = ((world->GetTransform().TransformPosition((FVector)Position) - cameraPosition).Size()) / world->GetActorScale3D().Size();
	const float Quality = world->GetQuality();

	const float MinDistance = GetWidth() * Quality;
	const float MaxDistance = GetWidth() * Quality * 2;

	if (MinDistance < DistanceToCamera && DistanceToCamera < MaxDistance)
	{
		if (bHasChilds)
		{
			// Update childs
			for (int i = 0; i < 8; i++)
			{
				Childs[i]->CreateTree(world, cameraPosition);
			}
		}
		else if (!bHasChunk)
		{
			// Not created, create
			Load(world);
		}
	}
	else if (DistanceToCamera < MinDistance)
	{
		if (bHasChunk)
		{
			Unload();
		}
		if (!bHasChilds)
		{
			CreateChilds();
		}
		if (bHasChilds)
		{
			// Update childs
			for (int i = 0; i < 8; i++)
			{
				Childs[i]->CreateTree(world, cameraPosition);
			}
		}
	}
	else
	{
		// DistanceToCamera > MaxDistance

		if (bHasChilds)
		{
			// Too far, delete childs
			DeleteChilds();
		}
		if (!bHasChunk)
		{
			// Not created, create
			Load(world);
		}
	}
	check(bHasChilds ^ bHasChunk);
}

void ChunkOctree::Update(bool async)
{
	check(bHasChunk == (VoxelChunk != nullptr));
	check(bHasChilds == Childs[0].IsValid());

	if (bHasChunk)
	{
		VoxelChunk->Update(async);
	}
	else if (bHasChilds)
	{
		for (int i = 0; i < 8; i++)
		{
			Childs[i]->Update(async);
		}
	}
	else
	{
		UE_LOG(ChunkOctreeLog, Fatal, TEXT("Cannot Update: !bHasChunk && !bHasChilds"));
	}
}

TWeakPtr<ChunkOctree> ChunkOctree::GetChunk(FIntVector position)
{
	check(bHasChunk == (VoxelChunk != nullptr));
	check(bHasChilds == Childs[0].IsValid());

	if (bHasChunk)
	{
		return AsShared();
	}
	else if (bHasChilds)
	{
		// Ex: Child 6 -> position (0, 1, 1) -> 0b011 == 6
		int d = GetWidth() / 2;
		TSharedPtr<ChunkOctree> child = Childs[(position.X >= Position.X ? 1 : 0) + (position.Y >= Position.Y ? 2 : 0) + (position.Z >= Position.Z ? 4 : 0)];
		check(child.IsValid());
		return child->GetChunk(position);
	}
	else
	{
		UE_LOG(ChunkOctreeLog, Fatal, TEXT("Cannot GetChunk: !bHasChunk && !bHasChilds"));
		return TWeakPtr<ChunkOctree>(nullptr);
	}
}




void ChunkOctree::Load(AVoxelWorld* world)
{
	check(bHasChunk == (VoxelChunk != nullptr));
	check(world);

	if (!bHasChunk)
	{
		VoxelChunk = world->GetWorld()->SpawnActor<AVoxelChunk>(FVector::ZeroVector, FRotator::ZeroRotator);
		int w = GetWidth() / 2;
		VoxelChunk->Init(Position - FIntVector(1, 1, 1) * GetWidth() / 2, Depth, world);
		world->ScheduleUpdate(AsShared());
		bHasChunk = true;
	}
	else
	{
		UE_LOG(ChunkOctreeLog, Error, TEXT("Cannot Load: bHasChunk"));
	}
}

void ChunkOctree::Unload()
{
	check(bHasChunk == (VoxelChunk != nullptr));

	if (bHasChunk)
	{
		VoxelChunk->Unload();

		VoxelChunk = nullptr;
		bHasChunk = false;
	}
	else
	{
		UE_LOG(ChunkOctreeLog, Error, TEXT("Cannot Unload: !bHasChunk"));
	}
}

void ChunkOctree::CreateChilds()
{
	check(bHasChilds == Childs[0].IsValid());
	check(Depth != 0)

		if (!bHasChilds)
		{
			int d = GetWidth() / 4;
			Childs[0] = MakeShareable(new ChunkOctree(Position + FIntVector(-d, -d, -d), Depth - 1));
			Childs[1] = MakeShareable(new ChunkOctree(Position + FIntVector(+d, -d, -d), Depth - 1));
			Childs[2] = MakeShareable(new ChunkOctree(Position + FIntVector(-d, +d, -d), Depth - 1));
			Childs[3] = MakeShareable(new ChunkOctree(Position + FIntVector(+d, +d, -d), Depth - 1));
			Childs[4] = MakeShareable(new ChunkOctree(Position + FIntVector(-d, -d, +d), Depth - 1));
			Childs[5] = MakeShareable(new ChunkOctree(Position + FIntVector(+d, -d, +d), Depth - 1));
			Childs[6] = MakeShareable(new ChunkOctree(Position + FIntVector(-d, +d, +d), Depth - 1));
			Childs[7] = MakeShareable(new ChunkOctree(Position + FIntVector(+d, +d, +d), Depth - 1));
			bHasChilds = true;
		}
		else
		{
			UE_LOG(ChunkOctreeLog, Error, TEXT("Error: Cannot create childs: bHasChilds"));
		}
}

void ChunkOctree::DeleteChilds()
{
	check(bHasChilds == Childs[0].IsValid());

	if (bHasChilds)
	{
		for (int i = 0; i < 8; i++)
		{
			Childs[i]->Delete();
			Childs[i].Reset();
		}
		bHasChilds = false;
	}
	else
	{
		UE_LOG(ChunkOctreeLog, Error, TEXT("Error: Cannot delete childs: !bHasChilds"));
	}
}