// Copyright 2020 Phyronnaz

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
	UVoxelGraphGenerator* const Generator;
	TSharedPtr<SVoxelGraphPreview> const Preview;
	TSharedPtr<SVoxelGraphPreviewViewport> const PreviewViewport;
	TSharedPtr<FAdvancedPreviewScene> const PreviewScene;
	
	TVoxelSharedPtr<FVoxelData> Data;

	UStaticMeshComponent* PreviewSceneFloor = nullptr;
	UVoxelLineBatchComponent* LineBatchComponent = nullptr;
	
	UMaterialInstanceDynamic* HeightmapMaterial = nullptr;
	UMaterialInstanceDynamic* SliceMaterial = nullptr;

	UTexture2D* DensitiesTexture = nullptr;
	UTexture2D* MaterialsTexture = nullptr;
	UTexture2D* MaterialsTextureWithCrossAndNoAlpha = nullptr;
	
	void UpdateTextures(EVoxelGraphPreviewFlags Flags);
	void UpdateMaterialParameters();
	void AddMessages(FVoxelGraphGeneratorInstance& GraphGeneratorInstance) const;
};