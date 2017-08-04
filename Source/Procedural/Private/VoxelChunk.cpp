// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "VoxelData.h"
#include "DrawDebugHelpers.h"
#include "ProceduralMeshComponent.h"
#include "VoxelTransitionChunk.h"
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
	Task->EnsureCompletion();
	delete Task;
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


	XMinChunk = GetWorld()->SpawnActor<AVoxelTransitionChunk>(FVector::ZeroVector, FRotator::ZeroRotator);
	XMinChunk->Init(World, this, Position, Depth, XMin);
	XMinChunk->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	XMinChunk->SetActorLabel("XMin");
	XMinChunk->SetActorRelativeLocation(FVector::ZeroVector);
	XMinChunk->SetActorRelativeRotation(FRotator::ZeroRotator);
	XMinChunk->SetActorRelativeScale3D(FVector::OneVector);

	XMaxChunk = GetWorld()->SpawnActor<AVoxelTransitionChunk>(FVector::ZeroVector, FRotator::ZeroRotator);
	XMaxChunk->Init(World, this, Position, Depth, XMax);
	XMaxChunk->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	XMaxChunk->SetActorLabel("XMax");
	XMaxChunk->SetActorRelativeLocation(FVector::ZeroVector);
	XMaxChunk->SetActorRelativeRotation(FRotator::ZeroRotator);
	XMaxChunk->SetActorRelativeScale3D(FVector::OneVector);

	YMinChunk = GetWorld()->SpawnActor<AVoxelTransitionChunk>(FVector::ZeroVector, FRotator::ZeroRotator);
	YMinChunk->Init(World, this, Position, Depth, YMin);
	YMinChunk->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	YMinChunk->SetActorLabel("YMin");
	YMinChunk->SetActorRelativeLocation(FVector::ZeroVector);
	YMinChunk->SetActorRelativeRotation(FRotator::ZeroRotator);
	YMinChunk->SetActorRelativeScale3D(FVector::OneVector);

	YMaxChunk = GetWorld()->SpawnActor<AVoxelTransitionChunk>(FVector::ZeroVector, FRotator::ZeroRotator);
	YMaxChunk->Init(World, this, Position, Depth, YMax);
	YMaxChunk->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	YMaxChunk->SetActorLabel("YMax");
	YMaxChunk->SetActorRelativeLocation(FVector::ZeroVector);
	YMaxChunk->SetActorRelativeRotation(FRotator::ZeroRotator);
	YMaxChunk->SetActorRelativeScale3D(FVector::OneVector);

	ZMinChunk = GetWorld()->SpawnActor<AVoxelTransitionChunk>(FVector::ZeroVector, FRotator::ZeroRotator);
	ZMinChunk->Init(World, this, Position, Depth, ZMin);
	ZMinChunk->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	ZMinChunk->SetActorLabel("ZMin");
	ZMinChunk->SetActorRelativeLocation(FVector::ZeroVector);
	ZMinChunk->SetActorRelativeRotation(FRotator::ZeroRotator);
	ZMinChunk->SetActorRelativeScale3D(FVector::OneVector);

	ZMaxChunk = GetWorld()->SpawnActor<AVoxelTransitionChunk>(FVector::ZeroVector, FRotator::ZeroRotator);
	ZMaxChunk->Init(World, this, Position, Depth, ZMax);
	ZMaxChunk->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
	ZMaxChunk->SetActorLabel("ZMax");
	ZMaxChunk->SetActorRelativeLocation(FVector::ZeroVector);
	ZMaxChunk->SetActorRelativeRotation(FRotator::ZeroRotator);
	ZMaxChunk->SetActorRelativeScale3D(FVector::OneVector);
}

void AVoxelChunk::Update()
{
	if (Task == nullptr || Task->IsDone())
	{
		int Width = 16 << Depth;
		XMinChunkHasHigherRes = HasChunkHigherRes(-Width, 0, 0);
		XMaxChunkHasHigherRes = HasChunkHigherRes(Width, 0, 0);
		YMinChunkHasHigherRes = HasChunkHigherRes(0, -Width, 0);
		YMaxChunkHasHigherRes = HasChunkHigherRes(0, Width, 0);
		ZMinChunkHasHigherRes = HasChunkHigherRes(0, 0, -Width);
		ZMaxChunkHasHigherRes = HasChunkHigherRes(0, 0, Width);

		if (Task != nullptr)
		{
			// Should not be necessary
			Task->EnsureCompletion();
			delete Task;
		}

		Task = new FAsyncTask<VoxelThread>(this);
		Task->StartBackgroundTask(World->ThreadPool);
		bNeedSectionUpdate = true;
	}
}

