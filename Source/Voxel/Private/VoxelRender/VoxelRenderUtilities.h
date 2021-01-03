// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelRender/VoxelMeshConfig.h"
#include "VoxelRender/VoxelProcMeshSectionSettings.h"

struct FVoxelIntBox;
struct FVoxelChunkMesh;
struct FVoxelChunkMeshBuffers;
struct FVoxelChunkMaterials;
struct FVoxelChunkSettings;
struct FVoxelProcMeshBuffers;
class IVoxelRenderer;
class FVoxelRuntimeSettings;
class UMaterialInstanceDynamic;
class UVoxelProceduralMeshComponent;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FVoxelOnMaterialInstanceCreated, int32 /*ChunkLOD*/, const FVoxelIntBox& /*ChunkBounds*/, UMaterialInstanceDynamic* /*Instance*/);

struct FVoxelChunkMeshSection
{
	int32 LOD = -1;
	FIntVector ChunkPosition = FIntVector(ForceInit);
	bool bEnableTessellation = false;
	bool bTranslateVertices = false;
	uint8 TransitionsMask = 0;
	
	TVoxelSharedPtr<const FVoxelChunkMeshBuffers> MainChunk;
	TVoxelSharedPtr<const FVoxelChunkMeshBuffers> TransitionChunk;

	FVoxelChunkMeshSection() = default;
	FVoxelChunkMeshSection(
		int32 LOD,
		const FIntVector& ChunkPosition,
		bool bEnableTessellation,
		bool bTranslateVertices,
		uint8 TransitionsMask)
		: LOD(LOD)
		, ChunkPosition(ChunkPosition)
		, bEnableTessellation(bEnableTessellation)
		, bTranslateVertices(bTranslateVertices)
		, TransitionsMask(TransitionsMask)
	{
	}
};

// Map from mesh config -> section config -> array of meshes to merge into that section
using FVoxelChunkMeshesToBuild = TMap<FVoxelMeshConfig, TMap<FVoxelProcMeshSectionSettings, TArray<FVoxelChunkMeshSection>>>;
// Map from mesh config -> section config -> built section
using FVoxelBuiltChunkMeshes = TArray<TPair<FVoxelMeshConfig, TArray<TPair<FVoxelProcMeshSectionSettings, TUniquePtr<FVoxelProcMeshBuffers>>>>>;

enum class EDitheringType : uint8
{
	// if we need to fade from the parent to the child
	SurfaceNets_LowResToHighRes,
	// if we need to fade from the child to the parent
	SurfaceNets_HighResToLowRes,
	Classic_DitherIn,
	Classic_DitherOut
};

namespace FVoxelRenderUtilities
{
	struct FDitheringInfo
	{
		bool bIsValid = false;
		EDitheringType DitheringType = EDitheringType::Classic_DitherIn;
		float Time = 0;
	};
	
	float GetWorldCurrentTime(UWorld* World);

	void InitializeMaterialInstance(
		UMaterialInstanceDynamic* MaterialInstance,
		int32 LOD,
		const FIntVector& Position,
		const FVoxelRuntimeSettings& Settings);
	
	void StartMeshDithering(UVoxelProceduralMeshComponent& Mesh, const FVoxelRuntimeSettings& Settings, const FDitheringInfo& DitheringInfo);
	void ResetDithering(UVoxelProceduralMeshComponent& Mesh, const FVoxelRuntimeSettings& Settings);

	// For surface nets
	void SetMeshTransitionsMask(UVoxelProceduralMeshComponent& Mesh, uint8 TransitionMask);

	void HideMesh(UVoxelProceduralMeshComponent& Mesh);
	void ShowMesh(UVoxelProceduralMeshComponent& Mesh);

	TUniquePtr<FVoxelProcMeshBuffers> MergeSections_AnyThread(
		const FVoxelRuntimeSettings& RendererSettings,
		const TArray<FVoxelChunkMeshSection>& Sections, 
		const FIntVector& CenterPosition,
		const FThreadSafeCounter& CancelCounter = FThreadSafeCounter(),
		int32 CancelThreshold = 0);
	TUniquePtr<FVoxelBuiltChunkMeshes> BuildMeshes_AnyThread(
		const FVoxelChunkMeshesToBuild& ChunkMeshesToBuild,
		const FVoxelRuntimeSettings& RendererSettings,
		const FIntVector& Position,
		const FThreadSafeCounter& CancelCounter = FThreadSafeCounter(),
		int32 CancelThreshold = 0);

	FVoxelChunkMeshesToBuild GetMeshesToBuild(
		int32 LOD, 
		const FIntVector& Position,
		const IVoxelRenderer& Renderer, 
		const FVoxelChunkSettings& ChunkSettings,
		FVoxelChunkMaterials& ChunkMaterials, 
		const FVoxelChunkMesh& MainChunk, 
		const FVoxelChunkMesh* TransitionChunk,
		const FDitheringInfo& DitheringInfo); // DitheringInfo to apply to newly spawned materials
};