// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "VoxelData.h"
#include "DrawDebugHelpers.h"
#include "ProceduralMeshComponent.h"
#include "VoxelThread.h"

// Sets default values
AVoxelChunk::AVoxelChunk() : bNeedSectionUpdate(false), Task(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;

	// Create primary mesh
	PrimaryMesh = CreateDefaultSubobject<UProceduralMeshComponent>(FName("PrimaryMesh"));
	RootComponent = PrimaryMesh;
}

AVoxelChunk::~AVoxelChunk()
{

}

// Called when the game starts or when spawned
void AVoxelChunk::BeginPlay()
{
	Super::BeginPlay();
}

void AVoxelChunk::Tick(float DeltaTime)
{
	if (bNeedSectionUpdate && Task != nullptr && Task->IsDone())
	{
		bNeedSectionUpdate = false;
		PrimaryMesh->SetProcMeshSection(0, Section);
		delete Task;
		Task = nullptr;
	}
}

void AVoxelChunk::Init(FIntVector position, int depth, AVoxelWorld* world)
{
	check(world);

	Position = position;
	Depth = depth;
	World = world;

	FString name = FString::FromInt(position.X) + ", " + FString::FromInt(position.Y) + ", " + FString::FromInt(position.Z);
	FVector relativeLocation = (FVector)position;

	this->AttachToActor(world, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	this->SetActorLabel(name);
	this->SetActorRelativeLocation(relativeLocation);
	this->SetActorRelativeRotation(FRotator::ZeroRotator);
	this->SetActorRelativeScale3D(FVector::OneVector);

	// Configure primary mesh
	PrimaryMesh->SetMaterial(0, world->VoxelMaterial);
	PrimaryMesh->bCastShadowAsTwoSided = true;
	PrimaryMesh->bUseAsyncCooking = true;
	PrimaryMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
}

void AVoxelChunk::Update()
{
	// Make sure we've ticked
	Tick(0);

	if (Task == nullptr || Task->IsDone())
	{
		if (Task != nullptr)
		{
			// Should not be necessary
			Task->EnsureCompletion();
			delete Task;
		}

		Task = new FAsyncTask<VoxelThread>(this);
		Task->StartBackgroundTask(World->ThreadPool);
		//Task->StartSynchronousTask();
		bNeedSectionUpdate = true;
	}
}

void AVoxelChunk::Unload()
{
	/*if (Task != nullptr)
	{
		Task->EnsureCompletion();
		delete Task;
	}*/
	if (this->IsValidLowLevel() && !this->IsPendingKill())
	{
		this->Destroy();
	}
}

int AVoxelChunk::GetDepth()
{
	return Depth;
}

signed char AVoxelChunk::GetValue(int x, int y, int z)
{
	return World->GetValue(Position + FIntVector(x, y, z));
}

FColor AVoxelChunk::GetColor(int x, int y, int z)
{
	return World->GetColor(Position + FIntVector(x, y, z));
}

bool AVoxelChunk::HasChunkHigherRes(int x, int y, int z)
{
	int Width = 16 << Depth;
	FIntVector P = Position + FIntVector(Width / 2, Width / 2, Width / 2) + FIntVector(x, y, z);
	if (World->IsInWorld(P))
	{
		return Depth > World->GetDepthAt(P);
	}
	else
	{
		return false;
	}
}
