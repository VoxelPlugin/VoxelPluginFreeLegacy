// Copyright 2020 Phyronnaz

#include "VoxelAssets/VoxelDataAssetData.h"
#include "VoxelUtilities/VoxelSerializationUtilities.h"
#include "VoxelFeedbackContext.h"

DEFINE_VOXEL_MEMORY_STAT(STAT_VoxelDataAssetMemory);

void FVoxelDataAssetData::SetSize(const FIntVector& NewSize, bool bCreateMaterials)
{
	VOXEL_FUNCTION_COUNTER();
	check(int64(NewSize.X) * int64(NewSize.Y) * int64(NewSize.Z) < MAX_int32);

	const int32 Num = NewSize.X * NewSize.Y * NewSize.Z;
	
	// Somewhat thread safe
	Values.Empty(Num);
	Values.SetNumUninitialized(Num);
	
	Materials.Empty(bCreateMaterials ? Num : 0);
	Materials.SetNumUninitialized(bCreateMaterials ? Num : 0);

	Size = NewSize;

	ensure(Size.GetMin() > 0);
	ensure(Size.GetMax() > 1); // Else it'll be considered empty
	UpdateStats();
}

void FVoxelDataAssetData::Serialize(FArchive& Ar, uint32 ValueConfigFlag, uint32 MaterialConfigFlag, FVoxelDataAssetDataVersion::Type Version)
{
	VOXEL_FUNCTION_COUNTER();
	
	FVoxelScopedSlowTask Serializing(2.f);
	
	Ar << Size;

	const auto SerializationVersion =
		Version >= FVoxelDataAssetDataVersion::ValueConfigFlagAndSaveGUIDs
		? FVoxelSerializationVersion::ValueConfigFlagAndSaveGUIDs
		: Version >= FVoxelDataAssetDataVersion::RemoveEnableVoxelSpawnedActorsEnableVoxelGrass
		? FVoxelSerializationVersion::RemoveEnableVoxelSpawnedActorsEnableVoxelGrass
		: FVoxelSerializationVersion::BeforeCustomVersionWasAdded;
	
	static_assert(FVoxelSerializationVersion::LatestVersion == FVoxelSerializationVersion::SHARED_StoreMaterialChannelsIndividuallyAndRemoveFoliage, "Need to add a new FVoxelDataAssetDataVersion");

	Serializing.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Serializing values"));
	FVoxelSerializationUtilities::SerializeValues(Ar, Values, ValueConfigFlag, SerializationVersion);

	Serializing.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Serializing materials"));
	FVoxelSerializationUtilities::SerializeMaterials(Ar, Materials, MaterialConfigFlag, SerializationVersion);

	if (Size.X * Size.Y * Size.Z != Values.Num() || (Materials.Num() > 0 && Size.X * Size.Y * Size.Z != Materials.Num()))
	{
		Ar.SetError();
	}

	UpdateStats();
}

void FVoxelDataAssetData::UpdateStats() const
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataAssetMemory, AllocatedSize);
	AllocatedSize = Values.GetAllocatedSize() + Materials.GetAllocatedSize();
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelDataAssetMemory, AllocatedSize);
}