// Copyright 2020 Phyronnaz

#include "VoxelDataAssetEditorManager.h"
#include "VoxelUtilities/VoxelMathUtilities.h"
#include "VoxelMinimal.h"
#include "VoxelWorld.h"
#include "VoxelDebug/VoxelDebugUtilities.h"
#include "VoxelData/VoxelDataIncludes.h"
#include "VoxelTools/VoxelDataTools.h"
#include "VoxelTools/VoxelBlueprintLibrary.h"
#include "VoxelAssets/VoxelDataAsset.h"
#include "VoxelAssets/VoxelDataAssetData.inl"
#include "VoxelUtilities/VoxelConfigUtilities.h"
#include "VoxelUtilities/VoxelExampleUtilities.h"
#include "VoxelRender/MaterialCollections/VoxelMaterialCollectionBase.h"
#include "VoxelSettings.h"
#include "VoxelFeedbackContext.h"

#include "DrawDebugHelpers.h"
#include "PreviewScene.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"

FVoxelDataAssetEditorManager::FVoxelDataAssetEditorManager(UVoxelDataAsset* DataAsset, FPreviewScene& PreviewScene)
	: DataAsset(DataAsset)
{
	check(DataAsset);

	if (!DataAsset->VoxelWorldTemplate)
	{
		auto* NewWorld = NewObject<AVoxelWorld>(DataAsset);
		NewWorld->MaterialCollection = FVoxelExampleUtilities::LoadExampleObject<UVoxelMaterialCollectionBase>(TEXT("/Voxel/Examples/Materials/Quixel/MC_Quixel"));
		NewWorld->VoxelMaterial = FVoxelExampleUtilities::LoadExampleObject<UMaterialInterface>(TEXT("/Voxel/Examples/Materials/Quixel/MI_VoxelQuixel_FiveWayBlend_Inst"));

		FVoxelConfigUtilities::LoadConfig(NewWorld, "VoxelDataAssetEditor.DefaultVoxelWorld");
		
		DataAsset->VoxelWorldTemplate = NewWorld;
		DataAsset->MarkPackageDirty();
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Template = DataAsset->VoxelWorldTemplate;
	World = PreviewScene.GetWorld()->SpawnActor<AVoxelWorld>(SpawnParameters);

	CreateWorld();
}

FVoxelDataAssetEditorManager::~FVoxelDataAssetEditorManager()
{
	World->GetData().ClearDirtyFlag(); // Avoid annoying save popup from the voxel world
	World->DestroyWorld();

	check(DataAsset->VoxelWorldTemplate);
	DataAsset->VoxelWorldTemplate->ReinitializeProperties(World);
}

void FVoxelDataAssetEditorManager::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(World);
	check(World);
}

AVoxelWorld& FVoxelDataAssetEditorManager::GetVoxelWorld() const
{
	check(World);
	return *World;
}

