// Copyright 2019 Phyronnaz

#include "VoxelSerializationUtilities.h"
#include "VoxelMaterial.h"
#include "VoxelValue.h"
#include "VoxelCustomVersion.h"
#include "VoxelVersionsFixup.h"
#include "VoxelGlobals.h"

DECLARE_CYCLE_STAT(TEXT("FVoxelSerializationUtilities::AddMaterialsToArchive"), STAT_FVoxelSerializationUtilities_AddMaterialsToArchive, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelSerializationUtilities::GetMaterialsFromArchive"), STAT_FVoxelSerializationUtilities_GetMaterialsFromArchive, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelSerializationUtilities::CompressData"), STAT_FVoxelSerializationUtilities_CompressData, STATGROUP_Voxel);
DECLARE_CYCLE_STAT(TEXT("FVoxelSerializationUtilities::DecompressData"), STAT_FVoxelSerializationUtilities_DecompressData, STATGROUP_Voxel);

void FVoxelSerializationUtilities::AddMaterialsToArchive(TArray<FVoxelMaterial>& Materials, FArchive& Archive)
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelSerializationUtilities_AddMaterialsToArchive);

	if (!ensureAlwaysMsgf(Archive.TotalSize() + Materials.Num() * sizeof(FVoxelMaterial) <= MAX_int32, TEXT("Materials array is too big, won't be saved!")))
	{
		int32 MaterialsSize = 0;
		Archive << MaterialsSize;
		return;
	}
	int32 MaterialsSize = Materials.Num();
	Archive << MaterialsSize;
	Archive.Serialize(Materials.GetData(), MaterialsSize * sizeof(FVoxelMaterial));
}

void FVoxelSerializationUtilities::GetMaterialsFromArchive(TArray<FVoxelMaterial>& Materials, FArchive& Archive, uint32 MaterialConfigFlag, int32 VoxelCustomVersion)
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelSerializationUtilities_GetMaterialsFromArchive);

	if (MaterialConfigFlag == GetVoxelMaterialConfigFlag() && VoxelCustomVersion > FVoxelCustomVersion::BeforeCustomVersionWasAdded)
	{
		int32 MaterialsSize;
		Archive << MaterialsSize;
		Materials.SetNumUninitialized(MaterialsSize);
		Archive.Serialize(Materials.GetData(), MaterialsSize * sizeof(FVoxelMaterial));
	}
	else
	{
		int32 MaterialsSize;
		Archive << MaterialsSize;
		Materials.SetNumUninitialized(MaterialsSize);
		for (int32 I = 0; I < MaterialsSize; I++)
		{
			Materials[I] = FVoxelMaterial::SerializeCompat(Archive, MaterialConfigFlag);
		}
	}
}

void FVoxelSerializationUtilities::SerializeMaterials(FArchive& Archive, TArray<FVoxelMaterial>& Materials, uint32 MaterialConfigFlag, int32 VoxelCustomVersion)
{
	if (Archive.IsLoading())
	{
		GetMaterialsFromArchive(Materials, Archive, MaterialConfigFlag, VoxelCustomVersion);
	}
	else if (Archive.IsSaving())
	{
		AddMaterialsToArchive(Materials, Archive);
	}
}

void FVoxelSerializationUtilities::CompressData(const TArray<uint8>& UncompressedData, TArray<uint8>& CompressedData, ECompressionFlags CompressionFlags /*= (ECompressionFlags)(COMPRESS_ZLIB | COMPRESS_BiasSpeed)*/)
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelSerializationUtilities_CompressData);

	int32 UncompressedSize = UncompressedData.Num();
	int32 CompressedSize = 0;
#if ENGINE_MINOR_VERSION < 22
	CompressedSize = FCompression::CompressMemoryBound(CompressionFlags, UncompressedSize);
