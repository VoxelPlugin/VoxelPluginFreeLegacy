// Copyright Voxel Plugin SAS. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"

class SVoxelGraphPreview;
class SVoxelGraphPreviewViewport;

class FVoxelData;
class FReferenceCollector;
class FAdvancedPreviewScene;
class FVoxelGraphGeneratorInstance;

class UTexture2D;
class UStaticMeshComponent;
class UVoxelGraphGenerator;
class UVoxelLineBatchComponent;
class UMaterialInstanceDynamic;

enum class EVoxelGraphPreviewFlags;

class FVoxelGraphPreview
{
public:
	FVoxelGraphPreview(
		UVoxelGraphGenerator* Generator,
		const TSharedPtr<SVoxelGraphPreview>& Preview,
		const TSharedPtr<SVoxelGraphPreviewViewport>& PreviewViewport,
		const TSharedPtr<FAdvancedPreviewScene>& PreviewScene);

	void Update(EVoxelGraphPreviewFlags Flags);
	void AddReferencedObjects(FReferenceCollector& Collector);

private:
	TObjectPtr<UVoxelGraphGenerator> const Generator;
	TSharedPtr<SVoxelGraphPreview> const Preview;
	TSharedPtr<SVoxelGraphPreviewViewport> const PreviewViewport;
	TSharedPtr<FAdvancedPreviewScene> const PreviewScene;
	
	TVoxelSharedPtr<FVoxelData> Data;

	TObjectPtr<UStaticMeshComponent> PreviewSceneFloor = nullptr;
	TObjectPtr<UVoxelLineBatchComponent> LineBatchComponent = nullptr;
	
	TObjectPtr<UMaterialInstanceDynamic> HeightmapMaterial = nullptr;
	TObjectPtr<UMaterialInstanceDynamic> SliceMaterial = nullptr;

	TObjectPtr<UTexture2D> DensitiesTexture = nullptr;
	TObjectPtr<UTexture2D> MaterialsTexture = nullptr;
	TObjectPtr<UTexture2D> MaterialsTextureWithCrossAndNoAlpha = nullptr;
	
	void UpdateTextures(EVoxelGraphPreviewFlags Flags);
	void UpdateMaterialParameters();
	void AddMessages(FVoxelGraphGeneratorInstance& GraphGeneratorInstance) const;
};