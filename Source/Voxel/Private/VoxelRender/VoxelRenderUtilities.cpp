// Copyright 2019 Phyronnaz

#include "VoxelRender/VoxelRenderUtilities.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelWorld.h"

#include "Materials/MaterialInstanceDynamic.h"

DECLARE_CYCLE_STAT(TEXT("FVoxelRenderUtilities::CreateMeshSectionFromChunks"), STAT_FVoxelRenderUtilities_CreateMeshSectionFromChunks, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("AddTransitions"), STAT_FVoxelRenderUtilities_CreateMeshSectionFromChunks_AddTransitions, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("SetBuffers"), STAT_FVoxelRenderUtilities_CreateMeshSectionFromChunks_SetBuffers, STATGROUP_Voxel);

static TAutoConsoleVariable<int32> CVarShowLODs(
	TEXT("voxel.ShowLODs"),
	0,
	TEXT("If true, will color chunks according to their LODs"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowInvisibleChunks( //TODO: Tick in debug manager that show info
	TEXT("voxel.ShowInvisibleChunks"),
	0,
	TEXT("If true, will show chunks used only for collisions/navmesh"),
	ECVF_Default);

inline FColor GetLODColor(int32 LOD)
{
	static const TArray<FColor> Colors = { FColor::Red, FColor::Green, FColor::Blue, FColor::Yellow, FColor::Cyan, FColor::Magenta, FColor::Orange, FColor::Purple, FColor::Turquoise, FColor::Emerald };
	return Colors[LOD % Colors.Num()];
}

inline FColor GetChunkSettingsColor(const FVoxelRenderChunkSettings& Settings)
{
	if (Settings.bEnableCollisions && Settings.bEnableNavmesh)
	{
		return FColor::Yellow;
	}
	if (Settings.bEnableCollisions)
	{
		return FColor::Blue;
	}
	if (Settings.bEnableNavmesh)
	{
		return FColor::Green;
	}
	return FColor::White;
}

inline UMaterialInstanceDynamic* InitializeMaterialInstance(int32 LOD, bool bShouldFade, UVoxelProceduralMeshComponent* Mesh, UMaterialInterface* Interface, float ChunksDitheringDuration)
{
	UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(Interface, Mesh);
	MaterialInstance->AddToCluster(Mesh, true);
	MaterialInstance->SetScalarParameterValue(FName(TEXT("StartTime")), bShouldFade ? FVoxelRenderUtilities::GetWorldCurrentTime(Mesh->GetWorld()) : -ChunksDitheringDuration);
	MaterialInstance->SetScalarParameterValue(FName(TEXT("FadeDuration")), ChunksDitheringDuration);
	MaterialInstance->SetScalarParameterValue(FName(TEXT("EndTime")), 0); // Needed for first init, as 1e10 is too big
	MaterialInstance->SetScalarParameterValue(FName(TEXT("EndTime")), 1e10);
	MaterialInstance->SetScalarParameterValue(FName(TEXT("LOD")), LOD);
	return MaterialInstance;
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

void FVoxelRenderUtilities::StartMeshDithering(UVoxelProceduralMeshComponent* Mesh, float ChunksDitheringDuration)
{
	for (auto& Section : Mesh->GetSections())
	{
		UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(Section.Material);
		if (Material)
		{
			Material->SetScalarParameterValue(FName(TEXT("EndTime")), FVoxelRenderUtilities::GetWorldCurrentTime(Mesh->GetWorld()) + ChunksDitheringDuration);
		}
	}
}

void FVoxelRenderUtilities::ResetDithering(UVoxelProceduralMeshComponent* Mesh)
{
	for (auto& Section : Mesh->GetSections())
	{
		UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(Section.Material);
		if (Material)
		{
			Material->SetScalarParameterValue(FName(TEXT("StartTime")), 0);
			Material->SetScalarParameterValue(FName(TEXT("EndTime")), 0); // Needed for first init, as 1e10 is too big
			Material->SetScalarParameterValue(FName(TEXT("EndTime")), 1e10);
		}
	}
}

void FVoxelRenderUtilities::HideMesh(UVoxelProceduralMeshComponent* Mesh)
{
	for (auto& Section : Mesh->GetSections())
	{
		Section.bSectionVisible = false;
	}
	Mesh->MarkRenderStateDirty();
}

inline void AddTransitions(
	FVoxelProcMeshSection& Section,
	const FVoxelChunkBuffers& MainChunkBuffers,
	const FVoxelChunkBuffers& TransitionChunkBuffers,
	uint8 TransitionsMask, 
	bool bTranslateVertices,
	int32 LOD,
	bool bEnableTessellation)
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelRenderUtilities_CreateMeshSectionFromChunks_AddTransitions);

	check(!bEnableTessellation || MainChunkBuffers.HasAdjacency() == TransitionChunkBuffers.HasAdjacency());

	int32 VertexCount = MainChunkBuffers.GetNumVertices() + TransitionChunkBuffers.GetNumVertices();

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
	if (bEnableTessellation)
	{
		Section.AdjacencyIndices.Append(MainChunkBuffers.AdjacencyIndices);
	}

	Section.Positions.AddUninitialized(MainChunkBuffers.GetNumVertices());
	for (int32 I = 0; I < MainChunkBuffers.GetNumVertices(); I++)
	{
		Section.Positions[I] = (bTranslateVertices && TransitionsMask) ? FVoxelRenderUtilities::GetTranslated(MainChunkBuffers.Positions[I], MainChunkBuffers.Normals[I], TransitionsMask, LOD) : MainChunkBuffers.Positions[I];
	}
	Section.Normals.Append(MainChunkBuffers.Normals);
	Section.Tangents.Append(MainChunkBuffers.Tangents);
	Section.Colors.Append(MainChunkBuffers.Colors);
	Section.TextureCoordinates.Append(MainChunkBuffers.TextureCoordinates);

	// Transition
	int32 VertexOffset = MainChunkBuffers.GetNumVertices();
	int32 IndexOffset = MainChunkBuffers.Indices.Num();
	Section.Indices.AddUninitialized(TransitionChunkBuffers.Indices.Num());
	for (int32 I = 0; I < TransitionChunkBuffers.Indices.Num(); I++)
	{
		Section.Indices[IndexOffset + I] = VertexOffset + TransitionChunkBuffers.Indices[I];
	}
	if (bEnableTessellation)
	{
		int32 AdjacencyIndexOffset = MainChunkBuffers.AdjacencyIndices.Num();
		Section.AdjacencyIndices.AddUninitialized(TransitionChunkBuffers.AdjacencyIndices.Num());
		for (int32 I = 0; I < TransitionChunkBuffers.AdjacencyIndices.Num(); I++)
		{
			Section.AdjacencyIndices[AdjacencyIndexOffset + I] = VertexOffset + TransitionChunkBuffers.AdjacencyIndices[I];
		}
	}
	Section.Positions.Append(TransitionChunkBuffers.Positions);
	Section.Normals.Append(TransitionChunkBuffers.Normals);
	Section.Tangents.Append(TransitionChunkBuffers.Tangents);
	Section.Colors.Append(TransitionChunkBuffers.Colors);
	Section.TextureCoordinates.Append(TransitionChunkBuffers.TextureCoordinates);

	check(bEnableTessellation || Section.AdjacencyIndices.Num() == 0);
}

inline void SetBuffers(FVoxelProcMeshSection& Section, const FVoxelChunkBuffers& Buffers)
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelRenderUtilities_CreateMeshSectionFromChunks_SetBuffers);

	Section.Indices = Buffers.Indices;	
	Section.AdjacencyIndices = Buffers.AdjacencyIndices;
	Section.Positions = Buffers.Positions;
	Section.Normals = Buffers.Normals;
	Section.Tangents = Buffers.Tangents;
	Section.Colors = Buffers.Colors;
	Section.TextureCoordinates = Buffers.TextureCoordinates;
}

