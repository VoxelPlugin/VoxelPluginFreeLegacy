// Copyright 2020 Phyronnaz

#include "VoxelRender/VoxelRenderUtilities.h"
#include "VoxelRender/VoxelProceduralMeshComponent.h"
#include "VoxelRender/VoxelProcMeshBuffers.h"
#include "VoxelRender/VoxelMaterialInterface.h"
#include "VoxelRender/VoxelChunkMaterials.h"
#include "VoxelRender/VoxelChunkMesh.h"
#include "VoxelRender/VoxelChunkToUpdate.h"
#include "VoxelRender/IVoxelRenderer.h"
#include "VoxelRender/Meshers/VoxelMesherUtilities.h"
#include "VoxelUtilities/VoxelMaterialUtilities.h"
#include "VoxelMessages.h"

#include "Materials/MaterialInstanceDynamic.h"

static TAutoConsoleVariable<int32> CVarMaxSectionsPerChunk(
	TEXT("voxel.renderer.MaxSectionsPerChunk"),
	128,
	TEXT("If a voxel chunk has more sections that this (eg due to single/double index), it won't be drawn. 1 section = 1 draw call"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarShowTransitions(
	TEXT("voxel.renderer.ShowTransitions"),
	0,
	TEXT("If true, will only show the transition meshes"),
	ECVF_Default);

float FVoxelRenderUtilities::GetWorldCurrentTime(UWorld* World)
{
	if (!ensure(World)) return 0;
	if (World->WorldType == EWorldType::Editor)
	{
		return FApp::GetCurrentTime() - GStartTime;
	}
	else
	{
		return World->GetTimeSeconds();
	}
}

void FVoxelRenderUtilities::InitializeMaterialInstance(
	UMaterialInstanceDynamic* MaterialInstance,
	int32 LOD,
	const FIntVector& Position,
	const FVoxelRendererSettingsBase& Settings)
{
	VOXEL_FUNCTION_COUNTER();

	MaterialInstance->SetScalarParameterValue(STATIC_FNAME("LOD"), LOD);
	MaterialInstance->SetVectorParameterValue(STATIC_FNAME("ChunkPosition"), FVector(Position));
	MaterialInstance->SetScalarParameterValue(STATIC_FNAME("VoxelSize"), Settings.VoxelSize);
	MaterialInstance->SetScalarParameterValue(STATIC_FNAME("ChunkSize"), RENDER_CHUNK_SIZE);
	MaterialInstance->SetScalarParameterValue(STATIC_FNAME("FadeDuration"), Settings.ChunksDitheringDuration);
}

template<typename T>
inline void IterateDynamicMaterials(UVoxelProceduralMeshComponent& Mesh, T Lambda)
{
	Mesh.IterateSectionsSettings([&](FVoxelProcMeshSectionSettings& SectionSettings)
	{
		if (!ensure(SectionSettings.Material.IsValid())) return;
		UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(SectionSettings.Material->GetMaterial());
		if (Material)
		{
			Lambda(*Material);
		}
	});
}

inline void SetMaterialDithering(
	UMaterialInstanceDynamic& Material,
	const FVoxelRendererSettingsBase& Settings,
	const FVoxelRenderUtilities::FDitheringInfo& DitheringInfo)
{
	if (Settings.RenderType == EVoxelRenderType::SurfaceNets)
	{
		check(DitheringInfo.DitheringType == EDitheringType::SurfaceNets_LowResToHighRes || DitheringInfo.DitheringType == EDitheringType::SurfaceNets_HighResToLowRes);
		Material.SetScalarParameterValue(STATIC_FNAME("StartTime"), DitheringInfo.Time);
		Material.SetScalarParameterValue(STATIC_FNAME("InvertedFade"), DitheringInfo.DitheringType == EDitheringType::SurfaceNets_HighResToLowRes ? 1 : 0);
	}
	else
	{
		check(DitheringInfo.DitheringType == EDitheringType::Classic_DitherIn || DitheringInfo.DitheringType == EDitheringType::Classic_DitherOut);

		// StartTime and EndTime are a bit tricky: what's actually done in the shader is
		// min(EndTime - Time, Time - StartTime) / FadeDuration
		if (DitheringInfo.DitheringType == EDitheringType::Classic_DitherIn)
		{
			Material.SetScalarParameterValue(STATIC_FNAME("StartTime"), DitheringInfo.Time);
			Material.SetScalarParameterValue(STATIC_FNAME("EndTime"), 1e8);
		}
		else
		{
			// First dither in new chunk, then dither out old chunk
			Material.SetScalarParameterValue(STATIC_FNAME("StartTime"), 0);
			Material.SetScalarParameterValue(STATIC_FNAME("EndTime"), DitheringInfo.Time + 2 * Settings.ChunksDitheringDuration);
		}
	}
}

void FVoxelRenderUtilities::StartMeshDithering(
	UVoxelProceduralMeshComponent& Mesh, 
	const FVoxelRendererSettingsBase& Settings, 
	const FDitheringInfo& DitheringInfo)
{
	VOXEL_FUNCTION_COUNTER();
	IterateDynamicMaterials(Mesh, [&](UMaterialInstanceDynamic& Material)
	{
		SetMaterialDithering(Material, Settings, DitheringInfo);
	});
}

void FVoxelRenderUtilities::ResetDithering(UVoxelProceduralMeshComponent& Mesh, const FVoxelRendererSettingsBase& Settings)
{
	VOXEL_FUNCTION_COUNTER();
	IterateDynamicMaterials(Mesh, [&](UMaterialInstanceDynamic& Material)
	{
		if (Settings.RenderType == EVoxelRenderType::SurfaceNets)
		{
			Material.SetScalarParameterValue(STATIC_FNAME("StartTime"), 0);
			Material.SetScalarParameterValue(STATIC_FNAME("InversedFade"), 0);
		}
		else
		{
			Material.SetScalarParameterValue(STATIC_FNAME("StartTime"), 0);
			Material.SetScalarParameterValue(STATIC_FNAME("EndTime"), 1e8);
		}
	});
}

void FVoxelRenderUtilities::SetMeshTransitionsMask(UVoxelProceduralMeshComponent& Mesh, uint8 TransitionMask)
{
	VOXEL_FUNCTION_COUNTER();
	IterateDynamicMaterials(Mesh, [&](UMaterialInstanceDynamic& Material)
	{
		float OldValue = 0;
		Material.GetScalarParameterValue(FMaterialParameterInfo(STATIC_FNAME("TransitionMask")), OldValue);
		if (OldValue != TransitionMask)
		{
			Material.SetScalarParameterValue(STATIC_FNAME("OldTransitionMask"), OldValue);
			Material.SetScalarParameterValue(STATIC_FNAME("TransitionMask"), TransitionMask);
			Material.SetScalarParameterValue(STATIC_FNAME("TransitionsStartTime"), GetWorldCurrentTime(Mesh.GetWorld()));
		}
	});
}

void FVoxelRenderUtilities::HideMesh(UVoxelProceduralMeshComponent& Mesh)
{
	VOXEL_FUNCTION_COUNTER();
	Mesh.IterateSectionsSettings([&](FVoxelProcMeshSectionSettings& SectionSettings)
	{
		SectionSettings.bSectionVisible = false;
	});
	Mesh.MarkRenderStateDirty();
}

void FVoxelRenderUtilities::ShowMesh(UVoxelProceduralMeshComponent& Mesh)
{
	VOXEL_FUNCTION_COUNTER();
	Mesh.IterateSectionsSettings([&](FVoxelProcMeshSectionSettings& SectionSettings)
	{
		SectionSettings.bSectionVisible = true;
	});
	Mesh.MarkRenderStateDirty();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#define CHECK_CANCEL() if (CancelCounter.GetValue() > CancelThreshold) return {};

TUniquePtr<FVoxelProcMeshBuffers> FVoxelRenderUtilities::MergeSections_AnyThread(
	const FVoxelRendererSettingsBase& RendererSettings,
	const TArray<FVoxelChunkMeshSection>& Sections,
	const FIntVector& CenterPosition,
	const FThreadSafeCounter& CancelCounter, 
	int32 CancelThreshold)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	const bool bShowMainChunks = CVarShowTransitions.GetValueOnAnyThread() == 0;

	auto ProcMeshBuffersPtr = MakeUnique<FVoxelProcMeshBuffers>();
	auto& ProcMeshBuffers = *ProcMeshBuffersPtr;

	int32 NumVertices = 0;
	int32 NumIndices = 0;
	int32 NumAdjacencyIndices = 0;
	int32 NumTextureCoordinates = -1;
	int32 NumTextureData = 0;
	for (auto& Section : Sections)
	{
		CHECK_CANCEL();
		
		const auto BufferIterator = [&](const FVoxelChunkMeshBuffers& ChunkBuffers)
		{
			ProcMeshBuffers.Guids.Add(ChunkBuffers.Guid);

			// Else wrong NumTextureCoordinates gets assigned
			// Only part of the buffers can be empty; having all of them empty is invalid
			if (ChunkBuffers.GetNumVertices() == 0) return;
			
			NumVertices += ChunkBuffers.GetNumVertices();
			NumIndices += ChunkBuffers.Indices.Num();
			if (Section.bEnableTessellation)
			{
				// 4x as much adjacency indices
				NumAdjacencyIndices += 4 * ChunkBuffers.Indices.Num();
			}

			if (NumTextureCoordinates == -1)
			{
				NumTextureCoordinates = ChunkBuffers.TextureCoordinates.Num();
			}
			else if (!ensure(NumTextureCoordinates == ChunkBuffers.TextureCoordinates.Num()))
			{
				NumTextureCoordinates = -2;
			}

			NumTextureData += ChunkBuffers.TextureData.Num();
		};

		if (Section.MainChunk.IsValid() && bShowMainChunks)
		{
			BufferIterator(*Section.MainChunk);
		}
		if (Section.TransitionChunk.IsValid())
		{
			BufferIterator(*Section.TransitionChunk);
		}
	}
	ensure(NumAdjacencyIndices == 4 * NumIndices || NumAdjacencyIndices == 0); // If false, then some chunks have tessellation enabled and some others don't
	if (!ensure(NumVertices > 0)) return {};
	if (!ensure(NumTextureCoordinates >= 0)) return {};
	
	auto& PositionBuffer = ProcMeshBuffers.VertexBuffers.PositionVertexBuffer;
	auto& StaticMeshBuffer = ProcMeshBuffers.VertexBuffers.StaticMeshVertexBuffer;
	auto& ColorBuffer = ProcMeshBuffers.VertexBuffers.ColorVertexBuffer;
	auto& IndexBuffer = ProcMeshBuffers.IndexBuffer;
	auto& AdjacencyIndexBuffer = ProcMeshBuffers.AdjacencyIndexBuffer;
	auto& TextureData = ProcMeshBuffers.TextureData;
	
	CHECK_CANCEL();

	{
		VOXEL_ASYNC_SCOPE_COUNTER("Init");
		
		PositionBuffer.Init(NumVertices, FVoxelProcMeshBuffers::bNeedsCPUAccess);
		if (RendererSettings.bRenderWorld)
		{
			StaticMeshBuffer.SetUseFullPrecisionUVs(!RendererSettings.bHalfPrecisionCoordinates);
			StaticMeshBuffer.Init(NumVertices, NumTextureCoordinates, FVoxelProcMeshBuffers::bNeedsCPUAccess);
			ColorBuffer.Init(NumVertices, FVoxelProcMeshBuffers::bNeedsCPUAccess);
			TextureData.Reserve(NumTextureData);
		}
		IndexBuffer.AllocateData(NumIndices);
		AdjacencyIndexBuffer.AllocateData(NumAdjacencyIndices);
	}

	CHECK_CANCEL();

	int32 VerticesOffset = 0;
	int32 IndicesOffset = 0;
	int32 AdjacencyIndicesOffset = 0;

	const auto CopyPositions = [&](const FVoxelChunkMeshBuffers& Chunk, const FVector& Offset)
	{
		VOXEL_ASYNC_SCOPE_COUNTER("CopyPositions");
		const int32 ChunkNumVertices = Chunk.GetNumVertices();
		for (int32 Index = 0; Index < ChunkNumVertices; Index++)
		{
			PositionBuffer.VertexPosition(VerticesOffset + Index) = FVoxelUtilities::Get(Chunk.Positions, Index) + Offset;
		}
	};
	const auto CopyColorsAndTextureData = [&](const FVoxelChunkMeshBuffers& Chunk)
	{
		if (!RendererSettings.bRenderWorld)
		{
			ensure(Chunk.Colors.Num() == 0);
			return;
		}

		if (Chunk.TextureData.Num() > 0)
		{
			VOXEL_ASYNC_SCOPE_COUNTER("CopyColorsAndTextureData");
			const int32 Offset = TextureData.Num();
			TextureData.Append(Chunk.TextureData);
			
			const int32 ChunkNumVertices = Chunk.GetNumVertices();
			for (int32 Index = 0; Index < ChunkNumVertices; Index++)
			{
				FVoxelMaterial Material = FVoxelMaterial::CreateFromColor(FVoxelUtilities::Get(Chunk.Colors, Index));
				if (Material.CubicColor_IsUsingTexture())
				{
					Material.CubicColor_SetTextureDataIndex(Material.CubicColor_GetTextureDataIndex() + Offset);
				}
				ColorBuffer.VertexColor(VerticesOffset + Index) = Material.GetColor();
			}
		}
		else
		{
			VOXEL_ASYNC_SCOPE_COUNTER("CopyColors");
			const int32 ChunkNumVertices = Chunk.GetNumVertices();
			for (int32 Index = 0; Index < ChunkNumVertices; Index++)
			{
				ColorBuffer.VertexColor(VerticesOffset + Index) = FVoxelUtilities::Get(Chunk.Colors, Index);
			}
		}
	};
	const auto CopyStaticMesh = [&](const FVoxelChunkMeshBuffers& Chunk)
	{
		if (!RendererSettings.bRenderWorld)
		{
			ensure(Chunk.Tangents.Num() == 0);
			ensure(Chunk.Normals.Num() == 0);
			for (auto& T : Chunk.TextureCoordinates) ensure(T.Num() == 0);
			return;
		}

		VOXEL_ASYNC_SCOPE_COUNTER("CopyStaticMesh");
		const int32 ChunkNumVertices = Chunk.GetNumVertices();
		for (int32 Index = 0; Index < ChunkNumVertices; Index++)
		{
			{
				auto& Tangent = FVoxelUtilities::Get(Chunk.Tangents, Index);
				auto& Normal = FVoxelUtilities::Get(Chunk.Normals, Index);
				StaticMeshBuffer.SetVertexTangents(VerticesOffset + Index, Tangent.TangentX, Tangent.GetY(Normal), Normal);
			}
			check(Chunk.TextureCoordinates.Num() == NumTextureCoordinates);
			for (int32 Tex = 0; Tex < NumTextureCoordinates; Tex++)
			{
				auto& TextureCoordinate = FVoxelUtilities::Get(Chunk.TextureCoordinates[Tex], Index);
				StaticMeshBuffer.SetVertexUV(VerticesOffset + Index, Tex, TextureCoordinate);
			}
		}
	};
	const auto CopyIndices = [&](const FVoxelChunkMeshBuffers& Chunk)
	{
		VOXEL_ASYNC_SCOPE_COUNTER("CopyIndices");
		for (int32 Index = 0; Index < Chunk.Indices.Num(); Index++)
		{
			IndexBuffer.SetIndex(IndicesOffset + Index, VerticesOffset + FVoxelUtilities::Get(Chunk.Indices, Index));
		}
	};
	const auto CopyAdjacencyIndices = [&](const FVoxelChunkMeshBuffers& Chunk)
	{
		TArray<uint32> AdjacencyIndices;
		Chunk.BuildAdjacency(AdjacencyIndices);
		ensure(AdjacencyIndices.Num() == 4 * Chunk.Indices.Num());
		
		VOXEL_ASYNC_SCOPE_COUNTER("CopyAdjacencyIndices");
		for (int32 Index = 0; Index < AdjacencyIndices.Num(); Index++)
		{
			AdjacencyIndexBuffer.SetIndex(AdjacencyIndicesOffset + Index, VerticesOffset + FVoxelUtilities::Get(AdjacencyIndices, Index));
		}
		return AdjacencyIndices.Num();
	};
	
	for (const FVoxelChunkMeshSection& Chunk : Sections)
	{
		CHECK_CANCEL();
		
		const FVector PositionOffset(Chunk.ChunkPosition - CenterPosition);

		// Copy main chunk
		if (Chunk.MainChunk.IsValid() && bShowMainChunks)
		{
			auto& MainChunk = *Chunk.MainChunk;

			// Copy bounds
			ProcMeshBuffers.LocalBounds += MainChunk.Bounds.ShiftBy(PositionOffset);

			if (Chunk.bTranslateVertices && Chunk.TransitionsMask)
			{
				VOXEL_ASYNC_SCOPE_COUNTER("TranslateVertices");
				for (int32 Index = 0; Index < MainChunk.GetNumVertices(); Index++)
				{
					PositionBuffer.VertexPosition(VerticesOffset + Index) = FVoxelMesherUtilities::GetTranslatedTransvoxel(
						FVoxelUtilities::Get(MainChunk.Positions, Index),
						FVoxelUtilities::Get(MainChunk.Normals, Index),
						Chunk.TransitionsMask,
						Chunk.LOD) + PositionOffset;
				}
			}
			else
			{
				CopyPositions(MainChunk, PositionOffset);
			}
			
			CopyColorsAndTextureData(MainChunk);
			CopyStaticMesh(MainChunk);
			CopyIndices(MainChunk);

			if (Chunk.bEnableTessellation)
			{
				AdjacencyIndicesOffset += CopyAdjacencyIndices(MainChunk);
			}
			
			VerticesOffset += MainChunk.GetNumVertices();
			IndicesOffset += MainChunk.Indices.Num();
		}

		// Copy transition chunk
		if (Chunk.TransitionChunk.IsValid())
		{
			auto& TransitionChunk = *Chunk.TransitionChunk;
			
			// Copy bounds
			ProcMeshBuffers.LocalBounds += TransitionChunk.Bounds.ShiftBy(PositionOffset);
			
			CopyPositions(TransitionChunk, PositionOffset);
			CopyColorsAndTextureData(TransitionChunk);
			CopyStaticMesh(TransitionChunk);
			CopyIndices(TransitionChunk);

			if (Chunk.bEnableTessellation)
			{
				AdjacencyIndicesOffset += CopyAdjacencyIndices(TransitionChunk);
			}
			
			VerticesOffset += TransitionChunk.GetNumVertices();
			IndicesOffset += TransitionChunk.Indices.Num();
		}
	}

	check(VerticesOffset == NumVertices);
	check(IndicesOffset == NumIndices);
	check(AdjacencyIndicesOffset == NumAdjacencyIndices);
	
	CHECK_CANCEL();

	// Bounds extension is in world space, and we're in local (voxel) space
	ProcMeshBuffers.LocalBounds = ProcMeshBuffers.LocalBounds.ExpandBy(RendererSettings.BoundsExtension / RendererSettings.VoxelSize);

#if VOXEL_DEBUG
	{
		VOXEL_ASYNC_SCOPE_COUNTER("Check");
		for (int32 Index = 0; Index < IndexBuffer.GetNumIndices(); Index++)
		{
			checkf(IndexBuffer.GetIndex(Index) < uint32(NumVertices), TEXT("Invalid index: %u < %u"), IndexBuffer.GetIndex(Index), uint32(NumVertices));
		}
		for (int32 Index = 0; Index < AdjacencyIndexBuffer.GetNumIndices(); Index++)
		{
			checkf(AdjacencyIndexBuffer.GetIndex(Index) < uint32(NumVertices), TEXT("Invalid index: %u < %u"), AdjacencyIndexBuffer.GetIndex(Index), uint32(NumVertices));
		}
	}
#endif

	ProcMeshBuffers.UpdateStats();
	
	CHECK_CANCEL();

	return ProcMeshBuffersPtr;
}

TUniquePtr<FVoxelBuiltChunkMeshes> FVoxelRenderUtilities::BuildMeshes_AnyThread(
	const FVoxelChunkMeshesToBuild& ChunkMeshesToBuild,
	const FVoxelRendererSettingsBase& RendererSettings,
	const FIntVector& Position,
	const FThreadSafeCounter& CancelCounter,
	int32 CancelThreshold)
{
	auto BuiltMeshesPtr = MakeUnique<FVoxelBuiltChunkMeshes>();
	auto& BuiltMeshes = *BuiltMeshesPtr;
	for (auto& MeshToBuild : ChunkMeshesToBuild)
	{
		const auto& MeshConfig = MeshToBuild.Key;
		TArray<TPair<FVoxelProcMeshSectionSettings, TUniquePtr<FVoxelProcMeshBuffers>>> BuiltSections;
		CHECK_CANCEL();
		for (auto& Section : MeshToBuild.Value)
		{
			const FVoxelProcMeshSectionSettings& SectionSettings = Section.Key;
			ensure(SectionSettings.bSectionVisible || SectionSettings.bEnableCollisions || SectionSettings.bEnableNavmesh);
			auto BuiltSection = MergeSections_AnyThread(RendererSettings, Section.Value, Position, CancelCounter, CancelThreshold);
			CHECK_CANCEL();
			BuiltSections.Emplace(SectionSettings, MoveTemp(BuiltSection));
		}
		BuiltMeshes.Emplace(MeshConfig, MoveTemp(BuiltSections));
		CHECK_CANCEL();
	}
	return BuiltMeshesPtr;
}

#undef CHECK_CANCEL

FVoxelChunkMeshesToBuild FVoxelRenderUtilities::GetMeshesToBuild(
	int32 LOD,
	const FIntVector& Position,
	const FVoxelRendererSettingsBase& RendererSettings,
	const FVoxelChunkSettings& ChunkSettings,
	FVoxelChunkMaterials& ChunkMaterials,
	const FVoxelChunkMesh& MainChunk,
	const FVoxelChunkMesh* TransitionChunk,
	const FVoxelOnMaterialInstanceCreated& OnMaterialInstanceCreated,
	const FDitheringInfo& DitheringInfo)
{
	VOXEL_FUNCTION_COUNTER();
	
	FVoxelChunkMeshesToBuild Meshes;

	const auto DefaultSection =
		FVoxelChunkMeshSection(
			LOD,
			Position,
			false, // Set below
			RendererSettings.RenderType == EVoxelRenderType::MarchingCubes && 
			// Don't translate if the transition chunk isn't built 
			TransitionChunk && 
			// No valid normals for these, so can't translate
			RendererSettings.NormalConfig != EVoxelNormalConfig::FlatNormal && 
			RendererSettings.NormalConfig != EVoxelNormalConfig::NoNormal,
			ChunkSettings.TransitionsMask);
	const auto DefaultMeshConfig = FVoxelMeshConfig().CopyFrom(*RendererSettings.ProcMeshClass->GetDefaultObject<UVoxelProceduralMeshComponent>());

	const auto CreateMaterialInstance = [&](UMaterialInterface* Interface) -> TVoxelSharedRef<FVoxelMaterialInterface>
	{
		if (!RendererSettings.bCreateMaterialInstances)
		{
			return FVoxelMaterialInterfaceManager::Get().CreateMaterial(Interface);
		}

		const auto MaterialInstance = FVoxelMaterialInterfaceManager::Get().CreateMaterialInstance(Interface);
		auto* MaterialInstanceObject = Cast<UMaterialInstanceDynamic>(MaterialInstance->GetMaterial());
		if (ensure(MaterialInstanceObject))
		{
			InitializeMaterialInstance(
				MaterialInstanceObject,
				LOD,
				Position,
				RendererSettings);
			OnMaterialInstanceCreated.Broadcast(LOD, FVoxelUtilities::GetBoundsFromPositionAndDepth<RENDER_CHUNK_SIZE>(Position, LOD), MaterialInstanceObject);
			if (DitheringInfo.bIsValid)
			{
				SetMaterialDithering(*MaterialInstanceObject, RendererSettings, DitheringInfo);
			}
		}

		return MaterialInstance;
	};
	
	if (MainChunk.IsSingle())
	{
		const auto CreateMaterial = [&]()
		{
			return CreateMaterialInstance(RendererSettings.GetVoxelMaterial(LOD));
		};
		const auto MaterialInstance = ChunkMaterials.FindOrAddSingle(CreateMaterial);

		auto& SectionMap = Meshes.FindOrAdd(DefaultMeshConfig);

		const bool bEnableTessellation = FVoxelUtilities::IsMaterialTessellated(MaterialInstance->GetMaterial());
		
		const FVoxelProcMeshSectionSettings SectionSettings(
			MaterialInstance,
			ChunkSettings.bEnableCollisions,
			ChunkSettings.bEnableNavmesh,
			bEnableTessellation,
			ChunkSettings.bVisible);
		auto& Sections = SectionMap.FindOrAdd(SectionSettings);

		auto& NewSection = Sections.Emplace_GetRef(DefaultSection);

		NewSection.bEnableTessellation = bEnableTessellation;
		NewSection.MainChunk = MainChunk.GetSingleBuffers();
		if (TransitionChunk)
		{
			NewSection.TransitionChunk = TransitionChunk->GetSingleBuffers();
		}
	}
	else
	{
		TSet<FVoxelMaterialIndices> MaterialsSet;
		{
			MainChunk.IterateMaterials([&](auto& Material) { MaterialsSet.Add(Material); });
			if (TransitionChunk)
			{
				TransitionChunk->IterateMaterials([&](auto& Material) { MaterialsSet.Add(Material); });
			}

			if (MaterialsSet.Num() > CVarMaxSectionsPerChunk.GetValueOnGameThread())
			{
				FVoxelMessages::Error(
						"Voxel chunk with more than voxel.renderer.MaxSectionsPerChunk mesh sections.\n"
						"Not rendering it to avoid performance drop (1 draw call per section).\n"
						"This is because you are changing your single index or double index too frequently\n"
						"You most likely painted RGB data with a Single or Double Index material config, or painted too many double index materials on a single chunk\n"
						"Decreasing your material collection Max Materials To Blend At Once might help\n"
						"You can use voxel.renderer.ShowMeshSections 1 to debug");
				return {};
			}
		}

		const auto ShouldSkip = [&](const FVoxelMaterialIndices& Indices)
		{
			for (uint8 HoleMaterial : RendererSettings.HolesMaterials)
			{
				for (int32 Index = 0; Index < Indices.NumIndices; Index++)
				{
					if (Indices.SortedIndices[Index] == HoleMaterial)
					{
						return true;
					}
				}
			}
			return false;
		};
		
		for (auto& Material : MaterialsSet)
		{
			if (ShouldSkip(Material))
			{
				continue;
			}

			const auto CreateMaterial = [&]()
			{
				return CreateMaterialInstance(RendererSettings.GetVoxelMaterial(LOD, Material));
			};
			const auto MaterialInstance = ChunkMaterials.FindOrAddMultiple(Material, CreateMaterial);

			// Note: we only use the first index to determine the mesh settings to use
			// This might lead to unwanted behavior in blendings
			auto* MaterialMeshConfig = RendererSettings.MaterialsMeshConfigs.Find(Material.SortedIndices[0]);
			auto& MeshConfig = MaterialMeshConfig ? *MaterialMeshConfig : DefaultMeshConfig;
			auto& SectionMap = Meshes.FindOrAdd(MeshConfig);

			const bool bEnableTessellation = FVoxelUtilities::IsMaterialTessellated(MaterialInstance->GetMaterial());
			
			const FVoxelProcMeshSectionSettings SectionSettings(
				MaterialInstance,
				ChunkSettings.bEnableCollisions,
				ChunkSettings.bEnableNavmesh,
				bEnableTessellation,
				ChunkSettings.bVisible);
			auto& Sections = SectionMap.FindOrAdd(SectionSettings);

			auto& NewSection = Sections[Sections.Emplace(DefaultSection)];
			NewSection.bEnableTessellation = bEnableTessellation;

			const auto MainBuffers = MainChunk.FindBuffer(Material);
			if (MainBuffers.IsValid())
			{
				NewSection.MainChunk = MainBuffers;
			}
			if (TransitionChunk)
			{
				const auto TransitionBuffers = TransitionChunk->FindBuffer(Material);
				if (TransitionBuffers.IsValid())
				{
					NewSection.TransitionChunk = TransitionBuffers;
				}
			}
			ensure(NewSection.MainChunk.IsValid() || NewSection.TransitionChunk.IsValid());
		}
	}

	return Meshes;
}