// Copyright 2019 Phyronnaz

#include "VoxelRender/VoxelRenderUtilities.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "VoxelWorld.h"
#include "VoxelLogStatDefinitions.h"

DECLARE_CYCLE_STAT(TEXT("FVoxelRenderUtilities::CreateMeshSectionFromChunks"), STAT_FVoxelRenderUtilities_CreateMeshSectionFromChunks, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelRenderUtilities::CreateMeshSectionFromChunks.AddTransitions"), STAT_FVoxelRenderUtilities_CreateMeshSectionFromChunks_AddTransitions, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelRenderUtilities::CreateMeshSectionFromChunks.SetBuffers"), STAT_FVoxelRenderUtilities_CreateMeshSectionFromChunks_SetBuffers, STATGROUP_Voxel);

inline FColor GetLODColor(int LOD)
{
	static const TArray<FColor> Colors = { FColor::Red, FColor::Green, FColor::Blue, FColor::Yellow, FColor::Cyan, FColor::Magenta, FColor::Orange, FColor::Purple, FColor::Turquoise, FColor::Emerald };
	return Colors[LOD % Colors.Num()];
}

inline UMaterialInstanceDynamic* InitializeMaterialInstance(int LOD, bool bShouldFade, UVoxelProceduralMeshComponent* Mesh, UMaterialInterface* Interface, float ChunkFadeDuration, UMaterialInstanceDynamic* ParamInstance)
{
	UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(Interface, Mesh);
	MaterialInstance->CopyInterpParameters(ParamInstance);
	MaterialInstance->AddToCluster(Mesh, true);
	MaterialInstance->SetScalarParameterValue(FName(TEXT("StartTime")), bShouldFade ? FVoxelRenderUtilities::GetWorldCurrentTime(Mesh->GetWorld()) : -ChunkFadeDuration);
	MaterialInstance->SetScalarParameterValue(FName(TEXT("FadeDuration")), ChunkFadeDuration);
	MaterialInstance->SetScalarParameterValue(FName(TEXT("EndTime")), 0); // Needed for first init, as 1e10 is too big
	MaterialInstance->SetScalarParameterValue(FName(TEXT("EndTime")), 1e10);
	MaterialInstance->SetScalarParameterValue(FName(TEXT("LOD")), LOD);
	return MaterialInstance;
}

