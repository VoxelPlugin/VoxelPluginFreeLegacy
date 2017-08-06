#include "ChunkOctree.h"
#include "VoxelChunk.h"
#include "VoxelWorld.h"

ChunkOctree::ChunkOctree(FIntVector Position, int Depth) : Position(Position), Depth(Depth), bHasChilds(false), bHasChunk(false), VoxelChunk(nullptr)
{
	check(Depth >= 0);
}


bool ChunkOctree::operator==(const ChunkOctree& Other)
{
	return Position == Other.Position && Depth == Other.Depth;
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

void ChunkOctree::CreateTree(AVoxelWorld* World, FVector CameraPosition)
{
	check(World);
	check(bHasChunk == (VoxelChunk != nullptr));
	check(bHasChilds == (Childs.Num() == 8));
	check(!(bHasChilds && bHasChunk));

	if (Depth == 0)
	{
		// Always create
		if (!bHasChunk)
		{
			Load(World);
		}
		return;
	}

	const float DistanceToCamera = ((World->GetTransform().TransformPosition((FVector)Position) - CameraPosition).Size()) / World->GetActorScale3D().Size();
	const float Quality = World->GetQuality();

	const float MinDistance = GetWidth() * Quality;
	const float MaxDistance = GetWidth() * Quality * 2;

	if (MinDistance < DistanceToCamera && DistanceToCamera < MaxDistance)
	{
		if (bHasChilds)
		{
			// Update childs
			for (int i = 0; i < 8; i++)
			{
				Childs[i]->CreateTree(World, CameraPosition);
			}
		}
		else if (!bHasChunk)
		{
			// Not created, create
			Load(World);
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
				Childs[i]->CreateTree(World, CameraPosition);
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
			Load(World);
		}
	}
	check(bHasChilds ^ bHasChunk);
}

void ChunkOctree::Update(bool Async)
{
	check(bHasChunk == (VoxelChunk != nullptr));
	check(bHasChilds == (Childs.Num() == 8));

	if (bHasChunk)
	{
		VoxelChunk->Update(Async);
	}
	else if (bHasChilds)
	{
		for (int i = 0; i < 8; i++)
		{
			Childs[i]->Update(Async);
		}
	}
	else
	{
		UE_LOG(VoxelLog, Fatal, TEXT("Cannot Update: !bHasChunk && !bHasChilds"));
	}
}

TWeakPtr<ChunkOctree> ChunkOctree::GetChunk(FIntVector Position)
{
	check(bHasChunk == (VoxelChunk != nullptr));
	check(bHasChilds == (Childs.Num() == 8));

	if (bHasChunk)
	{
		return AsShared();
	}
	else if (bHasChilds)
	{
		// Ex: Child 6 -> position (0, 1, 1) -> 0b011 == 6
		TSharedPtr<ChunkOctree> Child = Childs[(Position.X >= Position.X ? 1 : 0) + (Position.Y >= Position.Y ? 2 : 0) + (Position.Z >= Position.Z ? 4 : 0)];
		check(Child.IsValid());
		return Child->GetChunk(Position);
	}
	else
	{
		UE_LOG(VoxelLog, Fatal, TEXT("Cannot GetChunk: !bHasChunk && !bHasChilds"));
		return TWeakPtr<ChunkOctree>(nullptr);
	}
}

AVoxelChunk* ChunkOctree::GetVoxelChunk()
{
	return VoxelChunk;
}




void ChunkOctree::Load(AVoxelWorld* World)
{
	check(bHasChunk == (VoxelChunk != nullptr));
	check(World);

	if (!bHasChunk)
	{
		VoxelChunk = World->GetWorld()->SpawnActor<AVoxelChunk>(FVector::ZeroVector, FRotator::ZeroRotator);
		int w = GetWidth() / 2;
		VoxelChunk->Init(Position - FIntVector(1, 1, 1) * GetWidth() / 2, Depth, World);
		World->ScheduleUpdate(AsShared());
		bHasChunk = true;
	}
	else
	{
		UE_LOG(VoxelLog, Fatal, TEXT("Cannot Load: bHasChunk"));
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
		UE_LOG(VoxelLog, Fatal, TEXT("Cannot Unload: !bHasChunk"));
	}
}

void ChunkOctree::CreateChilds()
{
	check(bHasChilds == (Childs.Num() == 8));
	check(Depth != 0);

	if (!bHasChilds)
	{
		int d = GetWidth() / 4;
		Childs.Add(TSharedRef<ChunkOctree>(new ChunkOctree(Position + FIntVector(-d, -d, -d), Depth - 1)));
		Childs.Add(TSharedRef<ChunkOctree>(new ChunkOctree(Position + FIntVector(+d, -d, -d), Depth - 1)));
		Childs.Add(TSharedRef<ChunkOctree>(new ChunkOctree(Position + FIntVector(-d, +d, -d), Depth - 1)));
		Childs.Add(TSharedRef<ChunkOctree>(new ChunkOctree(Position + FIntVector(+d, +d, -d), Depth - 1)));
		Childs.Add(TSharedRef<ChunkOctree>(new ChunkOctree(Position + FIntVector(-d, -d, +d), Depth - 1)));
		Childs.Add(TSharedRef<ChunkOctree>(new ChunkOctree(Position + FIntVector(+d, -d, +d), Depth - 1)));
		Childs.Add(TSharedRef<ChunkOctree>(new ChunkOctree(Position + FIntVector(-d, +d, +d), Depth - 1)));
		Childs.Add(TSharedRef<ChunkOctree>(new ChunkOctree(Position + FIntVector(+d, +d, +d), Depth - 1)));
		bHasChilds = true;
	}
	else
	{
		UE_LOG(VoxelLog, Fatal, TEXT("Error: Cannot create childs: bHasChilds"));
	}
}

void ChunkOctree::DeleteChilds()
{
	check(bHasChilds == (Childs.Num() == 8));

	if (bHasChilds)
	{
		for (auto Child : Childs)
		{
			Child->Delete();
		}
		Childs.Reset();
		bHasChilds = false;
	}
	else
	{
		UE_LOG(VoxelLog, Fatal, TEXT("Error: Cannot delete childs: !bHasChilds"));
	}
}