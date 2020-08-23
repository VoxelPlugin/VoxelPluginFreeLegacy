// Copyright 2020 Phyronnaz

#include "VoxelGraphPreview.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"
#include "VoxelGraphEditor.h"
#include "VoxelGraphGenerator.h"
#include "VoxelGraphPreviewSettings.h"
#include "VoxelGraphErrorReporter.h"
#include "VoxelDebug/VoxelLineBatchComponent.h"
#include "VoxelData/VoxelDataIncludes.h"
#include "VoxelUtilities/VoxelMaterialUtilities.h"
#include "VoxelUtilities/VoxelTextureUtilities.h"
#include "VoxelUtilities/VoxelThreadingUtilities.h"
#include "VoxelUtilities/VoxelDistanceFieldUtilities.h"
#include "VoxelPlaceableItems/VoxelPlaceableItemManager.h"
#include "VoxelWorldGenerators/VoxelWorldGeneratorCache.h"
#include "VoxelWorldInterface.h"

#include "SVoxelGraphPreview.h"
#include "SVoxelGraphPreviewViewport.h"

#include "Misc/MessageDialog.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h"
#include "AdvancedPreviewScene.h"
#include "Kismet/KismetMathLibrary.h"

FVoxelGraphPreview::FVoxelGraphPreview(
		UVoxelGraphGenerator* WorldGenerator,
		const TSharedPtr<SVoxelGraphPreview>& Preview,
		const TSharedPtr<SVoxelGraphPreviewViewport>& PreviewViewport,
		const TSharedPtr<FAdvancedPreviewScene>& PreviewScene)
	: WorldGenerator(WorldGenerator)
	, Preview(Preview)
	, PreviewViewport(PreviewViewport)
	, PreviewScene(PreviewScene)
{
	check(WorldGenerator && WorldGenerator->PreviewSettings);
	
	PreviewScene->SetLightBrightness(0.f);
	PreviewScene->SetFloorVisibility(false, true);
	PreviewScene->SetEnvironmentVisibility(false, true);
	PreviewScene->SetSkyBrightness(0.f);
		
	PreviewSceneFloor = NewObject<UStaticMeshComponent>();
	LineBatchComponent = NewObject<UVoxelLineBatchComponent>();
	
	PreviewScene->AddComponent(PreviewSceneFloor, FTransform::Identity);
	PreviewScene->AddComponent(LineBatchComponent, FTransform::Identity);
}

void FVoxelGraphPreview::Update(EVoxelGraphPreviewFlags Flags)
{
	if (EnumHasAnyFlags(Flags, EVoxelGraphPreviewFlags::UpdatePlaceableItems))
	{
		Flags |= EVoxelGraphPreviewFlags::UpdateTextures;
	}
	if (EnumHasAnyFlags(Flags, EVoxelGraphPreviewFlags::UpdateTextures))
	{
		Flags |= EVoxelGraphPreviewFlags::UpdateMeshSettings;
	}
	
	if (EnumHasAnyFlags(Flags, EVoxelGraphPreviewFlags::UpdateTextures))
	{
		UpdateTextures(Flags);
	}
	if (EnumHasAnyFlags(Flags, EVoxelGraphPreviewFlags::UpdateMeshSettings))
	{
		UpdateMaterialParameters();
	}
	
	PreviewViewport->RefreshViewport();
}