#else
	ECompressionFlags NewCompressionFlags = (ECompressionFlags)(CompressionFlags & COMPRESS_OptionsFlagsMask);
	switch (CompressionFlags & COMPRESS_DeprecatedFormatFlagsMask)
	{
	case COMPRESS_ZLIB:
		CompressedSize = FCompression::CompressMemoryBound(NAME_Zlib, UncompressedSize, NewCompressionFlags);
		break;
	case COMPRESS_GZIP:
		CompressedSize = FCompression::CompressMemoryBound(NAME_Gzip, UncompressedSize, NewCompressionFlags);
		break;
	case COMPRESS_Custom:
		CompressedSize = FCompression::CompressMemoryBound(TEXT("Oodle"), UncompressedSize, NewCompressionFlags);
		break;
	}
#endif


	CompressedData.SetNumUninitialized(sizeof(UncompressedSize) + CompressedSize);
	FMemory::Memcpy(CompressedData.GetData(), &UncompressedSize, sizeof(UncompressedSize));
	uint8* CompressionStart = CompressedData.GetData() + sizeof(UncompressedSize);

	bool bSuccess = false;

#if ENGINE_MINOR_VERSION < 22
	bSuccess = FCompression::CompressMemory(CompressionFlags, CompressionStart, CompressedSize, UncompressedData.GetData(), UncompressedData.Num());
#else
	switch (CompressionFlags & COMPRESS_DeprecatedFormatFlagsMask)
	{
	case COMPRESS_ZLIB:
		bSuccess = FCompression::CompressMemory(NAME_Zlib, CompressionStart, CompressedSize, UncompressedData.GetData(), UncompressedData.Num(), NewCompressionFlags);
		break;
	case COMPRESS_GZIP:
		bSuccess = FCompression::CompressMemory(NAME_Gzip, CompressionStart, CompressedSize, UncompressedData.GetData(), UncompressedData.Num(), NewCompressionFlags);
		break;
	case COMPRESS_Custom:
		bSuccess = FCompression::CompressMemory(TEXT("Oodle"), CompressionStart, CompressedSize, UncompressedData.GetData(), UncompressedData.Num(), NewCompressionFlags);
		break;
	}
#endif

	check(bSuccess);

	CompressedData.SetNum(CompressedSize + sizeof(UncompressedSize));
	CompressedData.Add(CompressionFlags);
}

bool FVoxelSerializationUtilities::DecompressData(const TArray<uint8>& CompressedData, TArray<uint8>& UncompressedData)
{
	SCOPE_CYCLE_COUNTER(STAT_FVoxelSerializationUtilities_DecompressData);

	if (CompressedData.Num() == 0)
	{
		return false;
	}

	ECompressionFlags CompressionFlags = (ECompressionFlags)CompressedData.Last();

	int32 UncompressedSize;
	FMemory::Memcpy(&UncompressedSize, CompressedData.GetData(), sizeof(UncompressedSize));
	UncompressedData.SetNum(UncompressedSize);
	const uint8* CompressionStart = CompressedData.GetData() + sizeof(UncompressedSize);
	int32 CompressionSize = CompressedData.Num() - 1 - sizeof(UncompressedSize);

	bool bSuccess = false;

#if ENGINE_MINOR_VERSION < 22
	bSuccess = FCompression::UncompressMemory(CompressionFlags, UncompressedData.GetData(), UncompressedSize, CompressionStart, CompressionSize);
#else
	ECompressionFlags NewCompressionFlags = (ECompressionFlags)(CompressionFlags & COMPRESS_OptionsFlagsMask);
	switch (CompressionFlags & COMPRESS_DeprecatedFormatFlagsMask)
	{
	case COMPRESS_ZLIB:
		bSuccess = FCompression::UncompressMemory(NAME_Zlib, UncompressedData.GetData(), UncompressedSize, CompressionStart, CompressionSize, NewCompressionFlags);
		break;
	case COMPRESS_GZIP:
		bSuccess = FCompression::UncompressMemory(NAME_Gzip, UncompressedData.GetData(), UncompressedSize, CompressionStart, CompressionSize, NewCompressionFlags);
		break;
	case COMPRESS_Custom:
		bSuccess = FCompression::UncompressMemory(TEXT("Oodle"), UncompressedData.GetData(), UncompressedSize, CompressionStart, CompressionSize, NewCompressionFlags);
		break;
	}
#endif

	return bSuccess;
}