inline void AddTransitions(FVoxelProcMeshSection& Section, const FVoxelChunkBuffers& MainChunkBuffers, const FVoxelChunkBuffers& TransitionChunkBuffers, uint8 TransitionsMask, bool bColorTransitions, int LOD)
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelRenderUtilities_CreateMeshSectionFromChunks_AddTransitions);

	int VertexCount = MainChunkBuffers.GetNumVertices() + TransitionChunkBuffers.GetNumVertices();

	// Reserve
	Section.Indices.Reserve(MainChunkBuffers.Indices.Num() + TransitionChunkBuffers.Indices.Num());
	Section.AdjacencyIndices.Reserve(MainChunkBuffers.AdjacencyIndices.Num() + TransitionChunkBuffers.AdjacencyIndices.Num());

	Section.Positions.Reserve(VertexCount);
	Section.Normals.Reserve(VertexCount);
	Section.Tangents.Reserve(VertexCount);
	Section.Colors.Reserve(VertexCount);
	Section.TextureCoordinates.Reserve(VertexCount);

	// Main
	Section.Indices.Append(MainChunkBuffers.Indices);
	Section.AdjacencyIndices.Append(MainChunkBuffers.AdjacencyIndices);
	
	Section.Positions.AddUninitialized(MainChunkBuffers.GetNumVertices());
	for (int I = 0; I < MainChunkBuffers.GetNumVertices(); I++)
	{
		Section.Positions[I] = TransitionsMask ? FVoxelRenderUtilities::GetTranslated(MainChunkBuffers.Positions[I], MainChunkBuffers.Normals[I], TransitionsMask, LOD) : MainChunkBuffers.Positions[I];
	}
	Section.Normals.Append(MainChunkBuffers.Normals);
	Section.Tangents.Append(MainChunkBuffers.Tangents);
	Section.Colors.Append(MainChunkBuffers.Colors);
	Section.TextureCoordinates.Append(MainChunkBuffers.TextureCoordinates);

	// Transition
	int VertexOffset = MainChunkBuffers.GetNumVertices();
	int IndexOffset = MainChunkBuffers.Indices.Num();
	Section.Indices.AddUninitialized(TransitionChunkBuffers.Indices.Num());
	for (int I = 0; I < TransitionChunkBuffers.Indices.Num(); I++)
	{
		Section.Indices[IndexOffset + I] = VertexOffset + TransitionChunkBuffers.Indices[I];
	}
	int AdjacencyIndexOffset = MainChunkBuffers.AdjacencyIndices.Num();
	Section.AdjacencyIndices.AddUninitialized(TransitionChunkBuffers.AdjacencyIndices.Num());
	for (int I = 0; I < TransitionChunkBuffers.AdjacencyIndices.Num(); I++)
	{
		Section.AdjacencyIndices[AdjacencyIndexOffset + I] = VertexOffset + TransitionChunkBuffers.AdjacencyIndices[I];
	}
	Section.Positions.Append(TransitionChunkBuffers.Positions);
	Section.Normals.Append(TransitionChunkBuffers.Normals);
	Section.Tangents.Append(TransitionChunkBuffers.Tangents);
	Section.Colors.Append(TransitionChunkBuffers.Colors);
	Section.TextureCoordinates.Append(TransitionChunkBuffers.TextureCoordinates);
	
	if (bColorTransitions)
	{
		auto ColorToSet = GetLODColor(LOD);
		for (auto& Color : Section.Colors)
		{
			Color = ColorToSet;
		}
	}
}

inline void SetBuffers(FVoxelProcMeshSection& Section, const FVoxelChunkBuffers& Buffers, bool bColorTransitions, int LOD)
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelRenderUtilities_CreateMeshSectionFromChunks_SetBuffers);

	Section.Indices = Buffers.Indices;	
	Section.AdjacencyIndices = Buffers.AdjacencyIndices;
	Section.Positions = Buffers.Positions;
	Section.Normals = Buffers.Normals;
	Section.Tangents = Buffers.Tangents;
	Section.Colors = Buffers.Colors;
	Section.TextureCoordinates = Buffers.TextureCoordinates;

	if (bColorTransitions)
	{
		auto ColorToSet = GetLODColor(LOD);
		for (auto& Color : Section.Colors)
		{
			Color = ColorToSet;
		}
	}
}

