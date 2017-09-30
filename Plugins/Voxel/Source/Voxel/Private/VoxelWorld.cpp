// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelData.h"
#include "VoxelRender.h"
#include "Components/CapsuleComponent.h"
#include "Engine.h"
#include <forward_list>
#include "FlatWorldGenerator.h"
#include "VoxelInvokerComponent.h"

DEFINE_LOG_CATEGORY(VoxelLog)

// Sets default values
AVoxelWorld::AVoxelWorld()
	: Depth(9)
	, DeletionDelay(0.1f)
	, bComputeTransitions(true)
	, bIsCreated(false)
	, FoliageFPS(15)
	, MeshFPS(120)
	, VoxelSize(100)
	, MeshThreadCount(4)
	, FoliageThreadCount(4)
{
	PrimaryActorTick.bCanEverTick = true;

	auto TouchCapsule = CreateDefaultSubobject<UCapsuleComponent>(FName("Capsule"));
	TouchCapsule->InitCapsuleSize(0.1f, 0.1f);
	TouchCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TouchCapsule->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = TouchCapsule;

	WorldGenerator = TSubclassOf<AVoxelWorldGenerator>(AFlatWorldGenerator::StaticClass());

	bReplicates = true;
}

AVoxelWorld::~AVoxelWorld()
{
	delete Data;
	delete Render;
}

void AVoxelWorld::BeginPlay()
{
	Super::BeginPlay();

	Load();
}

void AVoxelWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Render->Tick(DeltaTime);
}

float AVoxelWorld::GetValue(FIntVector Position) const
{
	if (IsInWorld(Position))
	{
		return Data->GetValue(Position.X, Position.Y, Position.Z);
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Get value: Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
		return 0;
	}
}

FVoxelMaterial AVoxelWorld::GetMaterial(FIntVector Position) const
{
	return Data->GetMaterial(Position.X, Position.Y, Position.Z);
}

void AVoxelWorld::SetValue(FIntVector Position, float Value)
{
	if (IsInWorld(Position))
	{
		Data->SetValue(Position.X, Position.Y, Position.Z, Value);
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Get material: Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
	}
}

void AVoxelWorld::SetMaterial(FIntVector Position, FVoxelMaterial Material)
{
	if (IsInWorld(Position))
	{
		Data->SetMaterial(Position.X, Position.Y, Position.Z, Material);
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Set material: Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
	}
}


FVoxelWorldSave AVoxelWorld::GetSave() const
{
	return Data->GetSave();
}

void AVoxelWorld::LoadFromSave(FVoxelWorldSave Save, bool bReset)
{
	if (Save.Depth == Depth)
	{
		std::forward_list<FIntVector> ModifiedPositions;
		Data->LoadFromSaveAndGetModifiedPositions(Save, ModifiedPositions, bReset);
		for (auto Position : ModifiedPositions)
		{
			UpdateChunksAtPosition(Position, true);
		}
		Render->ApplyUpdates();
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("Current Depth is %d while Save one is %d"), Depth, Save.Depth);
	}
}



FIntVector AVoxelWorld::GlobalToLocal(FVector Position) const
{
	FVector P = GetTransform().InverseTransformPosition(Position);
	return FIntVector(FMath::RoundToInt(P.X), FMath::RoundToInt(P.Y), FMath::RoundToInt(P.Z));
}

FVector AVoxelWorld::LocalToGlobal(FIntVector Position) const
{
	return GetTransform().TransformPosition((FVector)Position);
}

void AVoxelWorld::UpdateChunksAtPosition(FIntVector Position, bool bAsync)
{
	Render->UpdateChunksAtPosition(Position, bAsync);
}

void AVoxelWorld::UpdateAll(bool bAsync)
{
	Render->UpdateAll(bAsync);
}

void AVoxelWorld::AddInvoker(TWeakObjectPtr<UVoxelInvokerComponent> Invoker)
{
	Render->AddInvoker(Invoker);
}

void AVoxelWorld::Load()
{
	if (!bIsCreated)
	{
		UE_LOG(VoxelLog, Warning, TEXT("Loading world"));
		bIsCreated = true;

		InstancedWorldGenerator = GetWorld()->SpawnActor<AVoxelWorldGenerator>(WorldGenerator);

		if (InstancedWorldGenerator == nullptr)
		{
			UE_LOG(VoxelLog, Error, TEXT("Invalid world generator"));
			InstancedWorldGenerator = Cast<AVoxelWorldGenerator>(GetWorld()->SpawnActor(AFlatWorldGenerator::StaticClass()));
		}

		InstancedWorldGenerator->SetVoxelWorld(this);

		Data = new VoxelData(Depth, InstancedWorldGenerator);
		Render = new VoxelRender(this, MeshThreadCount, FoliageThreadCount);

		SetActorScale3D(FVector::OneVector * VoxelSize);
	}
}

void AVoxelWorld::Unload()
{
	if (bIsCreated)
	{
		delete Render;
		delete Data;
		bIsCreated = false;
	}
}

bool AVoxelWorld::IsCreated() const
{
	return bIsCreated;
}

int AVoxelWorld::GetDepthAt(FIntVector Position) const
{
	if (IsInWorld(Position))
	{
		return Render->GetDepthAt(Position);
	}
	else
	{
		UE_LOG(VoxelLog, Error, TEXT("GetDepthAt: Not in world: (%d, %d, %d)"), Position.X, Position.Y, Position.Z);
		return 0;
	}
}

bool AVoxelWorld::IsInWorld(FIntVector Position) const
{
	return Data->IsInWorld(Position.X, Position.Y, Position.Z);
}

float AVoxelWorld::GetVoxelSize()
{
	return VoxelSize;
}

int AVoxelWorld::Size() const
{
	return Data->Size();
}
