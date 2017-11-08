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
}

UVoxelChunkComponent::~UVoxelChunkComponent()
{
	if (Render)
	{
		Render->ChunkHasBeenDestroyed(this);
	}
	DeleteTasks();
}

void UVoxelChunkComponent::Init(TWeakPtr<FChunkOctree> NewOctree)
{
	check(!Render);
	check(!MeshBuilder);
	check(!Builder);
	check(!CurrentOctree.IsValid());
	check(NewOctree.IsValid());

	CurrentOctree = NewOctree.Pin();
	Render = CurrentOctree->Render;

	FIntVector NewPosition = CurrentOctree->GetMinimalCornerPosition();

	SetWorldLocation(Render->GetGlobalPosition(NewPosition));
	SetWorldScale3D(FVector::OneVector * Render->World->GetVoxelSize());

	// Needed because octree is only partially builded when Init is called
	Render->AddTransitionCheck(this);

	ChunkHasHigherRes.SetNumZeroed(6);
}

bool UVoxelChunkComponent::Update(bool bAsync)
{
	check(Render);

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
			MeshBuilder->StartBackgroundTask(CurrentOctree->Depth == 0 ? Render->HighPriorityMeshThreadPool : Render->MeshThreadPool);

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
	check(Render);

	if (Render->World->GetComputeTransitions())
	{
		for (int i = 0; i < 6; i++)
		{
			auto Direction = (TransitionDirection)i;
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
	check(Render);

	DeleteTasks();

	Render->AddTransitionCheck(this); // Needed because octree is only partially updated when Unload is called
	Render->ScheduleDeletion(this);
}

void UVoxelChunkComponent::Delete()
{
	check(Render);

	Render = nullptr;

	DeleteTasks();

	// Reset mesh
	SetProcMeshSection(0, FVoxelProcMeshSection());

	// Delete foliage
	for (auto FoliageComponent : FoliageComponents)
	{
		FoliageComponent->DestroyComponent();
	}
	FoliageComponents.Empty();

	// Reset octree ptr
	CurrentOctree.Reset();
}

void UVoxelChunkComponent::OnMeshComplete(FVoxelProcMeshSection& InSection)
{
	check(Render);
	check(MeshBuilder);

	SCOPE_CYCLE_COUNTER(STAT_SetProcMeshSection);

	Section = InSection;

	Render->AddApplyNewMesh(this);
}

void UVoxelChunkComponent::ApplyNewMesh()
{
	check(Render);

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
	check(CurrentOctree.IsValid());

	return CurrentOctree->Depth != 0 && ChunkHasHigherRes[Direction];
}

bool UVoxelChunkComponent::UpdateFoliage()
{
	check(Render);

	if (FoliageTasks.Num() == 0)
	{
		CompletedFoliageTaskCount = 0;

		int GrassVarietyIndex = 0;
		for (int Index = 0; Index < Render->World->GrassTypes.Num(); Index++)
		{
			if (Render->World->GrassTypes[Index])
			{
				auto GrassType = Render->World->GrassTypes[Index];
				for (auto GrassVariety : GrassType->GrassVarieties)
				{
					if (GrassVariety.CullDepth >= CurrentOctree->Depth)
					{
						FAsyncTask<FAsyncFoliageTask>* FoliageTask = new FAsyncTask<FAsyncFoliageTask>(
							Section
							, GrassVariety
							, GrassVarietyIndex
							, Index
							, Render->World
							, CurrentOctree->GetMinimalCornerPosition()
							, this);
						GrassVarietyIndex++;

						FoliageTask->StartBackgroundTask(Render->FoliageThreadPool);
						FoliageTasks.Add(FoliageTask);
					}
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
	check(Render);

	Render->AddApplyNewFoliage(this);
	CompletedFoliageTaskCount = 0;
}

void UVoxelChunkComponent::ApplyNewFoliage()
{
	check(Render);

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
			UHierarchicalInstancedStaticMeshComponent* HierarchicalInstancedStaticMeshComponent = NewObject<UHierarchicalInstancedStaticMeshComponent>(Render->ChunksParent, NAME_None, RF_Transient);

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


void UVoxelChunkComponent::ResetRender()
{
	Render = nullptr;
}

void UVoxelChunkComponent::Serialize(FArchive& Ar)
{

}

void UVoxelChunkComponent::DeleteTasks()
{
	if (MeshBuilder)
	{
		if (!MeshBuilder->Cancel())
		{
			MeshBuilder->EnsureCompletion();
		}
		check(MeshBuilder->IsDone());
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
		if (!FoliageTask->Cancel())
		{
			FoliageTask->EnsureCompletion();
		}
		check(FoliageTask->IsDone());
		delete FoliageTask;
	}
	FoliageTasks.Empty();
	CompletedFoliageTaskCount = 0;
}

void UVoxelChunkComponent::CreateBuilder()
{
	check(Render);
	check(!Builder);

	Builder = new FVoxelPolygonizer(
		CurrentOctree->Depth,
		Render->Data,
		CurrentOctree->GetMinimalCornerPosition(),
		ChunkHasHigherRes,
		CurrentOctree->Depth != 0 && Render->World->GetComputeTransitions(),
		CurrentOctree->Depth == 0 && Render->World->GetComputeCollisions(),
		Render->World->GetEnableAmbientOcclusion(),
		Render->World->GetRayMaxDistance(),
		Render->World->GetRayCount()
	);
}