void FVoxelRenderUtilities::CreateMeshSectionFromChunks(int LOD, bool bShouldFade, AVoxelWorld* World, UVoxelProceduralMeshComponent* Mesh, const TSharedPtr<FVoxelChunkMaterials>& ChunkMaterials, const TSharedPtr<FVoxelChunk>& MainChunk, uint8 TransitionsMask, const TSharedPtr<FVoxelChunk>& TransitionChunk)
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelRenderUtilities_CreateMeshSectionFromChunks);

	check(World);
	check(Mesh);
	check(MainChunk.IsValid());

	const int ChunkSize = CHUNK_SIZE << LOD;
	const bool bColorTransitions = World->GetColorTransitions();

	Mesh->ClearSections(EVoxelProcMeshSectionUpdate::DelayUpdate);

	bool bNeedMaterialUpdate = false;

	FBox SectionBounds = FBox(-FVector::OneVector * (1 + World->GetBoundsExtension()), (ChunkSize + 2 + World->GetBoundsExtension()) * FVector::OneVector);
	bool bEnableCollision = Mesh->GetCollisionEnabled() != ECollisionEnabled::NoCollision;
	bool bEnableNavmesh = LOD <= World->GetMaxNavmeshLOD();

	if (MainChunk->bSingleBuffers)
	{
		FVoxelProcMeshSection Section;

		UMaterialInstanceDynamic* MaterialInstance = ChunkMaterials->GetSingleMaterial();
		if (!MaterialInstance)
		{
			MaterialInstance = InitializeMaterialInstance(LOD, bShouldFade, Mesh, World->GetVoxelMaterial(LOD), World->GetChunksFadeDuration(), World->GetMaterialInstance());
			ChunkMaterials->SetSingleMaterial(MaterialInstance);
			bNeedMaterialUpdate = true;
		}
		if (TransitionChunk.IsValid())
		{
			AddTransitions(Section, MainChunk->SingleBuffers, TransitionChunk->SingleBuffers, TransitionsMask, bColorTransitions, LOD);
		}
		else
		{
			SetBuffers(Section, MainChunk->SingleBuffers, bColorTransitions, LOD);
		}
		Section.SectionLocalBox = SectionBounds;
		Section.bEnableCollision = bEnableCollision;
		Section.bEnableNavmesh = bEnableNavmesh;
		Section.bSectionVisible = !World->GetDontRender();
		Section.Material = MaterialInstance;

		Mesh->SetProcMeshSection(0, MoveTemp(Section), EVoxelProcMeshSectionUpdate::DelayUpdate);
	}
	else
	{
		TSet<FVoxelBlendedMaterial> MaterialsSet;
		for (auto It = MainChunk->Map.CreateIterator(); It; ++It)
		{
			MaterialsSet.Add(It.Key());
		}
		if (TransitionChunk.IsValid())
		{
			for (auto It = TransitionChunk->Map.CreateIterator(); It; ++It)
			{
				MaterialsSet.Add(It.Key());
			}
		}

		int SectionIndex = 0;
		for (auto& Material : MaterialsSet)
		{
			FVoxelProcMeshSection Section;

			UMaterialInstanceDynamic* MaterialInstance = ChunkMaterials->GetMultipleMaterial(Material);
			if (!MaterialInstance)
			{
				MaterialInstance = InitializeMaterialInstance(LOD, bShouldFade, Mesh, World->GetVoxelMaterial(LOD, Material), World->GetChunksFadeDuration(), World->GetMaterialInstance());
				ChunkMaterials->SetMultipleMaterial(Material, MaterialInstance);
				bNeedMaterialUpdate = true;
			}

			FVoxelChunkBuffers* MainBuffers = MainChunk->Map.Find(Material);
			FVoxelChunkBuffers* TransitionBuffers = TransitionChunk.IsValid() ? TransitionChunk->Map.Find(Material) : nullptr;
			if (MainBuffers)
			{
				if (TransitionBuffers)
				{
					AddTransitions(Section, *MainBuffers, *TransitionBuffers, TransitionsMask, bColorTransitions, LOD);
				}
				else
				{
					SetBuffers(Section, *MainBuffers, bColorTransitions, LOD);
				}
			}
			else
			{
				check(TransitionBuffers); // Else why are we iterating on this material?
				SetBuffers(Section, *TransitionBuffers, bColorTransitions, LOD);
			}
			Section.SectionLocalBox = SectionBounds;
			Section.bEnableCollision = bEnableCollision;
			Section.bEnableNavmesh = bEnableNavmesh;
			Section.bSectionVisible = !World->GetDontRender();
			Section.Material = MaterialInstance;

			Mesh->SetProcMeshSection(SectionIndex++, MoveTemp(Section), EVoxelProcMeshSectionUpdate::DelayUpdate);
		}
	}

	if (bNeedMaterialUpdate)
	{
		Mesh->UpdateMaterials();
	}

	Mesh->FinishSectionsUpdates();
}

float FVoxelRenderUtilities::GetWorldCurrentTime(UWorld* World)
{
	if (World->WorldType == EWorldType::Editor)
	{
		return FApp::GetCurrentTime() - GStartTime;
	}
	else
	{
		return World->GetTimeSeconds();
	}
}