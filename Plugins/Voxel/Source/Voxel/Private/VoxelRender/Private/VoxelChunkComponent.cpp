// Copyright 2017 Phyronnaz

#include "VoxelPrivatePCH.h"
#include "VoxelChunkComponent.h"
#include "ProceduralMeshComponent.h"
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
#include "VoxelPolygonizer.h"

DECLARE_CYCLE_STAT(TEXT("VoxelChunk ~ SetProcMeshSection"), STAT_SetProcMeshSection, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("VoxelChunk ~ Update"), STAT_Update, STATGROUP_Voxel);

// Sets default values
UVoxelChunkComponent::UVoxelChunkComponent()
	: Render(nullptr)
	, MeshBuilder(nullptr)
	, Builder(nullptr)
	, CompletedFoliageTaskCount(0)
{
	bCastShadowAsTwoSided = true;
	bUseAsyncCooking = true;
	SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	Mobility = EComponentMobility::Movable;

	ChunkHasHigherRes.SetNumZeroed(6);
}

void UVoxelChunkComponent::Init(TWeakPtr<FChunkOctree> NewOctree)
{
	check(NewOctree.IsValid());
	CurrentOctree = NewOctree.Pin();
	Render = CurrentOctree->Render;

	FIntVector NewPosition = CurrentOctree->GetMinimalCornerPosition();

	this->SetRelativeLocationAndRotation(Render->World->LocalToGlobal(NewPosition), FRotator::ZeroRotator);
	this->SetRelativeScale3D(FVector::OneVector * Render->World->GetVoxelSize());

	// Needed because octree is only partially builded when Init is called
	Render->AddTransitionCheck(this);
}

