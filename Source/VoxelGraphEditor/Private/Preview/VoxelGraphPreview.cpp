// Copyright 2020 Phyronnaz

#include "VoxelGraphPreview.h"
#include "VoxelValue.h"
#include "VoxelMaterial.h"
#include "VoxelGraphGenerator.h"
#include "VoxelGraphPreviewSettings.h"
#include "VoxelGraphErrorReporter.h"
#include "VoxelUtilities/VoxelMaterialUtilities.h"

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

		TArray<v_flt> Values;
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

		// Show seed errors & update preview settings seeds
		{
			TMap<FName, int32> Seeds = WorldGenerator->GetDefaultSeeds();

			for (auto& It : Settings->Seeds)
			{
				int32* Value = Seeds.Find(It.Key);
				if (Value)
				{
					*Value = It.Value;
				}
			}
			Settings->Seeds = MoveTemp(Seeds);
		}

		const bool bIsPreviewingPin = WorldGenerator->PreviewedPin.Get() != nullptr;
		const bool bShowDensities = Settings->PreviewType2D == EVoxelGraphPreviewType::Density || bIsPreviewingPin;

		TArray<FColor> NewDensities;
		TArray<FColor> NewMaterials;
		NewDensities.SetNumUninitialized(Resolution * Resolution);
		NewMaterials.SetNumUninitialized(Resolution * Resolution);

		v_flt MinValue;
		v_flt MaxValue;
		if (Settings->bAutoNormalize)
		{
			MinValue = Values[0];
			MaxValue = Values[0];
			for (const auto& Value : Values)
			{
				MinValue = FMath::Min(Value, MinValue);
				MaxValue = FMath::Max(Value, MaxValue);
			}
		}
		else
		{
			MinValue = Settings->NormalizeMinValue;
			MaxValue = Settings->NormalizeMaxValue;
		}

		if (bShowDensities)
		{
			Settings->MinValue = float(MinValue);
			Settings->MaxValue = float(MaxValue);
		}
		else
		{
			// For materials, values are between 0 and 1
			Settings->MinValue = 0.f;
			Settings->MaxValue = 1.f;
		}
		
		for (int32 X = 0; X < Resolution; X++)
		{
			for (int32 Y = 0; Y < Resolution; Y++)
			{
				const FIntVector Position = Settings->GetPosition(X, Y);
				const int32 TextureIndex = X + Resolution * Y;
				const int32 DataIndex = Position.X + Position.Y * Size.X + Position.Z * Size.X * Size.Y;

				ensure(FVoxelIntBox(FIntVector::ZeroValue, Size).Contains(Position));
				
				const float Value = Values[DataIndex];
				const float Alpha = (Value - MinValue) / (MaxValue - MinValue);
			
				{
					uint8 IntAlpha = FVoxelUtilities::FloatToUINT8(Alpha);
					NewDensities[TextureIndex] = FColor(IntAlpha, IntAlpha, IntAlpha, 255);
				}
				
				if (bShowDensities)
				{
					if (Settings->bDrawColoredDistanceField)
					{
						// Credit for this snippet goes to Inigo Quilez
						const float ScaledValue = Value / (Resolution << ResolutionScale) * 2;

						FLinearColor Color = FLinearColor::White - FMath::Sign(ScaledValue) * FLinearColor(0.1, 0.4, 0.7, 0.f);
						Color *= 1.0 - FMath::Exp(-3.0 * FMath::Abs(ScaledValue));
						Color *= 0.8 + 0.2 * FMath::Cos(150.0 * ScaledValue);
						Color = FMath::Lerp(Color, FLinearColor::White, 1.0 - FMath::SmoothStep(0.0, 0.01, FMath::Abs(ScaledValue)));
						Color.A = 1.f;
						NewMaterials[TextureIndex] = FLinearColor(Color.ToFColor(false)).ToFColor(false);
					}
					else
					{
						NewMaterials[TextureIndex] = NewDensities[TextureIndex];
					}
				}
				else
				{
					const FVoxelMaterial& Material = Materials[DataIndex];
					FColor Color = FColor::Black;

					const auto GetColor = [&](int32 Index)
					{
						if (Settings->IndexColors.IsValidIndex(Index))
						{
							return Settings->IndexColors[Index];
						}
						else
						{
							return FColor::Black;
						}
					};

					switch (Settings->MaterialPreviewType)
					{
					case EVoxelGraphMaterialPreviewType::RGB:
					{
						Color.R = Material.GetR();
						Color.G = Material.GetG();
						Color.B = Material.GetB();
						break;
					}
					case EVoxelGraphMaterialPreviewType::Alpha:
					{
						Color.R = Material.GetA();
						Color.G = Material.GetA();
						Color.B = Material.GetA();
						break;
					}
					case EVoxelGraphMaterialPreviewType::SingleIndex:
					{
						Color = GetColor(Material.GetSingleIndex());
						break;
					}
					case EVoxelGraphMaterialPreviewType::MultiIndex_Overview:
					{
						const TVoxelStaticArray<float, 4> Strengths = FVoxelUtilities::GetMultiIndexStrengths(Material);

						const FColor Color0 = GetColor(Material.GetMultiIndex_Index0());
						const FColor Color1 = GetColor(Material.GetMultiIndex_Index1());
						const FColor Color2 = GetColor(Material.GetMultiIndex_Index2());
						const FColor Color3 = GetColor(Material.GetMultiIndex_Index3());
						
						Color.R = FVoxelUtilities::ClampToUINT8(FMath::RoundToInt(Color0.R * Strengths[0] + Color1.R * Strengths[1] + Color2.R * Strengths[2] + Color3.R * Strengths[3]));
						Color.G = FVoxelUtilities::ClampToUINT8(FMath::RoundToInt(Color0.G * Strengths[0] + Color1.G * Strengths[1] + Color2.G * Strengths[2] + Color3.G * Strengths[3]));
						Color.B = FVoxelUtilities::ClampToUINT8(FMath::RoundToInt(Color0.B * Strengths[0] + Color1.B * Strengths[1] + Color2.B * Strengths[2] + Color3.B * Strengths[3]));
						
						break;
					}
					case EVoxelGraphMaterialPreviewType::MultiIndex_SingleIndexPreview:
					{
						const TVoxelStaticArray<float, 4> Strengths = FVoxelUtilities::GetMultiIndexStrengths(Material);

						float Strength = 0;
						if (Settings->MultiIndexToPreview == Material.GetMultiIndex_Index0()) Strength += Strengths[0];
						if (Settings->MultiIndexToPreview == Material.GetMultiIndex_Index1()) Strength += Strengths[1];
						if (Settings->MultiIndexToPreview == Material.GetMultiIndex_Index2()) Strength += Strengths[2];
						if (Settings->MultiIndexToPreview == Material.GetMultiIndex_Index3()) Strength += Strengths[3];

						Color.R = FVoxelUtilities::FloatToUINT8(Strength);
						Color.G = FVoxelUtilities::FloatToUINT8(Strength);
						Color.B = FVoxelUtilities::FloatToUINT8(Strength);

						break;
					}
					case EVoxelGraphMaterialPreviewType::MultiIndex_Wetness:
					{
						Color.R = Material.GetMultiIndex_Wetness();
						Color.G = Material.GetMultiIndex_Wetness();
						Color.B = Material.GetMultiIndex_Wetness();

						break;
					}
					case EVoxelGraphMaterialPreviewType::UV0:
					{
						Color.R = Material.GetU0();
						Color.G = Material.GetV0();
						break;
					}
					case EVoxelGraphMaterialPreviewType::UV1:
					{
						Color.R = Material.GetU1();
						Color.G = Material.GetV1();
						break;
					}
					case EVoxelGraphMaterialPreviewType::UV2:
					{
						Color.R = Material.GetU2();
						Color.G = Material.GetV2();
						break;
					}
					case EVoxelGraphMaterialPreviewType::UV3:
					{
						Color.R = Material.GetU3();
						Color.G = Material.GetV3();
						break;
					}
					default: ensure(false);
					}

					Color.A = 255;

					NewMaterials[TextureIndex] = Color;
				}
			}
		}

		UpdateTexture(DensitiesTexture, Resolution, NewDensities);
		UpdateTexture(MaterialsTexture, Resolution, NewMaterials);

		Preview->SetTexture(MaterialsTexture);
		WorldGenerator->SetPreviewTexture(NewMaterials, Resolution);
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