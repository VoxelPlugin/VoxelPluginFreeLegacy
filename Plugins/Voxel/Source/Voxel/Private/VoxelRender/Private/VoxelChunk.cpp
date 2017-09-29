// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelChunk.h"
#include "ProceduralMeshComponent.h"
#include "VoxelThread.h"
#include "Misc/IQueuedWork.h"
#include "AI/Navigation/NavigationSystem.h"
#include "VoxelRender.h"
#include "Engine.h"
#include "Camera/PlayerCameraManager.h"
#include "InstancedStaticMesh.h"
#include "Kismet/KismetMathLibrary.h"
#include "ChunkOctree.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

DECLARE_CYCLE_STAT(TEXT("VoxelChunk ~ SetProcMeshSection"), STAT_SetProcMeshSection, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelChunk ~ Update"), STAT_Update, STATGROUP_Voxel);

// Sets default values
AVoxelChunk::AVoxelChunk() : MeshTask(nullptr), Render(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;

	// Create primary mesh
	PrimaryMesh = CreateDefaultSubobject<UProceduralMeshComponent>(FName("PrimaryMesh"));
	PrimaryMesh->bCastShadowAsTwoSided = true;
	PrimaryMesh->bUseAsyncCooking = true;
	PrimaryMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	PrimaryMesh->Mobility = EComponentMobility::Movable;
	RootComponent = PrimaryMesh;

	ChunkHasHigherRes.SetNumZeroed(6);
}

void AVoxelChunk::Init(TWeakPtr<ChunkOctree> NewOctree)
{
	check(NewOctree.IsValid());
	CurrentOctree = NewOctree.Pin();
	Render = CurrentOctree->Render;

	FIntVector NewPosition = CurrentOctree->GetMinimalCornerPosition();

#if WITH_EDITOR
	FString Name = FString::FromInt(NewPosition.X) + ", " + FString::FromInt(NewPosition.Y) + ", " + FString::FromInt(NewPosition.Z);
	this->SetActorLabel(Name);
#endif

	this->SetActorRelativeLocation((FVector)NewPosition);
	this->SetActorRelativeRotation(FRotator::ZeroRotator);
	this->SetActorRelativeScale3D(FVector::OneVector);

	// Needed because octree is only partially builded when Init is called
	Render->AddTransitionCheck(this);
}

bool AVoxelChunk::Update(bool bAsync)
{
	SCOPE_CYCLE_COUNTER(STAT_Update);

	// Update ChunkHasHigherRes
	if (CurrentOctree->Depth != 0)
	{
		for (int i = 0; i < 6; i++)
		{
			TransitionDirection Direction = (TransitionDirection)i;
			TWeakPtr<ChunkOctree> Chunk = CurrentOctree->GetAdjacentChunk(Direction);
			if (Chunk.IsValid())
			{
				ChunkHasHigherRes[i] = Chunk.Pin()->Depth < CurrentOctree->Depth;
			}
			else
			{
				ChunkHasHigherRes[i] = false;
			}
		}
	}

	if (bAsync)
	{
		if (!MeshTask)
		{
			MeshTask = new MeshBuilderAsyncTask(
				CurrentOctree->Depth,
				Render->World->Data,
				CurrentOctree->GetMinimalCornerPosition(),
				ChunkHasHigherRes,
				CurrentOctree->Depth != 0,
				this
			);
			Render->MeshThreadPool->AddQueuedWork(MeshTask);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (MeshTask)
		{
			Render->MeshThreadPool->RetractQueuedWork(MeshTask);
			delete MeshTask;
			MeshTask = nullptr;
		}

		{
			VoxelPolygonizer* Polygonizer = new VoxelPolygonizer(
				CurrentOctree->Depth,
				Render->World->Data,
				CurrentOctree->GetMinimalCornerPosition(),
				ChunkHasHigherRes
			);
			Polygonizer->CreateSection(Section, CurrentOctree->Depth != 0);
			delete Polygonizer;
		}

		ApplyNewSection();

		return true;
	}
}

void AVoxelChunk::CheckTransitions()
{
	for (int i = 0; i < 6; i++)
	{
		TransitionDirection Direction = (TransitionDirection)i;
		TWeakPtr<ChunkOctree> Chunk = CurrentOctree->GetAdjacentChunk(Direction);
		if (Chunk.IsValid())
		{
			TSharedPtr<ChunkOctree> ChunkPtr = Chunk.Pin();

			bool bThisHasHigherRes = ChunkPtr->Depth > CurrentOctree->Depth;

			check(ChunkPtr->GetVoxelChunk());
			if (bThisHasHigherRes != ChunkPtr->GetVoxelChunk()->HasChunkHigherRes(InvertTransitionDirection(Direction)))
			{
				Render->UpdateChunk(Chunk, true);
			}
		}
	}
}

void AVoxelChunk::Unload()
{
	DeleteTasks();

	// Needed because octree is only partially updated when Unload is called
	Render->AddTransitionCheck(this);

	GetWorld()->GetTimerManager().SetTimer(DeleteTimer, this, &AVoxelChunk::Delete, Render->World->DeletionDelay, false);
}

void AVoxelChunk::Delete()
{
	// In case delete is called directly
	DeleteTasks();

	// Reset mesh & position & clear lines
	PrimaryMesh->SetProcMeshSection(0, FProcMeshSection());

#if WITH_EDITOR
	SetActorLabel("InactiveChunk");
#endif // WITH_EDITOR

	// Delete foliage
	for (auto FoliageComponent : FoliageComponents)
	{
		FoliageComponent->DestroyComponent();
	}
	FoliageComponents.Empty();


	// Add to pool
	Render->SetChunkAsInactive(this);


	// Reset variables
	Render = nullptr;
	CurrentOctree.Reset();
}

void AVoxelChunk::OnMeshComplete()
{
	SCOPE_CYCLE_COUNTER(STAT_SetProcMeshSection);

	Section = MeshTask->GetSectionCopy();
	delete MeshTask;
	MeshTask = nullptr;

	ApplyNewSection();
}

void AVoxelChunk::ApplyNewSection()
{
	// TODO
	//if (CurrentOctree->Depth <= Render->World->MaxGrassDepth)

	Render->AddFoliageUpdate(this);

	PrimaryMesh->SetProcMeshSection(0, Section);

	UNavigationSystem::UpdateComponentInNavOctree(*PrimaryMesh);
}

void AVoxelChunk::SetMaterial(UMaterialInterface* Material)
{
	PrimaryMesh->SetMaterial(0, Material);
}

bool AVoxelChunk::HasChunkHigherRes(TransitionDirection Direction)
{
	return CurrentOctree->Depth != 0 && ChunkHasHigherRes[Direction];
}

bool AVoxelChunk::UpdateFoliage()
{
	if (FoliageTasks.Num() == 0)
	{
		for (int Index = 0; Index < Render->World->GrassTypes.Num(); Index++)
		{
			auto GrassType = Render->World->GrassTypes[Index];
			for (auto GrassVariety : GrassType->GrassVarieties)
			{
				auto FoliageTask = new FoliageBuilderAsyncTask(
					Section,
					GrassVariety,
					Index,
					GetTransform(),
					Render->World->GetVoxelSize(),
					CurrentOctree->GetMinimalCornerPosition(),
					10,
					this
				);

				Render->FoliageThreadPool->AddQueuedWork(FoliageTask);
				FoliageTasks.Add(FoliageTask);
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}


void AVoxelChunk::OnFoliageComplete()
{
	CompletedFoliageTaskCount++;
	if (CompletedFoliageTaskCount == FoliageTasks.Num())
	{
		OnAllFoliageComplete();
	}
}

void AVoxelChunk::OnAllFoliageComplete()
{
	for (int i = 0; i < FoliageComponents.Num(); i++)
	{
		FoliageComponents[i]->DestroyComponent();
	}
	FoliageComponents.Empty();

	for (auto FoliageTask : FoliageTasks)
	{
		if (FoliageTask->InstanceBuffer.NumInstances())
		{
			FGrassVariety GrassVariety = FoliageTask->GrassVariety;

			int32 FolSeed = FCrc::StrCrc32((GrassVariety.GrassMesh->GetName() + GetName()).GetCharArray().GetData());
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
				HierarchicalInstancedStaticMeshComponent->InitPerInstanceRenderData(&FoliageTask->InstanceBuffer);
			}
			else
			{
				HierarchicalInstancedStaticMeshComponent->PerInstanceRenderData->UpdateFromPreallocatedData(HierarchicalInstancedStaticMeshComponent, FoliageTask->InstanceBuffer);
			}

			HierarchicalInstancedStaticMeshComponent->AcceptPrebuiltTree(FoliageTask->ClusterTree, FoliageTask->OutOcclusionLayerNum);

			//HierarchicalInstancedStaticMeshComponent->RecreateRenderState_Concurrent();
			HierarchicalInstancedStaticMeshComponent->MarkRenderStateDirty();
		}

		delete FoliageTask;
	}
	FoliageTasks.Empty();
	CompletedFoliageTaskCount = 0;
}

void AVoxelChunk::DeleteTasks()
{
	if (MeshTask)
	{
		Render->MeshThreadPool->RetractQueuedWork(MeshTask);
		delete MeshTask;
		MeshTask = nullptr;
	}
	for (auto FoliageTask : FoliageTasks)
	{
		Render->FoliageThreadPool->RetractQueuedWork(FoliageTask);
		delete FoliageTask;
	}
	FoliageTasks.Empty();
}
