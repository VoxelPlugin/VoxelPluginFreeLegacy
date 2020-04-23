// Copyright 2020 Phyronnaz

#include "VoxelGraphPreview.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"
#include "VoxelGraphGenerator.h"
#include "VoxelGraphPreviewSettings.h"
#include "VoxelGraphErrorReporter.h"

#include "SVoxelGraphPreview.h"
#include "SVoxelGraphPreviewViewport.h"

#include "Misc/MessageDialog.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h"
#include "AdvancedPreviewScene.h"

inline void UpdateTexture(UTexture2D*& Texture, int32 Size, const TArray<FColor>& Colors)
{
	if (!Texture || Texture->GetSizeX() != Size || Texture->GetSizeY() != Size)
	{
		Texture = UTexture2D::CreateTransient(Size, Size);
		Texture->CompressionSettings = TC_HDR;
		Texture->SRGB = false;
	}
	FTexture2DMipMap& Mip = Texture->PlatformData->Mips[0];
	{
		void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memcpy(Data, Colors.GetData(), Colors.Num() * sizeof(FColor));
	}
	Mip.BulkData.Unlock();
	Texture->UpdateResource();
}

inline FLinearColor GetColor(const TArray<FLinearColor>& IndexColors, uint8 Index)
{
	if (IndexColors.IsValidIndex(Index))
	{
		return IndexColors[Index];
	}
	else
	{
		return FColor::Black;
	}
}

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
	PreviewScene->SetLightBrightness(0.f);
	PreviewScene->SetFloorVisibility(false, true);
	PreviewScene->SetEnvironmentVisibility(false, true);
	PreviewScene->SetSkyBrightness(0.f);
		
	PreviewSceneFloor = NewObject<UStaticMeshComponent>(GetTransientPackage(), NAME_None);
	PreviewScene->AddComponent(PreviewSceneFloor, FTransform::Identity);
}