void FVoxelGraphPreview::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(PreviewSceneFloor);
	Collector.AddReferencedObject(LineBatchComponent);
	
	Collector.AddReferencedObject(HeightmapMaterial);
	Collector.AddReferencedObject(SliceMaterial);
	
	Collector.AddReferencedObject(DensitiesTexture);
	Collector.AddReferencedObject(MaterialsTexture);
	Collector.AddReferencedObject(MaterialsTextureWithCrossAndNoAlpha);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphPreview::UpdateTextures(EVoxelGraphPreviewFlags Flags)
{
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphPreview::UpdateMaterialParameters()
{
	VOXEL_FUNCTION_COUNTER();
	
	const UVoxelGraphPreviewSettings& Settings = *WorldGenerator->PreviewSettings;
	const auto Wrapper = FVoxelGraphPreviewSettingsWrapper(Settings);

	if (Settings.bHeightmapMode)
	{
		HeightmapMaterial = UMaterialInstanceDynamic::Create(Settings.HeightmapMaterial, nullptr);
		if (!ensure(HeightmapMaterial))
		{
			return;
		}

		HeightmapMaterial->SetTextureParameterValue(TEXT("Color"), MaterialsTexture);
		HeightmapMaterial->SetTextureParameterValue(TEXT("Height"), DensitiesTexture);
		HeightmapMaterial->SetScalarParameterValue(TEXT("Height"), Settings.Height);
		HeightmapMaterial->SetScalarParameterValue(TEXT("StartBias"), Settings.StartBias);
		HeightmapMaterial->SetScalarParameterValue(TEXT("MaxSteps"), Settings.MaxSteps);
		HeightmapMaterial->SetScalarParameterValue(TEXT("UseHeightAsColor"), Settings.bHeightBasedColor ? 1.f : 0.f);
		HeightmapMaterial->SetScalarParameterValue(TEXT("UseWater"), Settings.bEnableWater ? 1.f : 0.f);
		HeightmapMaterial->SetVectorParameterValue(TEXT("LightDirection"), Settings.LightDirection);
		HeightmapMaterial->SetScalarParameterValue(TEXT("Brightness"), Settings.Brightness);
		HeightmapMaterial->SetScalarParameterValue(TEXT("ShadowDensity"), Settings.ShadowDensity);

		PreviewSceneFloor->SetStaticMesh(Settings.Mesh);
		PreviewSceneFloor->SetMaterial(0, HeightmapMaterial);
		PreviewSceneFloor->SetWorldScale3D(FVector(10));
		PreviewSceneFloor->SetBoundsScale(1e6f);
		PreviewSceneFloor->SetWorldRotation(FRotator::ZeroRotator);
	}
	else
	{
		SliceMaterial = UMaterialInstanceDynamic::Create(Settings.SliceMaterial, nullptr);
		if (!ensure(SliceMaterial))
		{
			return;
		}

		SliceMaterial->SetTextureParameterValue(TEXT("Color"), MaterialsTexture);

		PreviewSceneFloor->SetStaticMesh(Settings.Mesh);
		PreviewSceneFloor->SetMaterial(0, SliceMaterial);
		PreviewSceneFloor->SetWorldScale3D(FVector(Wrapper.Resolution / 200.f));

		const auto GetRotation = [&]()
		{
			const auto Make = [](const FVector& Vector, float Angle)
			{
				return FTransform(UKismetMathLibrary::RotatorFromAxisAndAngle(Vector, Angle));
			};
			const FVector X(1, 0, 0);
			const FVector Y(0, 1, 0);
			const FVector Z(0, 0, 1);
			
			switch (Settings.LeftToRight)
			{
			default: ensure(false);
			case EVoxelGraphPreviewAxes::X:
			{
				switch (Settings.BottomToTop)
				{
				default: ensure(false);
				case EVoxelGraphPreviewAxes::Y: return Make(Z, 90) * Make(Y, 180);
				case EVoxelGraphPreviewAxes::Z: return Make(X, -90) * Make(Y, -90);
				}
			}
			case EVoxelGraphPreviewAxes::Y:
			{
				switch (Settings.BottomToTop)
				{
				default: ensure(false);
				case EVoxelGraphPreviewAxes::X: return FTransform::Identity;
				case EVoxelGraphPreviewAxes::Z: return Make(Y, -90);
				}
			}
			case EVoxelGraphPreviewAxes::Z:
			{
				switch (Settings.BottomToTop)
				{
				default: ensure(false);
				case EVoxelGraphPreviewAxes::X: return Make(X, 90);
				case EVoxelGraphPreviewAxes::Y: return Make(Y, 90) * Make(X, 90);
				}
			}
			}
		};
		PreviewSceneFloor->SetWorldRotation(GetRotation().Rotator());
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelGraphPreview::AddMessages(FVoxelGraphGeneratorInstance& GraphGeneratorInstance) const
{
}