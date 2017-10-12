// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelData.h"
#include "VoxelRender.h"
#include "Components/CapsuleComponent.h"
#include "Engine.h"
#include <forward_list>
#include "FlatWorldGenerator.h"
#include "VoxelInvokerComponent.h"
#include "VoxelModifier.h"
#include "VoxelWorldEditor.h"

DEFINE_LOG_CATEGORY(VoxelLog)

// Sets default values
AVoxelWorld::AVoxelWorld()
	: NewDepth(9)
	, DeletionDelay(0.1f)
	, bComputeTransitions(true)
	, bIsCreated(false)
	, FoliageFPS(15)
	, NewVoxelSize(100)
	, MeshThreadCount(4)
	, FoliageThreadCount(4)
	, Render(nullptr)
	, Data(nullptr)
	, InstancedWorldGenerator(nullptr)
	, VoxelWorldEditor(nullptr)
	, bComputeCollisions(false)
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

	if (!IsCreated())
	{
		CreateWorld();
	}

	bComputeCollisions = true;
}

void AVoxelWorld::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsCreated())
	{
		Render->Tick(DeltaTime);
	}
}

#if WITH_EDITOR
bool AVoxelWorld::ShouldTickIfViewportsOnly() const
{
	return true;
}

void AVoxelWorld::PostLoad()
{
	Super::PostLoad();

	if (GetWorld())
	{
		CreateInEditor();
	}
}

#endif

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


void AVoxelWorld::GetSave(FVoxelWorldSave& OutSave) const
{
	Data->GetSave(OutSave);
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
		UE_LOG(VoxelLog, Error, TEXT("LoadFromSave: Current Depth is %d while Save one is %d"), Depth, Save.Depth);
	}
}



void AVoxelWorld::UpdateVoxelModifiers()
{
	if (IsCreated())
	{
		DestroyWorld();
	}
	CreateWorld(false);

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		AVoxelModifier* Modifier = Cast<AVoxelModifier>(Actor);

		if (Modifier)
		{
			Modifier->ApplyToWorld(this);
		}
	}

	GetSave(WorldSave);

	DestroyWorld();

	CreateInEditor();
}

AVoxelWorldEditor* AVoxelWorld::GetVoxelWorldEditor()
{
	return VoxelWorldEditor;
}

FVoxelData* AVoxelWorld::GetData()
{
	return Data;
}

AVoxelWorldGenerator* AVoxelWorld::GetWorldGenerator()
{
	return InstancedWorldGenerator;
}

int32 AVoxelWorld::GetSeed()
{
	return Seed;
}

float AVoxelWorld::GetFoliageFPS()
{
	return FoliageFPS;
}

UMaterialInterface* AVoxelWorld::GetVoxelMaterial()
{
	return VoxelMaterial;
}

bool AVoxelWorld::GetComputeTransitions()
{
	return bComputeTransitions;
}

bool AVoxelWorld::GetComputeCollisions()
{
	return bComputeCollisions;
}

float AVoxelWorld::GetDeletionDelay()
{
	return DeletionDelay;
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

void AVoxelWorld::CreateWorld(bool bLoadFromSave)
{
	check(!IsCreated());

	UE_LOG(VoxelLog, Warning, TEXT("Loading world"));

	Depth = NewDepth;
	VoxelSize = NewVoxelSize;

	SetActorScale3D(FVector::OneVector * VoxelSize);

	check(!Data);
	check(!Render);

	if (!InstancedWorldGenerator || InstancedWorldGenerator->GetClass() != WorldGenerator->GetClass())
	{
		// Create generator

		if (InstancedWorldGenerator)
		{
			// Delete if created
			InstancedWorldGenerator->Destroy();
		}

		InstancedWorldGenerator = GetWorld()->SpawnActor<AVoxelWorldGenerator>(WorldGenerator);
		if (InstancedWorldGenerator == nullptr)
		{
			UE_LOG(VoxelLog, Error, TEXT("Invalid world generator"));
			InstancedWorldGenerator = Cast<AVoxelWorldGenerator>(GetWorld()->SpawnActor(AFlatWorldGenerator::StaticClass()));
		}
	}

	InstancedWorldGenerator->SetVoxelWorld(this);

	// Create Data
	Data = new FVoxelData(Depth, InstancedWorldGenerator);

	// Create Render
	Render = new FVoxelRender(this, this, Data, MeshThreadCount, FoliageThreadCount);

	// Load from save
	if (bLoadFromSave && WorldSave.Depth == Depth)
	{
		std::forward_list<FIntVector> ModifiedPositions;
		Data->LoadFromSaveAndGetModifiedPositions(WorldSave, ModifiedPositions, false);
	}

	bIsCreated = true;
}

void AVoxelWorld::DestroyWorld()
{
	check(IsCreated());

	UE_LOG(VoxelLog, Warning, TEXT("Unloading world"));

	check(Data);
	check(Render);
	delete Data;
	delete Render;
	Data = nullptr;
	Render = nullptr;

	bIsCreated = false;
}

void AVoxelWorld::CreateInEditor()
{
	// Create/Find VoxelWorldEditor
	VoxelWorldEditor = nullptr;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVoxelWorldEditor::StaticClass(), FoundActors);

	for (auto Actor : FoundActors)
	{
		auto VoxelWorldEditorActor = Cast<AVoxelWorldEditor>(Actor);
		if (VoxelWorldEditorActor)
		{
			VoxelWorldEditor = VoxelWorldEditorActor;
			break;
		}
	}
	if (!VoxelWorldEditor)
	{
		// else spawn
		VoxelWorldEditor = GetWorld()->SpawnActor<AVoxelWorldEditor>();
	}

	VoxelWorldEditor->Init(this);


	if (IsCreated())
	{
		DestroyWorld();
	}
	CreateWorld();

	bComputeCollisions = false;

	AddInvoker(VoxelWorldEditor->GetInvoker());

	UpdateAll(true);
}

void AVoxelWorld::DestroyInEditor()
{
	if (IsCreated())
	{
		Render->Delete();
		DestroyWorld();
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
