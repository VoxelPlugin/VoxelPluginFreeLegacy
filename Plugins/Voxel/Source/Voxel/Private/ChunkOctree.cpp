#include "VoxelPrivatePCH.h"
#include "ChunkOctree.h"
#include "VoxelChunk.h"
#include "Engine/World.h"

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

int ChunkOctree::Width()
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

	const float DistanceToCamera = ((World->GetTransform().TransformPosition((FVector)Position) - CameraPosition).Size()) / World->GetActorScale3D().Size() - World->GetHighResolutionDistanceOffset();
	const float Quality = World->GetQuality();

	const float MinDistance = Width() * Quality;
	const float MaxDistance = Width() * Quality * 2;


	if (DistanceToCamera < 0)
	{
		// Always at highest resolution
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
				check(Childs[i].IsValid());
				Childs[i]->CreateTree(World, CameraPosition);
			}
		}
		return;
	}


	if (MinDistance < DistanceToCamera && DistanceToCamera < MaxDistance)
	{
		// Depth OK
		if (bHasChilds)
		{
			// Update childs
			for (int i = 0; i < 8; i++)
			{
				check(Childs[i].IsValid());
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
		// Depth too high
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
				check(Childs[i].IsValid());
				Childs[i]->CreateTree(World, CameraPosition);
			}
		}
	}
	else // DistanceToCamera > MaxDistance
	{
		// Depth too low
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

void ChunkOctree::Update(bool bAsync)
{
	check(bHasChunk == (VoxelChunk != nullptr));
	check(bHasChilds == (Childs.Num() == 8));
	check(bHasChilds ^ bHasChunk);

	if (bHasChunk)
	{
		VoxelChunk->Update(bAsync);
	}
	else if (bHasChilds)
	{
		for (int i = 0; i < 8; i++)
		{
			check(Childs[i].IsValid());
			Childs[i]->Update(bAsync);
		}
	}
	else
	{
		UE_LOG(VoxelLog, Fatal, TEXT("Cannot Update: !bHasChunk && !bHasChilds"));
	}
}

TWeakPtr<ChunkOctree> ChunkOctree::GetChunk(FIntVector PointPosition)
{
	check(bHasChunk == (VoxelChunk != nullptr));
	check(bHasChilds == (Childs.Num() == 8));
	check(bHasChilds ^ bHasChunk);

	if (bHasChunk)
	{
		return AsShared();
	}
	else if (bHasChilds)
	{
		// Ex: Child 6 -> position (0, 1, 1) -> 0b011 == 6
		TSharedPtr<ChunkOctree> Child = Childs[(PointPosition.X >= Position.X ? 1 : 0) + (PointPosition.Y >= Position.Y ? 2 : 0) + (PointPosition.Z >= Position.Z ? 4 : 0)];
		check(Child.IsValid());
		return Child->GetChunk(PointPosition);
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
	check(!bHasChilds);
	check(World);

	if (!bHasChunk)
	{
		VoxelChunk = World->GetWorld()->SpawnActor<AVoxelChunk>(FVector::ZeroVector, FRotator::ZeroRotator);
		VoxelChunk->Init(Position - FIntVector(1, 1, 1) * Width() / 2, Depth, World);
		World->QueueUpdate(AsShared());
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
	check(!bHasChunk);
	check(Depth != 0);

	if (!bHasChilds)
	{
		int d = Width() / 4;
		Childs.Add(TSharedPtr<ChunkOctree>(new ChunkOctree(Position + FIntVector(-d, -d, -d), Depth - 1)));
		Childs.Add(TSharedPtr<ChunkOctree>(new ChunkOctree(Position + FIntVector(+d, -d, -d), Depth - 1)));
		Childs.Add(TSharedPtr<ChunkOctree>(new ChunkOctree(Position + FIntVector(-d, +d, -d), Depth - 1)));
		Childs.Add(TSharedPtr<ChunkOctree>(new ChunkOctree(Position + FIntVector(+d, +d, -d), Depth - 1)));
		Childs.Add(TSharedPtr<ChunkOctree>(new ChunkOctree(Position + FIntVector(-d, -d, +d), Depth - 1)));
		Childs.Add(TSharedPtr<ChunkOctree>(new ChunkOctree(Position + FIntVector(+d, -d, +d), Depth - 1)));
		Childs.Add(TSharedPtr<ChunkOctree>(new ChunkOctree(Position + FIntVector(-d, +d, +d), Depth - 1)));
		Childs.Add(TSharedPtr<ChunkOctree>(new ChunkOctree(Position + FIntVector(+d, +d, +d), Depth - 1)));
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
		for (TSharedPtr<ChunkOctree> Child : Childs)
		{
			Child->Delete();
			Child.Reset();
		}
		Childs.Reset();
		bHasChilds = false;
	}
	else
	{
		UE_LOG(VoxelLog, Fatal, TEXT("Error: Cannot delete childs: !bHasChilds"));
	}
}