void FVoxelGraphPreview::Update(bool bUpdateTextures, bool bAutomaticPreview)
{
	const UVoxelGraphPreviewSettings* Settings = WorldGenerator->PreviewSettings;

	if (!Settings->Mesh || !Settings->Material)
	{
		LOG_VOXEL(Error, TEXT("Invalid Mesh or Material in voxel graph preview settings (%s)"), *WorldGenerator->GetName());
		return;
	}

	if (bUpdateTextures)
	{
		FVoxelGraphErrorReporter::ClearNodesMessages(WorldGenerator);

		const int32 Resolution = Settings->Resolution;
		const int32 ResolutionScale = Settings->ResolutionScale;
		const FIntVector Size = Settings->GetSize();

		TArray<float> Values;
		TArray<FVoxelMaterial> Materials;
		Values.SetNumUninitialized(Resolution * Resolution);
		Materials.SetNumUninitialized(Resolution * Resolution);

		const bool bComputeMaterials = Settings->PreviewType2D == EVoxelGraphPreviewType::Material || !Settings->bHeightBasedColor;

		for (int32 X = 0; X < Resolution; X++)
		{
			for (int32 Y = 0; Y < Resolution; Y++)
			{
				const float DX = X / float(Resolution) * 2 - 1;
				const float DY = Y / float(Resolution) * 2 - 1;
				Values[X + Resolution * Y] = FVector2D(DX, DY).Size() - 0.5f;
				Materials[X + Resolution * Y] = FVector2D(DX, DY).Size() > 0.1
					? FVoxelMaterial::CreateFromColor(FColor::Blue)
					: FVoxelMaterial::CreateFromColor(FColor::Red);
			}
		}

		// Show seed errors
		WorldGenerator->GetDefaultSeeds();

		TArray<FColor> NewDensities;
		TArray<FColor> NewMaterials;
		NewDensities.SetNumUninitialized(Resolution * Resolution);
		NewMaterials.SetNumUninitialized(Resolution * Resolution);
		
		const EVoxelMaterialConfig MaterialConfig = Settings->MaterialConfig;
		const TArray<FLinearColor>& IndexColors = Settings->IndexColors;
		for (int32 X = 0; X < Resolution; X++)
		{
			for (int32 Y = 0; Y < Resolution; Y++)
			{
				const FIntVector Position = Settings->GetPosition(X, Y);
				const int32 TextureIndex = X + Resolution * Y;
				const int32 DataIndex = Position.X + Position.Y * Size.X + Position.Z * Size.X * Size.Y;

				ensure(FIntBox(FIntVector::ZeroValue, Size).Contains(Position));

				{
					float Alpha = (Values[DataIndex] - Settings->MinValue) / (Settings->MaxValue - Settings->MinValue);
					uint8 IntAlpha = FVoxelUtilities::FloatToUINT8(Alpha);
					NewDensities[TextureIndex] = FColor(IntAlpha, IntAlpha, IntAlpha, 255);
				}

				{
					FVoxelMaterial& Material = Materials[DataIndex];
					FColor Color;

					if (MaterialConfig == EVoxelMaterialConfig::RGB)
					{
						Color = Material.GetColor();
					}
					else if (MaterialConfig == EVoxelMaterialConfig::SingleIndex)
					{
						Color = GetColor(IndexColors, Material.GetSingleIndex_Index()).ToFColor(false);
					}
					else
					{
						check(MaterialConfig == EVoxelMaterialConfig::DoubleIndex);
						FLinearColor ColorA = GetColor(IndexColors, Material.GetDoubleIndex_IndexA());
						FLinearColor ColorB = GetColor(IndexColors, Material.GetDoubleIndex_IndexB());
						Color = FMath::Lerp(ColorA, ColorB, Material.GetDoubleIndex_Blend_AsFloat()).ToFColor(false);
					}

					// Ignore alpha as it does silly stuff
					Color.A = 255;

					NewMaterials[TextureIndex] = Color;
				}
			}
		}

		UpdateTexture(DensitiesTexture, Resolution, NewDensities);
		UpdateTexture(MaterialsTexture, Resolution, NewMaterials);

		Preview->SetTexture(Settings->PreviewType2D == EVoxelGraphPreviewType::Density ? DensitiesTexture : MaterialsTexture);
		WorldGenerator->SetPreviewTexture(Settings->PreviewType2D == EVoxelGraphPreviewType::Density ? NewDensities : NewMaterials, Resolution);
	}

	if (!PreviewSceneMaterial || PreviewSceneMaterial->Parent != Settings->Material)
	{
		PreviewSceneMaterial = UMaterialInstanceDynamic::Create(Settings->Material, GetTransientPackage(), NAME_None);
	}
	PreviewSceneMaterial->SetTextureParameterValue(TEXT("Color"), MaterialsTexture);
	PreviewSceneMaterial->SetTextureParameterValue(TEXT("Height"), DensitiesTexture);
	PreviewSceneMaterial->SetScalarParameterValue(TEXT("Height"), Settings->Height);
	PreviewSceneMaterial->SetScalarParameterValue(TEXT("StartBias"), Settings->StartBias);
	PreviewSceneMaterial->SetScalarParameterValue(TEXT("MaxSteps"), Settings->MaxSteps);
	PreviewSceneMaterial->SetScalarParameterValue(TEXT("UseHeightAsColor"), Settings->bHeightBasedColor ? 1.f : 0.f);
	PreviewSceneMaterial->SetScalarParameterValue(TEXT("UseWater"), Settings->bEnableWater ? 1.f : 0.f);
	PreviewSceneMaterial->SetVectorParameterValue(TEXT("LightDirection"), Settings->LightDirection);
	PreviewSceneMaterial->SetScalarParameterValue(TEXT("Brightness"), Settings->Brightness);
	PreviewSceneMaterial->SetScalarParameterValue(TEXT("ShadowDensity"), Settings->ShadowDensity);

	PreviewSceneFloor->SetStaticMesh(Settings->Mesh);
	PreviewSceneFloor->SetMaterial(0, PreviewSceneMaterial);
	PreviewSceneFloor->SetWorldScale3D(Settings->MeshScale);
	PreviewSceneFloor->SetBoundsScale(1e6f);
	
	PreviewViewport->RefreshViewport();
}

void FVoxelGraphPreview::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(PreviewSceneFloor);
	Collector.AddReferencedObject(PreviewSceneMaterial);
	Collector.AddReferencedObject(DensitiesTexture);
	Collector.AddReferencedObject(MaterialsTexture);
}