void FVoxelDataAssetEditorManager::Save(bool bShowDebug)
{
	FVoxelScopedSlowTask Progress(6);

	auto& Data = World->GetData();
	
	Progress.EnterProgressFrame(1, VOXEL_LOCTEXT("Rounding voxels"));
	if (GetDefault<UVoxelSettings>()->bRoundBeforeSaving)
	{
		UVoxelDataTools::RoundVoxels(World, FVoxelIntBox::Infinite);
	}

	Progress.EnterProgressFrame(1, VOXEL_LOCTEXT("Finding dirty voxels"));
	FVoxelIntBoxWithValidity OptionalDirtyBounds;
	bool bHasMaterials = false;
	{
		FVoxelReadScopeLock Lock(Data, FVoxelIntBox::Infinite, "data asset save");
		FVoxelOctreeUtilities::IterateAllLeaves(Data.GetOctree(), [&](FVoxelDataOctreeLeaf& Leaf)
		{
			bHasMaterials |= Leaf.Materials.IsDirty();
			if (Leaf.Values.IsDirty() || Leaf.Materials.IsDirty())
			{
				OptionalDirtyBounds += Leaf.GetBounds();
			}
		});
	}
	
	// Should always have at least one dirty voxel, else it would mean that the original data asset had a size of 0 which is invalid
	if (!ensure(OptionalDirtyBounds.IsValid())) return;

	const auto DirtyBounds = OptionalDirtyBounds.GetBox();
	
	const bool bSubtractiveAsset = DataAsset->bSubtractiveAsset;

	Progress.EnterProgressFrame(1, VOXEL_LOCTEXT("Finding voxels to save"));
	FVoxelIntBox BoundsToSave;
	TArray<FIntVector> PointsAlone;
	{
		FVoxelReadScopeLock Lock(Data, DirtyBounds, "data asset save");
		FVoxelConstDataAccelerator OctreeAccelerator(Data, DirtyBounds);
		DirtyBounds.Iterate([&](int32 X, int32 Y, int32 Z)
		{
			const FVoxelValue Value = OctreeAccelerator.Get<FVoxelValue>(X, Y, Z, 0);
			if ((bSubtractiveAsset && !Value.IsTotallyFull()) || (!bSubtractiveAsset && !Value.IsTotallyEmpty()))
			{
				if (!BoundsToSave.IsValid())
				{
					BoundsToSave = FVoxelIntBox(X, Y, Z);
				}
				else if (!BoundsToSave.Contains(X, Y, Z))
				{
					BoundsToSave = BoundsToSave + FIntVector(X, Y, Z);
					PointsAlone.Emplace(X, Y, Z);
				}
			}
		});
	}

	const FIntVector PositionOffset = BoundsToSave.Min;
	const FIntVector Size = BoundsToSave.Size();
	
	const auto AssetData = MakeVoxelShared<FVoxelDataAssetData>();
	AssetData->SetSize(Size, bHasMaterials);

	{
		FVoxelReadScopeLock Lock(Data, BoundsToSave, "Data Asset Save");
		
		Progress.EnterProgressFrame(1, VOXEL_LOCTEXT("Copying values"));
		{
			TVoxelQueryZone<FVoxelValue> QueryZone(BoundsToSave, AssetData->GetRawValues());
			Data.Get<FVoxelValue>(QueryZone, 0);
		}
			
		Progress.EnterProgressFrame(1, VOXEL_LOCTEXT("Copying materials"));
		if (bHasMaterials)
		{
			TVoxelQueryZone<FVoxelMaterial> QueryZone(BoundsToSave, AssetData->GetRawMaterials());
			Data.Get<FVoxelMaterial>(QueryZone, 0);
		}
	}

	Progress.EnterProgressFrame(1, VOXEL_LOCTEXT("Compressing"));
	DataAsset->PositionOffset = PositionOffset;
	DataAsset->SetData(AssetData);

	Data.ClearDirtyFlag();

	LOG_VOXEL(Log, TEXT("Data asset saved. Has materials: %s"), bHasMaterials ? TEXT("yes") : TEXT("no"));

	if (bShowDebug)
	{
		UVoxelDebugUtilities::DrawDebugIntBox(World, BoundsToSave, 10, 100, FColor::Red);
		for (auto& Point : PointsAlone)
		{
			DrawDebugPoint(World->GetWorld(), World->LocalToGlobal(Point), 10, FColor::Magenta, false, 10);
		}
	}
}

void FVoxelDataAssetEditorManager::RecreateWorld()
{
	World->DestroyWorld();
	CreateWorld();
}

bool FVoxelDataAssetEditorManager::IsDirty() const
{
	return ensure(World->IsCreated()) && World->GetData().IsDirty();
}

void FVoxelDataAssetEditorManager::CreateWorld()
{
	check(!World->IsCreated());
	World->SetGeneratorObject(DataAsset);
	World->CreateInEditor();
	UVoxelDataTools::SetBoxAsDirty(World, DataAsset->GetBounds(), true, DataAsset->GetData()->HasMaterials());
}