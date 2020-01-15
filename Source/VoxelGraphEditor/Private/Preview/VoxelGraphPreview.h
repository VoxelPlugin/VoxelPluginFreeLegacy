// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"

class UVoxelGraphGenerator;
class SVoxelGraphPreview;
class SVoxelGraphPreviewViewport;
class FAdvancedPreviewScene;
class FReferenceCollector;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UTexture2D;

class FVoxelGraphPreview
{
public:

	FVoxelGraphPreview(
		UVoxelGraphGenerator* WorldGenerator,
		const TSharedPtr<SVoxelGraphPreview>& Preview,
		const TSharedPtr<SVoxelGraphPreviewViewport>& PreviewViewport,
		const TSharedPtr<FAdvancedPreviewScene>& PreviewScene);

	void Update(bool bUpdateTextures, bool bAutomaticPreview);
	void AddReferencedObjects(FReferenceCollector& Collector);

private:
	UVoxelGraphGenerator* const WorldGenerator;
	TSharedPtr<SVoxelGraphPreview> const Preview;
	TSharedPtr<SVoxelGraphPreviewViewport> const PreviewViewport;
	TSharedPtr<FAdvancedPreviewScene> const PreviewScene;

	UStaticMeshComponent* PreviewSceneFloor = nullptr;
	UMaterialInstanceDynamic* PreviewSceneMaterial = nullptr;

	UTexture2D* DensitiesTexture = nullptr;
	UTexture2D* MaterialsTexture = nullptr;
};