bool UVoxelChunkComponent::Update(bool bAsync)
{
	SCOPE_CYCLE_COUNTER(STAT_Update);

	// Update ChunkHasHigherRes
	if (CurrentOctree->Depth != 0)
	{
		for (int i = 0; i < 6; i++)
		{
			TransitionDirection Direction = (TransitionDirection)i;
			TWeakPtr<FChunkOctree> Chunk = CurrentOctree->GetAdjacentChunk(Direction);
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
		if (!MeshBuilder)
		{
			CreateBuilder();
			MeshBuilder = new FAsyncTask<FAsyncPolygonizerTask>(Builder, this);
			MeshBuilder->StartBackgroundTask(Render->MeshThreadPool);

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (MeshBuilder)
		{
			MeshBuilder->EnsureCompletion();
			delete MeshBuilder;
			MeshBuilder = nullptr;
		}
		if (Builder)
		{
			delete Builder;
			Builder = nullptr;
		}

		CreateBuilder();
		Builder->CreateSection(Section);
		delete Builder;
		Builder = nullptr;

		ApplyNewMesh();

		return true;
	}
}

void UVoxelChunkComponent::CheckTransitions()
{
	if (Render->World->bComputeTransitions)
	{
		for (int i = 0; i < 6; i++)
		{
			TransitionDirection Direction = (TransitionDirection)i;
			TWeakPtr<FChunkOctree> Chunk = CurrentOctree->GetAdjacentChunk(Direction);
			if (Chunk.IsValid())
			{
				TSharedPtr<FChunkOctree> ChunkPtr = Chunk.Pin();

				bool bThisHasHigherRes = ChunkPtr->Depth > CurrentOctree->Depth;

				check(ChunkPtr->GetVoxelChunk());
				if (bThisHasHigherRes != ChunkPtr->GetVoxelChunk()->HasChunkHigherRes(InvertTransitionDirection(Direction)))
				{
					Render->UpdateChunk(Chunk, true);
				}
			}
		}
	}
}

void UVoxelChunkComponent::Unload()
{
	DeleteTasks();

	// Needed because octree is only partially updated when Unload is called
	Render->AddTransitionCheck(this);

	GetWorld()->GetTimerManager().SetTimer(DeleteTimer, this, &UVoxelChunkComponent::Delete, Render->World->DeletionDelay + KINDA_SMALL_NUMBER, false);

	// Cancel any update
	Render->RemoveFromQueues(this);
}

void UVoxelChunkComponent::Delete()
{
	// In case delete is called directly
	DeleteTasks();

	// Reset mesh & position & clear lines
	SetProcMeshSection(0, FProcMeshSection());

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

void UVoxelChunkComponent::OnMeshComplete(FProcMeshSection& InSection)
{
	SCOPE_CYCLE_COUNTER(STAT_SetProcMeshSection);

	Section = InSection;

	Render->AddApplyNewMesh(this);
}

void UVoxelChunkComponent::ApplyNewMesh()
{
	// TODO
	//if (CurrentOctree->Depth <= Render->World->MaxGrassDepth)

	if (MeshBuilder)
	{
		MeshBuilder->EnsureCompletion();
		delete MeshBuilder;
		MeshBuilder = nullptr;
	}
	if (Builder)
	{
		delete Builder;
		Builder = nullptr;
	}

	Render->AddFoliageUpdate(this);

	SetProcMeshSection(0, Section);

	UNavigationSystem::UpdateComponentInNavOctree(*this);
}

void UVoxelChunkComponent::SetVoxelMaterial(UMaterialInterface* Material)
{
	SetMaterial(0, Material);
}

bool UVoxelChunkComponent::HasChunkHigherRes(TransitionDirection Direction)
{
	return CurrentOctree->Depth != 0 && ChunkHasHigherRes[Direction];
}

bool UVoxelChunkComponent::UpdateFoliage()
{
	if (FoliageTasks.Num() == 0)
	{
		CompletedFoliageTaskCount = 0;
		for (int Index = 0; Index < Render->World->GrassTypes.Num(); Index++)
		{
			auto GrassType = Render->World->GrassTypes[Index];
			for (auto GrassVariety : GrassType->GrassVarieties)
			{
				if (GrassVariety.CullDepth >= CurrentOctree->Depth)
				{
					FAsyncTask<FAsyncFoliageTask>* FoliageTask = new FAsyncTask<FAsyncFoliageTask>(
						Section
						, GrassVariety
						, Index
						, Render->World
						, CurrentOctree->GetMinimalCornerPosition()
						, this);

					FoliageTask->StartBackgroundTask(Render->FoliageThreadPool);
					FoliageTasks.Add(FoliageTask);
				}
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}


void UVoxelChunkComponent::OnFoliageComplete()
{
	CompletedFoliageTaskCount++;
	if (CompletedFoliageTaskCount == FoliageTasks.Num())
	{
		OnAllFoliageComplete();
	}
}

void UVoxelChunkComponent::OnAllFoliageComplete()
{
	Render->AddApplyNewFoliage(this);
	CompletedFoliageTaskCount = 0;
}

void UVoxelChunkComponent::ApplyNewFoliage()
{
	for (int i = 0; i < FoliageComponents.Num(); i++)
	{
		FoliageComponents[i]->DestroyComponent();
	}
	FoliageComponents.Empty();

	for (auto FoliageTask : FoliageTasks)
	{
		FoliageTask->EnsureCompletion();
		FAsyncFoliageTask Task = FoliageTask->GetTask();
		if (Task.InstanceBuffer.NumInstances())
		{
			FVoxelGrassVariety GrassVariety = Task.GrassVariety;

			int32 FolSeed = FCrc::StrCrc32((GrassVariety.GrassMesh->GetName() + GetName()).GetCharArray().GetData());
			if (FolSeed == 0)
			{
				FolSeed++;
			}

			//Create component
			UHierarchicalInstancedStaticMeshComponent* HierarchicalInstancedStaticMeshComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(Render->World, NAME_None, RF_Transient);

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

			HierarchicalInstancedStaticMeshComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
			FTransform DesiredTransform = this->GetComponentTransform();
			DesiredTransform.RemoveScaling();
			HierarchicalInstancedStaticMeshComponent->SetWorldTransform(DesiredTransform);

			FoliageComponents.Add(HierarchicalInstancedStaticMeshComponent);

			if (!HierarchicalInstancedStaticMeshComponent->PerInstanceRenderData.IsValid())
			{
				HierarchicalInstancedStaticMeshComponent->InitPerInstanceRenderData(&Task.InstanceBuffer);
			}
			else
			{
				HierarchicalInstancedStaticMeshComponent->PerInstanceRenderData->UpdateFromPreallocatedData(HierarchicalInstancedStaticMeshComponent, Task.InstanceBuffer);
			}

			HierarchicalInstancedStaticMeshComponent->AcceptPrebuiltTree(Task.ClusterTree, Task.OutOcclusionLayerNum);

			//HierarchicalInstancedStaticMeshComponent->RecreateRenderState_Concurrent();
			HierarchicalInstancedStaticMeshComponent->MarkRenderStateDirty();
		}

		delete FoliageTask;
	}
	FoliageTasks.Empty();
}


void UVoxelChunkComponent::Serialize(FArchive& Ar)
{

}

void UVoxelChunkComponent::DeleteTasks()
{
	if (MeshBuilder)
	{
		MeshBuilder->EnsureCompletion();
		delete MeshBuilder;
		MeshBuilder = nullptr;
	}
	if (Builder)
	{
		delete Builder;
		Builder = nullptr;
	}
	for (auto FoliageTask : FoliageTasks)
	{
		FoliageTask->EnsureCompletion();
		delete FoliageTask;
	}
	FoliageTasks.Empty();
	CompletedFoliageTaskCount = 0;
}

void UVoxelChunkComponent::CreateBuilder()
{
	check(!Builder);
	Builder = new FVoxelPolygonizer(
		CurrentOctree->Depth,
		Render->World->Data,
		CurrentOctree->GetMinimalCornerPosition(),
		ChunkHasHigherRes,
		CurrentOctree->Depth != 0 && Render->World->bComputeTransitions,
		CurrentOctree->Depth == 0 && Render->World->bComputeCollisions
	);
}