void AVoxelChunk::Unload()
{
	if (this->IsValidLowLevel() && !this->IsPendingKill())
	{
		if (!this->XMinChunk->IsPendingKill())
		{
			this->XMinChunk->Destroy();
		}
		if (!this->XMaxChunk->IsPendingKill())
		{
			this->XMaxChunk->Destroy();
		}
		if (!this->YMinChunk->IsPendingKill())
		{
			this->YMinChunk->Destroy();
		}
		if (!this->YMaxChunk->IsPendingKill())
		{
			this->YMaxChunk->Destroy();
		}
		if (!this->ZMinChunk->IsPendingKill())
		{
			this->ZMinChunk->Destroy();
		}
		if (!this->ZMaxChunk->IsPendingKill())
		{
			this->ZMaxChunk->Destroy();
		}
		this->Destroy();
	}
}

void AVoxelChunk::SaveVertex(int x, int y, int z, short edgeIndex, int index)
{
	auto NewCache = NewCacheIs1 ? Cache1 : Cache2;
	NewCache[1 + x][1 + y][edgeIndex] = index;
}

int AVoxelChunk::LoadVertex(int x, int y, int z, short direction, short edgeIndex)
{
	auto NewCache = NewCacheIs1 ? Cache1 : Cache2;
	auto OldCache = NewCacheIs1 ? Cache2 : Cache1;

	bool xIsDifferent = direction & 0x01;
	bool yIsDifferent = direction & 0x02;
	bool zIsDifferent = direction & 0x04;
	check(0 <= 1 + x - xIsDifferent && 1 + x - xIsDifferent < 18);
	check(0 <= 1 + y - yIsDifferent && 1 + y - yIsDifferent < 18);
	check(0 <= edgeIndex && edgeIndex < 4);
	return (zIsDifferent ? OldCache : NewCache)[1 + x - xIsDifferent][1 + y - yIsDifferent][edgeIndex];
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

FVector AVoxelChunk::GetTranslated(FVector V, FVector N, VertexProperties P)
{
	// If an adjacent block is rendered at the same resolution, return primary position
	if ((P.IsNearXMin && !XMinChunkHasHigherRes) || (P.IsNearXMax && !XMaxChunkHasHigherRes) ||
		(P.IsNearYMin && !YMinChunkHasHigherRes) || (P.IsNearYMax && !YMaxChunkHasHigherRes) ||
		(P.IsNearZMin && !ZMinChunkHasHigherRes) || (P.IsNearZMax && !ZMaxChunkHasHigherRes))
	{
		return V;
	}


	double DeltaX = 0;
	double DeltaY = 0;
	double DeltaZ = 0;

	double TwoPowerK = 1 << Depth;
	double w = TwoPowerK / 4;

	if ((P.IsNearXMin && XMinChunkHasHigherRes) || (P.IsNearXMax && XMaxChunkHasHigherRes))
	{
		if (V.X < TwoPowerK)
		{
			DeltaX = (1 - (double)V.X / TwoPowerK) * w;
		}
		else if (V.X > TwoPowerK * (16 - 1))
		{
			DeltaX = (16 - 1 - (double)V.X / TwoPowerK) * w;
		}
	}
	if ((P.IsNearYMin && YMinChunkHasHigherRes) || (P.IsNearYMax && YMaxChunkHasHigherRes))
	{
		if (V.Y < TwoPowerK)
		{
			DeltaY = (1 - (double)V.Y / TwoPowerK) * w;
		}
		else if (V.Y > TwoPowerK * (16 - 1))
		{
			DeltaY = (16 - 1 - (double)V.Y / TwoPowerK) * w;
		}
	}
	if ((P.IsNearZMin && ZMinChunkHasHigherRes) || (P.IsNearZMax && ZMaxChunkHasHigherRes))
	{
		if (V.Z < TwoPowerK)
		{
			DeltaZ = (1 - (double)V.Z / TwoPowerK) * w;
		}
		else if (V.Z > TwoPowerK * (16 - 1))
		{
			DeltaZ = (16 - 1 - (double)V.Z / TwoPowerK) * w;
		}
	}

	FVector Q = FVector(
		(1 - N.X * N.X) * DeltaX - N.X * N.Y * DeltaY - N.X * N.Z * DeltaZ,
		-N.X * N.Y * DeltaX + (1 - N.Y * N.Y) * DeltaY - N.Y * N.Z * DeltaZ,
		-N.X * N.Z * DeltaX - N.Y * N.Z * DeltaY + (1 - N.Z * N.Z) * DeltaZ);
	return V + Q;
}

bool AVoxelChunk::IsNormalOnly(FVector vertex)
{
	int Step = 1 << Depth;
	return vertex.X < 0 || vertex.Y < 0 || vertex.Z < 0 || vertex.X > 16 * Step || vertex.Y > 16 * Step || vertex.Z > 16 * Step;
}
