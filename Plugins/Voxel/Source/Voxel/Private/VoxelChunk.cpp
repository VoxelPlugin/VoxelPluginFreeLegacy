// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelChunk.h"
#include "ProceduralMeshComponent.h"
#include "VoxelThread.h"
#include "Misc/IQueuedWork.h"
#include "AI/Navigation/NavigationSystem.h"
#include "DrawDebugHelpers.h"
#include "Engine.h"
#include "Camera/PlayerCameraManager.h"
#include "InstancedStaticMesh.h"
#include "Kismet/KismetMathLibrary.h"

DECLARE_CYCLE_STAT(TEXT("VoxelChunk ~ SetProcMeshSection"), STAT_SetProcMeshSection, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelChunk ~ Update"), STAT_Update, STATGROUP_Voxel);

// Sets default values
AVoxelChunk::AVoxelChunk() : bNeedSectionUpdate(false), RenderTask(nullptr), bNeedDeletion(false), bAdjacentChunksNeedUpdate(false), World(nullptr), bIsUsed(false)
{
	PrimaryActorTick.bCanEverTick = true;

	// Create primary mesh
	PrimaryMesh = CreateDefaultSubobject<UProceduralMeshComponent>(FName("PrimaryMesh"));
	PrimaryMesh->bCastShadowAsTwoSided = true;
	PrimaryMesh->bUseAsyncCooking = true;
	PrimaryMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	PrimaryMesh->Mobility = EComponentMobility::Movable;
	RootComponent = PrimaryMesh;

	DebugLineBatch = CreateDefaultSubobject<ULineBatchComponent>(FName("LineBatch"));

	ChunkHasHigherRes.SetNum(6);
}

void AVoxelChunk::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsUsed)
	{
		if (!World)
		{
			Destroy(this);
			return;
		}

		if (bNeedSectionUpdate && RenderTask && RenderTask->IsDone())
		{
			bNeedSectionUpdate = false;
			UpdateSection();
		}


		if (FoliageTask && FoliageTask->IsDone())
		{
			FoliageComplete();
			/*if (Depth == 0)
			{
				FoliageTask = new FAsyncTask<FoliageBuilderAsyncTask>(Section, World->GrassType, GetTransform());
				FoliageTask->StartBackgroundTask();
			}*/
		}

		if (bAdjacentChunksNeedUpdate && World->GetRebuildBorders())
		{
			bAdjacentChunksNeedUpdate = false;
			for (int i = 0; i < 6; i++)
			{
				AVoxelChunk* Chunk = GetChunk((TransitionDirection)i);
				if (Chunk)
				{
					Chunk->BasicUpdate();
				}
			}
		}

		if (bNeedDeletion)
		{
			TimeUntilDeletion -= DeltaTime;
			if (TimeUntilDeletion < 0)
			{
				Delete();
			}
		}

		if (bUpdate)
		{
			bUpdate = false;
			Update(false);
		}
	}
}

#if WITH_EDITOR
bool AVoxelChunk::ShouldTickIfViewportsOnly() const
{
	return true;
}
#endif //WITH_EDITOR