static FString DebugMaterialName("/Voxel/M_VoxelDebugMaterial");

inline UMaterialInterface* GetDebugMaterial()
{
	static TWeakObjectPtr<UMaterialInterface> Material;
	if (!Material.IsValid())
	{
		Material = LoadObject<UMaterialInterface>(nullptr, *DebugMaterialName);
		Material->AddToRoot();
	}
	return Material.Get();
}

inline void ApplyDebugSettings(FVoxelProcMeshSection& Section, int32 LOD, const FVoxelRenderChunkSettings& ChunkSettings)
{
	if (CVarShowLODs.GetValueOnAnyThread())
	{
		auto ColorToSet = GetLODColor(LOD);
		for (auto& Color : Section.Colors)
		{
			Color = ColorToSet;
		}
		Section.Material = GetDebugMaterial();
	}
	else if (CVarShowInvisibleChunks.GetValueOnGameThread() && !ChunkSettings.bVisible)
	{
		auto ColorToSet = GetChunkSettingsColor(ChunkSettings);
		for (auto& Color : Section.Colors)
		{
			Color = ColorToSet;
		}
		Section.bSectionVisible = true;
		Section.Material = GetDebugMaterial();
	}
}

void FVoxelRenderUtilities::CreateMeshSectionFromChunks(
	int32 LOD,
	uint64 Priority,
	bool bShouldFade,
	const FVoxelRendererSettings& RendererSettings, 
	const FVoxelRenderChunkSettings& ChunkSettings, 
	UVoxelProceduralMeshComponent* Mesh,
	FVoxelChunkMaterials& ChunkMaterials,
	const TSharedPtr<FVoxelChunk>& MainChunk,
	uint8 TransitionsMask,
	const TSharedPtr<FVoxelChunk>& TransitionChunk)
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelRenderUtilities_CreateMeshSectionFromChunks);

	check(Mesh);
	check(MainChunk.IsValid());

	const int32 ChunkSize = CHUNK_SIZE << LOD;
	const bool bTranslateVertices = RendererSettings.RenderType == EVoxelRenderType::MarchingCubes;
	const float BoundsExtension = ChunkSettings.bEnableTessellation ? RendererSettings.TessellationBoundsExtension : 0;
	const FBox SectionBounds = FBox(-FVector::OneVector * (1 + BoundsExtension), (ChunkSize + 2 + BoundsExtension) * FVector::OneVector);

	Mesh->ClearSections(EVoxelProcMeshSectionUpdate::DelayUpdate);

	bool bNeedMaterialUpdate = false;

	if (MainChunk->bSingleBuffers)
	{
		FVoxelProcMeshSection Section;

		UMaterialInterface* MaterialInstance = ChunkMaterials.GetSingleMaterial();
		if (!MaterialInstance)
		{
			MaterialInstance = InitializeMaterialInstance(LOD, bShouldFade, Mesh, RendererSettings.GetVoxelMaterial(ChunkSettings.bEnableTessellation), RendererSettings.ChunksDitheringDuration);
			ChunkMaterials.SetSingleMaterial(MaterialInstance);
			bNeedMaterialUpdate = true;
		}
		if (TransitionChunk.IsValid())
		{
			AddTransitions(
				Section, 
				MainChunk->SingleBuffers, 
				TransitionChunk->SingleBuffers, 
				TransitionsMask, 
				bTranslateVertices, 
				LOD, 
				ChunkSettings.bEnableTessellation);
		}
		else
		{
			SetBuffers(Section, MainChunk->SingleBuffers);
		}

		Section.SectionLocalBox = SectionBounds;
		Section.bEnableCollision = ChunkSettings.bEnableCollisions;
		Section.bEnableNavmesh =  ChunkSettings.bEnableNavmesh;
		Section.bSectionVisible = ChunkSettings.bVisible;
		Section.Material = MaterialInstance;
		ApplyDebugSettings(Section, LOD, ChunkSettings);

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

		int32 SectionIndex = 0;
		for (auto& Material : MaterialsSet)
		{
			FVoxelProcMeshSection Section;

			UMaterialInterface* MaterialInstance = ChunkMaterials.GetMultipleMaterial(Material);
			if (!MaterialInstance)
			{
				MaterialInstance = InitializeMaterialInstance(LOD, bShouldFade, Mesh, RendererSettings.GetVoxelMaterial(Material, ChunkSettings.bEnableTessellation), RendererSettings.ChunksDitheringDuration);
				ChunkMaterials.SetMultipleMaterial(Material, MaterialInstance);
				bNeedMaterialUpdate = true;
			}

			FVoxelChunkBuffers* MainBuffers = MainChunk->Map.Find(Material);
			FVoxelChunkBuffers* TransitionBuffers = TransitionChunk.IsValid() ? TransitionChunk->Map.Find(Material) : nullptr;
			if (MainBuffers)
			{
				if (TransitionBuffers)
				{
					AddTransitions(
						Section, 
						*MainBuffers, 
						*TransitionBuffers, 
						TransitionsMask, 
						bTranslateVertices, 
						LOD,
						ChunkSettings.bEnableTessellation);
				}
				else
				{
					SetBuffers(Section, *MainBuffers);
				}
			}
			else
			{
				check(TransitionBuffers); // Else why are we iterating on this material?
				SetBuffers(Section, *TransitionBuffers);
			}
			Section.SectionLocalBox = SectionBounds;
			Section.bEnableCollision = ChunkSettings.bEnableCollisions;
			Section.bEnableNavmesh =  ChunkSettings.bEnableNavmesh;
			Section.bSectionVisible = ChunkSettings.bVisible;
			Section.Material = MaterialInstance;
			ApplyDebugSettings(Section, LOD, ChunkSettings);

			Mesh->SetProcMeshSection(SectionIndex++, MoveTemp(Section), EVoxelProcMeshSectionUpdate::DelayUpdate);
		}
	}

	if (bNeedMaterialUpdate)
	{
		Mesh->UpdatePhysicalMaterials();
	}

	Mesh->Priority = Priority;
	Mesh->FinishSectionsUpdates();
}

bool FVoxelRenderUtilities::DebugInvisibleChunks()
{
	return CVarShowInvisibleChunks.GetValueOnGameThread();
}