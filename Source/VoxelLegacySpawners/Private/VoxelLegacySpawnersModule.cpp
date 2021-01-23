// Copyright 2021 Phyronnaz

#include "VoxelLegacySpawnersModule.h"
#include "VoxelMinimal.h"
#include "VoxelEditorUtilities.h"
#include "VoxelWorld.h"
#include "VoxelFoliage.h"
#include "VoxelMessages.h"
#include "VoxelFoliageCollection.h"
#include "VoxelMeshSpawner.h"
#include "VoxelSpawnerGroup.h"
#include "VoxelEditorDelegates.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelUtilities/VoxelSystemUtilities.h"

#include "Modules/ModuleManager.h"
#include "ContentBrowserModule.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

IMPLEMENT_MODULE(FVoxelLegacySpawnersModule, VoxelLegacySpawners)

void FVoxelLegacySpawnersModule::StartupModule()
{
	FVoxelEditorDelegates::OnMigrateLegacySpawners.AddLambda([=](AVoxelWorld* World)
	{
		// This is called in PostLoad, which isn't what we want as asset can't be dirtied then
		FVoxelSystemUtilities::DelayedCall([=]()
		{
			auto* SpawnerConfig = Cast<UVoxelSpawnerConfig>(World->SpawnerConfig_DEPRECATED);
			if (!ensure(SpawnerConfig))
			{
				return;
			}

			World->FoliageWorldType = EVoxelFoliageWorldType(SpawnerConfig->WorldType);
			World->bIsFourWayBlend = SpawnerConfig->FiveWayBlendSetup.bFourWayBlend;

			auto* NewCollection = Cast<UVoxelFoliageCollection>(Convert(SpawnerConfig));
			if (ensure(NewCollection))
			{
				World->FoliageCollections.Add(NewCollection);

				for (UVoxelFoliage* Foliage : NewCollection->Foliages)
				{
					if (Foliage && !Foliage->OutputPickerGenerator.IsValid())
					{
						Foliage->OutputPickerGenerator = World->Generator;
					}
				}
			}

			FVoxelMessages::Info("Legacy voxel spawners have been migrated to the new foliage system. New assets have been created. To manually migrate an asset, right click it.", World);

			World->MarkPackageDirty();

			if (World->IsCreated())
			{
				UVoxelBlueprintLibrary::Recreate(World);
			}
		});
	});
	
#define ADD(Old, New) \
	AssetsToConvert.Add( \
	{ \
		Old::StaticClass(), \
		New::StaticClass(), \
		[=](const UObject* OldObject, UObject* NewObject) \
		{ \
			Convert_ ## Old ## _ ## New(*CastChecked<const Old>(OldObject), *CastChecked<New>(NewObject)); \
		} \
	});

	ADD(UVoxelMeshSpawner, UVoxelFoliage);
	ADD(UVoxelMeshSpawnerGroup, UVoxelFoliage);
	ADD(UVoxelSpawnerGroup, UVoxelFoliage);
	ADD(UVoxelSpawnerConfig, UVoxelFoliageCollection);

#undef ADD
	
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	ContentBrowserModule.GetAllAssetViewContextMenuExtenders().Add(MakeLambdaDelegate([=](const TArray<FAssetData>& SelectedAssets)
	{
		const auto Extender = MakeShared<FExtender>();

		for (const FAssetData& Asset : SelectedAssets)
		{
			bool bReturn = true;
			for (const FAssetToConvert& AssetToConvert : AssetsToConvert)
			{
				if (AssetToConvert.OldClass == Asset.GetClass())
				{
					ensure(bReturn);
					bReturn = false;
				}
			}

			if (bReturn)
			{
				return Extender;
			}
		}
		
		Extender->AddMenuExtension(
			"CommonAssetActions",
			EExtensionHook::After,
			nullptr,
			FMenuExtensionDelegate::CreateLambda([=](FMenuBuilder& MenuBuilder)
			{
				MenuBuilder.AddMenuEntry(
					VOXEL_LOCTEXT("Convert legacy spawners to voxel foliage"),
					VOXEL_LOCTEXT("Will upgrate the legacy spawners assets to the new foliage system"),
					FSlateIcon(NAME_None, NAME_None),
					FUIAction(FExecuteAction::CreateLambda([=]()
					{
						for (const FAssetData& Asset : SelectedAssets)
						{
							auto* OldObject = Cast<UVoxelLegacySpawnerBase>(Asset.GetAsset());
							if (ensure(OldObject))
							{
								Convert(OldObject);
							}
						
						}
					})));
			}));

		return Extender;
	}));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UObject* FVoxelLegacySpawnersModule::Convert(const UVoxelLegacySpawnerBase* OldObject) const
{
	if (!OldObject)
	{
		return nullptr;
	}
	if (OldObject->NewAsset)
	{
		return OldObject->NewAsset;
	}

	OldObject->MarkPackageDirty();
	
	for (const FAssetToConvert& AssetToConvert : AssetsToConvert)
	{
		if (AssetToConvert.OldClass == OldObject->GetClass())
		{
			UObject* NewObject = FVoxelEditorUtilities::CreateAsset(OldObject->GetName(), FPackageName::GetLongPackagePath(OldObject->GetOutermost()->GetPathName()), AssetToConvert.NewClass, "_New");
			if (ensure(NewObject))
			{
				LOG_VOXEL(Log, TEXT("Converted %s to %s"), *OldObject->GetName(), *NewObject->GetName());
				AssetToConvert.Convert(OldObject, NewObject);
			}
			OldObject->NewAsset = NewObject;
			return NewObject;
		}
	}
	
	return nullptr;
}

void FVoxelLegacySpawnersModule::Convert_UVoxelMeshSpawner_UVoxelFoliage(const UVoxelMeshSpawner& MeshSpawner, UVoxelFoliage& Foliage) const
{
	Foliage.Meshes = { { MeshSpawner.Mesh } };
	Convert_UVoxelMeshSpawnerBase_UVoxelFoliage(MeshSpawner, Foliage);
}

void FVoxelLegacySpawnersModule::Convert_UVoxelMeshSpawnerGroup_UVoxelFoliage(const UVoxelMeshSpawnerGroup& MeshSpawner, UVoxelFoliage& Foliage) const
{
	Foliage.Meshes.Reset();
	for (UStaticMesh* Mesh : MeshSpawner.Meshes)
	{
		Foliage.Meshes.Add({ Mesh });
	}
	Convert_UVoxelMeshSpawnerBase_UVoxelFoliage(MeshSpawner, Foliage);
}

void FVoxelLegacySpawnersModule::Convert_UVoxelSpawnerGroup_UVoxelFoliage(const UVoxelSpawnerGroup& SpawnerGroup, UVoxelFoliage& Foliage) const
{
	Foliage.Meshes.Reset();
	for (const FVoxelSpawnerGroupChild& Child : SpawnerGroup.Children)
	{
		if (auto* Spawner = Cast<UVoxelMeshSpawnerBase>(Child.Spawner))
		{
			Convert_UVoxelMeshSpawnerBase_UVoxelFoliage(*Spawner, Foliage);
		}

		if (auto* Spawner = Cast<UVoxelMeshSpawner>(Child.Spawner))
		{
			Foliage.Meshes.Add({ Spawner->Mesh, {}, Child.Probability });
		}
		if (auto* Spawner = Cast<UVoxelMeshSpawnerGroup>(Child.Spawner))
		{
			for (UStaticMesh* Mesh : Spawner->Meshes)
			{
				Foliage.Meshes.Add({ Mesh, {}, Child.Probability / Spawner->Meshes.Num() });
			}
		}
	}
}

void FVoxelLegacySpawnersModule::Convert_UVoxelMeshSpawnerBase_UVoxelFoliage(const UVoxelMeshSpawnerBase& MeshSpawner, UVoxelFoliage& Foliage) const
{
	if (MeshSpawner.ActorSettings.ActorClass)
	{
		Foliage.ActorClass = MeshSpawner.ActorSettings.ActorClass;
	}
	else
	{
		Foliage.ActorClass = LoadClass<AVoxelFoliageActor>(nullptr, TEXT("/Voxel/Foliage/VoxelTreeFoliageActor.VoxelTreeFoliageActor_C"));
	}
	ensure(Foliage.ActorClass);
	
	Foliage.InstanceSettings = MeshSpawner.InstancedMeshSettings;
	Foliage.FloatingDetectionOffset = MeshSpawner.FloatingDetectionOffset;

	Foliage.GroundSlopeAngle = MeshSpawner.GroundSlopeAngle;
	Foliage.bEnableHeightRestriction = MeshSpawner.bEnableHeightRestriction;
	Foliage.HeightRestriction = MeshSpawner.HeightRestriction;
	Foliage.HeightRestrictionFalloff = MeshSpawner.HeightRestrictionFalloff;

	Foliage.Scaling.Scaling = EVoxelFoliageScaling(MeshSpawner.Scaling.Scaling);
	Foliage.Scaling.ScaleX = MeshSpawner.Scaling.ScaleX;
	Foliage.Scaling.ScaleY = MeshSpawner.Scaling.ScaleY;
	Foliage.Scaling.ScaleZ = MeshSpawner.Scaling.ScaleZ;

	Foliage.RotationAlignment = EVoxelFoliageRotation(MeshSpawner.RotationAlignment);
	
	Foliage.bRandomYaw = MeshSpawner.bRandomYaw;
	Foliage.RandomPitchAngle = MeshSpawner.RandomPitchAngle;
	Foliage.LocalPositionOffset = MeshSpawner.LocalPositionOffset;
	Foliage.LocalRotationOffset = MeshSpawner.LocalRotationOffset;
	Foliage.GlobalPositionOffset = MeshSpawner.GlobalPositionOffset;
}

void FVoxelLegacySpawnersModule::Convert_UVoxelSpawnerConfig_UVoxelFoliageCollection(const UVoxelSpawnerConfig& SpawnerConfig, UVoxelFoliageCollection& FoliageCollection) const
{
	for (const FVoxelSpawnerConfigSpawner& Spawner : SpawnerConfig.Spawners)
	{
		if (!Spawner.Spawner)
		{
			continue;
		}
		
		auto* Foliage = Cast<UVoxelFoliage>(Convert(Spawner.Spawner));
		if (!ensure(Foliage))
		{
			continue;
		}
		FoliageCollection.Foliages.Add(Foliage);
		// Another collection might have used them
		Foliage->Densities.Reset();

		FVoxelFoliageSpawnSettings& SpawnSettings = Foliage->SpawnSettings;

		SpawnSettings.SpawnType = EVoxelFoliageSpawnType(Spawner.SpawnerType);

		Convert_FVoxelSpawnerDensity_FVoxelFoliageDensity(Spawner.Density, Foliage->Densities.Emplace_GetRef());

		if (Spawner.SpawnerType == EVoxelSpawnerType::Ray)
		{
			Convert_FVoxelSpawnerDensity_FVoxelFoliageDensity(Spawner.DensityMultiplier_RayOnly, Foliage->Densities.Emplace_GetRef());
		}
		
		SpawnSettings.HeightGraphOutputName_HeightOnly.Name = Spawner.HeightGraphOutputName_HeightOnly.Name;
		SpawnSettings.ChunkSize = FVoxelUtilities::GetSizeFromDepth<RENDER_CHUNK_SIZE>(Spawner.LOD);
		SpawnSettings.GenerationDistance = FVoxelDistance::Voxels(Spawner.GenerationDistanceInChunks * SpawnSettings.ChunkSize);
		SpawnSettings.bInfiniteGenerationDistance = Spawner.bInfiniteGenerationDistance;
		Foliage->bSave = Spawner.bSave;
		Foliage->bDoNotDespawn = Spawner.bDoNotDespawn;
		SpawnSettings.RandomGenerator = EVoxelFoliageRandomGenerator(Spawner.RandomGenerator);
		Foliage->Guid = Spawner.Guid;
		if (!Foliage->Guid.IsValid())
		{
			Foliage->Guid = FGuid::NewGuid();
		}
		SpawnSettings.bCheckIfFloating_HeightOnly = Spawner.bCheckIfFloating_HeightOnly;
		SpawnSettings.bCheckIfCovered_HeightOnly = Spawner.bCheckIfCovered_HeightOnly;
	}
}

void FVoxelLegacySpawnersModule::Convert_FVoxelSpawnerDensity_FVoxelFoliageDensity(const FVoxelSpawnerDensity& OldDensity, FVoxelFoliageDensity& NewDensity) const
{
	NewDensity.Type = EVoxelFoliageDensityType(OldDensity.Type);
	NewDensity.Constant = OldDensity.Constant;
	NewDensity.GeneratorOutputName.Name = OldDensity.GeneratorOutputName.Name;
	NewDensity.RGBAChannel = OldDensity.RGBAChannel;
	NewDensity.UVChannel = OldDensity.UVChannel;
	NewDensity.UVAxis = EVoxelUVAxis(OldDensity.UVAxis);
	NewDensity.FiveWayBlendChannel = OldDensity.FiveWayBlendChannel;
	NewDensity.SingleIndexChannels = OldDensity.SingleIndexChannels;
	NewDensity.MultiIndexChannels = OldDensity.MultiIndexChannels;
	NewDensity.bInvertDensity = OldDensity.Transform == EVoxelSpawnerDensityTransform::OneMinus;
}