void AVoxelChunk::Init(FIntVector NewPosition, int NewDepth, AVoxelWorld* NewWorld)
{
	check(NewWorld);

	Position = NewPosition;
	Depth = NewDepth;
	World = NewWorld;

	FString Name = FString::FromInt(NewPosition.X) + ", " + FString::FromInt(NewPosition.Y) + ", " + FString::FromInt(NewPosition.Z);
	FVector RelativeLocation = (FVector)NewPosition;

	this->AttachToActor(NewWorld, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
#if WITH_EDITOR
	this->SetActorLabel(Name);
#endif
	this->SetActorRelativeLocation(RelativeLocation);
	this->SetActorRelativeRotation(FRotator::ZeroRotator);
	this->SetActorRelativeScale3D(FVector::OneVector);

	// Set material
	PrimaryMesh->SetMaterial(0, NewWorld->VoxelMaterial);

	// Force world update before this
	AddTickPrerequisiteActor(World);

	// Update adjacent
	bAdjacentChunksNeedUpdate = true;

	// Set as used
	bIsUsed = true;

	// Debug
	if (World->bDrawChunksBorders)
	{
		int w = Width();

		TArray<FBatchedLine> Lines;
		TArray<FIntVector> Starts = { FIntVector(0, 0, 0), FIntVector(w, 0, 0), FIntVector(w, w, 0), FIntVector(0, w, 0),
									  FIntVector(0, 0, 0), FIntVector(w, 0, 0), FIntVector(0, w, 0), FIntVector(w, w, 0),
									  FIntVector(0, 0, w), FIntVector(w, 0, w), FIntVector(w, w, w), FIntVector(0, w, w) };

		TArray<FIntVector> Ends = { FIntVector(w, 0, 0), FIntVector(w, w, 0), FIntVector(0, w, 0), FIntVector(0, 0, 0),
									FIntVector(0, 0, w), FIntVector(w, 0, w), FIntVector(0, w, w), FIntVector(w, w, w),
									FIntVector(w, 0, w), FIntVector(w, w, w), FIntVector(0, w, w), FIntVector(0, 0, w) };

		for (int i = 0; i < Starts.Num(); i++)
		{
			FVector Start = World->GetTransform().TransformPosition((FVector)(Position + Starts[i]));
			FVector End = World->GetTransform().TransformPosition((FVector)(Position + Ends[i]));
			Lines.Add(FBatchedLine(Start, End, FColor::Red, -1, Depth * Depth * Depth * Depth + 10, 0));
		}
		DebugLineBatch->DrawLines(Lines);
	}
}

void AVoxelChunk::Update(bool bAsync)
{
	SCOPE_CYCLE_COUNTER(STAT_Update);

	check(bIsUsed);

	// Make sure we've ticked
	Tick(0);

	if (!RenderTask)
	{
		// Update ChunkHasHigherRes
		for (int i = 0; i < 6; i++)
		{
			if (Depth == 0)
			{
				ChunkHasHigherRes[i] = false;
			}
			else
			{
				auto Direction = (TransitionDirection)i;
				ChunkHasHigherRes[i] = GetChunk(Direction) && (GetChunk(Direction)->GetDepth() < Depth);
			}
		}

		RenderTask = new VoxelThread(this);
		if (bAsync)
		{
			World->GetThreadPool()->AddQueuedWork(RenderTask);
			bNeedSectionUpdate = true;
		}
		else
		{
			RenderTask->DoThreadedWork();
			// Update immediately to avoid holes
			UpdateSection();
		}
	}
}

void AVoxelChunk::BasicUpdate()
{
	for (int i = 0; i < 6; i++)
	{
		TransitionDirection Direction = (TransitionDirection)i;
		bool bHigherRes = GetChunk(Direction) && (GetChunk(Direction)->GetDepth() < Depth);
		if (ChunkHasHigherRes[i] != bHigherRes)
		{
			if (!RenderTask)
			{
				Update(true);
				return;
			}
			else
			{
				ChunkHasHigherRes[i] = bHigherRes;
			}
		}
	}
}

void AVoxelChunk::Unload()
{
	bNeedDeletion = true;
	TimeUntilDeletion = World->GetDeletionDelay();
	bAdjacentChunksNeedUpdate = true;

	// If task is queued, remove it
	if (RenderTask)
	{
		World->GetThreadPool()->RetractQueuedWork(RenderTask);
		delete RenderTask;
		RenderTask = nullptr;
	}
}

void AVoxelChunk::Delete()
{	// If task if queued, remove it
	if (RenderTask)
	{
		World->GetThreadPool()->RetractQueuedWork(RenderTask);
		delete RenderTask;
		RenderTask = nullptr;
	}
	if (FoliageTask)
	{
		FoliageTask->EnsureCompletion();
		delete FoliageTask;
		FoliageTask = nullptr;
	}

	// Reset mesh & position & clear lines
	PrimaryMesh->SetProcMeshSection(0, FProcMeshSection());
	DebugLineBatch->Flush();
	SetActorLocation(FVector(0, 0, 0));
#if WITH_EDITOR
	SetActorLabel("PoolChunk");
#endif // WITH_EDITOR

	// Delete foliage
	for (int i = 0; i < FoliageComponents.Num(); i++)
	{
		FoliageComponents[i]->DestroyComponent();
	}
	FoliageComponents.Empty();


	// Add to pool
	check(World);
	World->AddChunkToPool(this);


	// Reset variables
	bNeedSectionUpdate = false;
	bNeedDeletion = false;
	bAdjacentChunksNeedUpdate = false;
	World = nullptr;
	bIsUsed = false;
}

void AVoxelChunk::UpdateSection()
{
	SCOPE_CYCLE_COUNTER(STAT_SetProcMeshSection);
	check(RenderTask->IsDone());

	Section = RenderTask->GetSection();

	if (!FoliageTask && Depth == 0)
	{
		FoliageTask = new FAsyncTask<FoliageBuilderAsyncTask>(Section, World->GrassType, GetTransform());
		FoliageTask->StartBackgroundTask();
	}

	PrimaryMesh->SetProcMeshSection(0, RenderTask->GetSection());
	delete RenderTask;
	RenderTask = nullptr;

	UNavigationSystem::UpdateComponentInNavOctree(*PrimaryMesh);
}

int AVoxelChunk::GetDepth() const
{
	return Depth;
}

int AVoxelChunk::Width() const
{
	return 16 << Depth;
}

float AVoxelChunk::GetValue(int x, int y, int z) const
{
	return World->GetValue(Position + FIntVector(x, y, z));
}

FColor AVoxelChunk::GetColor(int x, int y, int z) const
{
	return World->GetColor(Position + FIntVector(x, y, z));
}

AVoxelChunk* AVoxelChunk::GetChunk(TransitionDirection Direction) const
{
	TArray<FIntVector> L = { FIntVector(-Width(), 0, 0) , FIntVector(Width(), 0, 0) , FIntVector(0, -Width(), 0), FIntVector(0, Width(), 0) , FIntVector(0, 0, -Width()) , FIntVector(0, 0, Width()) };

	FIntVector P = Position + FIntVector(Width() / 2, Width() / 2, Width() / 2) + L[Direction];
	if (World->IsInWorld(P))
	{
		return World->GetChunkAt(P);
	}
	else
	{
		return nullptr;
	}
}


void FoliageBuilderAsyncTask::DoWork()
{
	// TODO: setnum
	TArray<FMatrix> InstanceTransforms;

	auto GrassVariety = GrassType->GrassVarieties[0];

	for (int Index = 0; Index < Section.ProcIndexBuffer.Num(); Index += 3)
	{
		FVector A = Section.ProcVertexBuffer[Section.ProcIndexBuffer[Index]].Position;
		FVector B = Section.ProcVertexBuffer[Section.ProcIndexBuffer[Index + 1]].Position;
		FVector C = Section.ProcVertexBuffer[Section.ProcIndexBuffer[Index + 2]].Position;

		FVector N = (Section.ProcVertexBuffer[Section.ProcIndexBuffer[Index]].Normal +
					 Section.ProcVertexBuffer[Section.ProcIndexBuffer[Index + 1]].Normal +
					 Section.ProcVertexBuffer[Section.ProcIndexBuffer[Index + 2]].Normal) / 3;

		FVector X = B - A;
		FVector Y = C - A;

		const float SizeX = X.Size();
		const float SizeY = Y.Size();

		X.Normalize();
		Y.Normalize();

		for (int i = 0; i < GrassVariety.GrassDensity; i++)
		{
			float CoordX = FMath::RandRange(0.f, SizeY);
			float CoordY = FMath::RandRange(0.f, SizeX);

			if (SizeY - CoordX * SizeY / SizeX < CoordY)
			{
				CoordX = SizeX - CoordX;
				CoordY = SizeY - CoordY;
			}

			FVector P = A + X * CoordX + Y * CoordY;

			FVector Scale(1.0f);

			switch (GrassVariety.Scaling)
			{
			case EGrassScaling::Uniform:
				Scale.X = GrassVariety.ScaleX.Interpolate(FMath::RandRange(0.f, 1.f));
				Scale.Y = Scale.X;
				Scale.Z = Scale.X;
				break;
			case EGrassScaling::Free:
				Scale.X = GrassVariety.ScaleX.Interpolate(FMath::RandRange(0.f, 1.f));
				Scale.Y = GrassVariety.ScaleY.Interpolate(FMath::RandRange(0.f, 1.f));
				Scale.Z = GrassVariety.ScaleZ.Interpolate(FMath::RandRange(0.f, 1.f));
				break;
			case EGrassScaling::LockXY:
				Scale.X = GrassVariety.ScaleX.Interpolate(FMath::RandRange(0.f, 1.f));
				Scale.Y = Scale.X;
				Scale.Z = GrassVariety.ScaleZ.Interpolate(FMath::RandRange(0.f, 1.f));
				break;
			default:
				check(0);
			}

			InstanceTransforms.Add(FTransform(UKismetMathLibrary::MakeRotFromZ(N), ChunkTransform.GetScale3D() * P, Scale).ToMatrixWithScale());
		}
	}


	if (InstanceTransforms.Num())
	{
		InstanceBuffer.AllocateInstances(InstanceTransforms.Num(), true);
		for (int32 InstanceIndex = 0; InstanceIndex < InstanceTransforms.Num(); InstanceIndex++)
		{
			InstanceBuffer.SetInstance(InstanceIndex, InstanceTransforms[InstanceIndex], 0);
		}

		TArray<int32> SortedInstances;
		TArray<int32> InstanceReorderTable;
		UHierarchicalInstancedStaticMeshComponent::BuildTreeAnyThread(InstanceTransforms, GrassType->GrassVarieties[0].GrassMesh->GetBounds().GetBox(), ClusterTree, SortedInstances, InstanceReorderTable, OutOcclusionLayerNum, /*DesiredInstancesPerLeaf*/1);

		//SORT
		// in-place sort the instances
		const uint32 InstanceStreamSize = InstanceBuffer.GetStride();
		FInstanceStream32 SwapBuffer;
		check(sizeof(SwapBuffer) >= InstanceStreamSize);

		for (int32 FirstUnfixedIndex = 0; FirstUnfixedIndex < InstanceTransforms.Num(); FirstUnfixedIndex++)
		{
			int32 LoadFrom = SortedInstances[FirstUnfixedIndex];
			if (LoadFrom != FirstUnfixedIndex)
			{
				check(LoadFrom > FirstUnfixedIndex);
				FMemory::Memcpy(&SwapBuffer, InstanceBuffer.GetInstanceWriteAddress(FirstUnfixedIndex), InstanceStreamSize);
				FMemory::Memcpy(InstanceBuffer.GetInstanceWriteAddress(FirstUnfixedIndex), InstanceBuffer.GetInstanceWriteAddress(LoadFrom), InstanceStreamSize);
				FMemory::Memcpy(InstanceBuffer.GetInstanceWriteAddress(LoadFrom), &SwapBuffer, InstanceStreamSize);

				int32 SwapGoesTo = InstanceReorderTable[FirstUnfixedIndex];
				check(SwapGoesTo > FirstUnfixedIndex);
				check(SortedInstances[SwapGoesTo] == FirstUnfixedIndex);
				SortedInstances[SwapGoesTo] = LoadFrom;
				InstanceReorderTable[LoadFrom] = SwapGoesTo;

				InstanceReorderTable[FirstUnfixedIndex] = FirstUnfixedIndex;
				SortedInstances[FirstUnfixedIndex] = FirstUnfixedIndex;
			}
		}
	}
}

void AVoxelChunk::FoliageComplete()
{
	check(FoliageTask && FoliageTask->IsDone());


	for (int i = 0; i < FoliageComponents.Num(); i++)
	{
		FoliageComponents[i]->DestroyComponent();
	}
	FoliageComponents.Empty();

	for (int GrassVarietyIndex = 0; GrassVarietyIndex < World->GrassType->GrassVarieties.Num(); GrassVarietyIndex++)
	{
		if (FoliageTask->GetTask().InstanceBuffer.NumInstances())
		{
			auto GrassVariety = World->GrassType->GrassVarieties[GrassVarietyIndex];

			int32 FolSeed = FCrc::StrCrc32((World->GrassType->GetName() + GetName() + FString::Printf(TEXT("%d %d %d"), 0, 0, GrassVarietyIndex)).GetCharArray().GetData());
			if (FolSeed == 0)
			{
				FolSeed++;
			}

			//Create component
			UHierarchicalInstancedStaticMeshComponent* HierarchicalInstancedStaticMeshComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, NAME_None, RF_Transient);

			HierarchicalInstancedStaticMeshComponent->OnComponentCreated();
			HierarchicalInstancedStaticMeshComponent->RegisterComponent();
			if (HierarchicalInstancedStaticMeshComponent->bWantsInitializeComponent) HierarchicalInstancedStaticMeshComponent->InitializeComponent();

			HierarchicalInstancedStaticMeshComponent->Mobility = EComponentMobility::Movable;
			HierarchicalInstancedStaticMeshComponent->bCastStaticShadow = false;

			HierarchicalInstancedStaticMeshComponent->SetStaticMesh(GrassVariety.GrassMesh);
			HierarchicalInstancedStaticMeshComponent->MinLOD = GrassVariety.MinLOD;
			HierarchicalInstancedStaticMeshComponent->bSelectable = false;
			HierarchicalInstancedStaticMeshComponent->bHasPerInstanceHitProxies = false;
			HierarchicalInstancedStaticMeshComponent->bReceivesDecals = GrassVariety.bReceivesDecals;
			static FName NoCollision(TEXT("NoCollision"));
			HierarchicalInstancedStaticMeshComponent->SetCollisionProfileName(NoCollision);
			HierarchicalInstancedStaticMeshComponent->bDisableCollision = true;
			HierarchicalInstancedStaticMeshComponent->SetCanEverAffectNavigation(false);
			HierarchicalInstancedStaticMeshComponent->InstancingRandomSeed = FolSeed;
			HierarchicalInstancedStaticMeshComponent->LightingChannels = GrassVariety.LightingChannels;

			HierarchicalInstancedStaticMeshComponent->InstanceStartCullDistance = GrassVariety.StartCullDistance;
			HierarchicalInstancedStaticMeshComponent->InstanceEndCullDistance = GrassVariety.EndCullDistance;

			HierarchicalInstancedStaticMeshComponent->bAffectDistanceFieldLighting = false;

			HierarchicalInstancedStaticMeshComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			FTransform DesiredTransform = GetRootComponent()->GetComponentTransform();
			DesiredTransform.RemoveScaling();
			HierarchicalInstancedStaticMeshComponent->SetWorldTransform(DesiredTransform);

			FoliageComponents.Add(HierarchicalInstancedStaticMeshComponent);

			if (!HierarchicalInstancedStaticMeshComponent->PerInstanceRenderData.IsValid())
			{
				HierarchicalInstancedStaticMeshComponent->InitPerInstanceRenderData(&FoliageTask->GetTask().InstanceBuffer);
			}
			else
			{
				HierarchicalInstancedStaticMeshComponent->PerInstanceRenderData->UpdateFromPreallocatedData(HierarchicalInstancedStaticMeshComponent, FoliageTask->GetTask().InstanceBuffer);
			}

			HierarchicalInstancedStaticMeshComponent->AcceptPrebuiltTree(FoliageTask->GetTask().ClusterTree, FoliageTask->GetTask().OutOcclusionLayerNum);

			//HierarchicalInstancedStaticMeshComponent->RecreateRenderState_Concurrent();
			HierarchicalInstancedStaticMeshComponent->MarkRenderStateDirty();
		}
	}

	delete FoliageTask;
	FoliageTask = nullptr;
}
