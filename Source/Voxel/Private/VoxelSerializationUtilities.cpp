// Copyright 2020 Phyronnaz

#include "VoxelSerializationUtilities.h"
#include "VoxelMaterial.h"
#include "VoxelCustomVersion.h"
#include "VoxelGlobals.h"

template<typename T>
FORCEINLINE FArchive& operator<<(FArchive& Ar, TVoxelValueImpl<T>& Value)
{
	Ar << Value.GetStorage();
	return Ar;
}

void FVoxelSerializationUtilities::SerializeValues(FArchive& Archive, TArray<FVoxelValue>& Values, uint32 ValueConfigFlag, int32 VoxelCustomVersion)
{
	VOXEL_FUNCTION_COUNTER();

	if (Archive.IsLoading())
	{
		if (VoxelCustomVersion == FVoxelCustomVersion::BeforeCustomVersionWasAdded)
		{
			TArray<FVoxelValue16> CompatValues;
			Archive << CompatValues;
			Values = FVoxelValueConverter::ConvertValues(MoveTemp(CompatValues));
		}
		else if (VoxelCustomVersion < FVoxelCustomVersion::RemoveEnableVoxelSpawnedActorsEnableVoxelGrass)
		{
			TArray<FVoxelValue16> CompatValues;
			CompatValues.BulkSerialize(Archive);
			for (auto& Value : CompatValues)
			{
				Value.GetStorage() = FVoxelValue16::ClampToStorage(2 * Value.GetStorage());
			}
			Values = FVoxelValueConverter::ConvertValues(MoveTemp(CompatValues));
		}
		else if (VoxelCustomVersion < FVoxelCustomVersion::ValueConfigFlagAndSaveGUIDs)
		{
			TArray<FVoxelValue16> CompatValues;
			CompatValues.BulkSerialize(Archive);
			Values = FVoxelValueConverter::ConvertValues(MoveTemp(CompatValues));
		}
		else
		{
			int32 ValuesSize;
			Archive << ValuesSize;
			
			check(ValueConfigFlag);
			if (ValueConfigFlag & EVoxelValueConfigFlag::EightBitsValue)
			{
				check(!(ValueConfigFlag & EVoxelValueConfigFlag::SixteenBitsValue));
				TArray<FVoxelValue8> CompatValues;
				CompatValues.SetNumUninitialized(ValuesSize);
				Archive.Serialize(CompatValues.GetData(), ValuesSize * sizeof(FVoxelValue8));
				Values = FVoxelValueConverter::ConvertValues(MoveTemp(CompatValues));
			}
			else
			{
				check(ValueConfigFlag & EVoxelValueConfigFlag::SixteenBitsValue);
				TArray<FVoxelValue16> CompatValues;
				CompatValues.SetNumUninitialized(ValuesSize);
				Archive.Serialize(CompatValues.GetData(), ValuesSize * sizeof(FVoxelValue16));
				Values = FVoxelValueConverter::ConvertValues(MoveTemp(CompatValues));
			}
		}
	}
	else if (Archive.IsSaving())
	{
		int32 ValuesSize = Values.Num();
		Archive << ValuesSize;
		Archive.Serialize(Values.GetData(), ValuesSize * sizeof(FVoxelValue));
	}
}

void FVoxelSerializationUtilities::SerializeMaterials(FArchive& Archive, TArray<FVoxelMaterial>& Materials, uint32 MaterialConfigFlag, int32 VoxelCustomVersion)
{
	VOXEL_FUNCTION_COUNTER();

	if (Archive.IsLoading())
	{
		enum ELegacyVoxelMaterialConfigFlag : uint32
		{
			LegacyEnableVoxelColors = 0x01,
			LegacyEnableVoxelSpawnedActors = 0x02,
			LegacyEnableVoxelGrass = 0x04,
			LegacyDisableIndex = 0x10
		};
		
		constexpr uint32 LegacyVoxelMaterialConfigFlag =
			LegacyEnableVoxelColors * 1 +
			LegacyEnableVoxelSpawnedActors * 0 +
			LegacyEnableVoxelGrass * 0 +
			LegacyDisableIndex * 0;
		
		const auto LegacySerializeCompat = [](FArchive& Ar, uint32 ConfigFlags)
		{
			check(Ar.IsLoading());

			uint8 Index = 0;
			uint8 R = 0;
			uint8 G = 0;
			uint8 B = 0;
			uint8 VoxelActor = 0;
			uint8 VoxelGrass = 0;

			if (!(ConfigFlags & LegacyDisableIndex))
			{
				Ar << Index;
			}
			if (ConfigFlags & LegacyEnableVoxelColors)
			{
				Ar << R;
				Ar << G;
				Ar << B;
			}
			if (ConfigFlags & LegacyEnableVoxelSpawnedActors)
			{
				Ar << VoxelActor;
			}
			if (ConfigFlags & LegacyEnableVoxelGrass)
			{
				Ar << VoxelGrass;
			}

			FVoxelMaterial Material(ForceInit);
			Material.SetA(Index);
			Material.SetR(R);
			Material.SetG(G);
			Material.SetB(B);

			return Material;
		};
		
		if (VoxelCustomVersion == FVoxelCustomVersion::BeforeCustomVersionWasAdded)
		{
			int32 MaterialsSize;
			Archive << MaterialsSize;
			Materials.SetNumUninitialized(MaterialsSize);
			for (int32 I = 0; I < MaterialsSize; I++)
			{
				Materials[I] = LegacySerializeCompat(Archive, MaterialConfigFlag);
			}
		}
		else if (VoxelCustomVersion < FVoxelCustomVersion::RemoveEnableVoxelSpawnedActorsEnableVoxelGrass)
		{
			int32 MaterialsSize;
			Archive << MaterialsSize;
			Materials.SetNumUninitialized(MaterialsSize);
			for (int32 I = 0; I < MaterialsSize; I++)
			{
				Materials[I] = LegacySerializeCompat(Archive, MaterialConfigFlag);
			}
		}
		else
		{
			if (MaterialConfigFlag == GVoxelMaterialConfigFlag)
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
					Materials[I] = FVoxelMaterial::SerializeWithCustomConfig(Archive, MaterialConfigFlag);
				}
			}
		}
	}
	else if (Archive.IsSaving())
	{
		int32 MaterialsSize = Materials.Num();
		Archive << MaterialsSize;
		Archive.Serialize(Materials.GetData(), MaterialsSize * sizeof(FVoxelMaterial));
	}
}

void FVoxelSerializationUtilities::CompressData(const uint8* const UncompressedData, const int32 UncompressedDataNum, TArray<uint8>& CompressedData, ECompressionFlags CompressionFlags /*= (ECompressionFlags)(COMPRESS_ZLIB | COMPRESS_BiasSpeed)*/)
{
	VOXEL_FUNCTION_COUNTER();

	int32 UncompressedSize = UncompressedDataNum;
	int32 CompressedSize = 0;
	const ECompressionFlags NewCompressionFlags = (ECompressionFlags)(CompressionFlags & COMPRESS_OptionsFlagsMask);
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
	default:
		ensure(false);
	}

	CompressedData.SetNumUninitialized(sizeof(UncompressedSize) + CompressedSize);
	FMemory::Memcpy(CompressedData.GetData(), &UncompressedSize, sizeof(UncompressedSize));
	uint8* CompressionStart = CompressedData.GetData() + sizeof(UncompressedSize);

	bool bSuccess = false;
	switch (CompressionFlags & COMPRESS_DeprecatedFormatFlagsMask)
	{
	case COMPRESS_ZLIB:
		bSuccess = FCompression::CompressMemory(NAME_Zlib, CompressionStart, CompressedSize, UncompressedData, UncompressedDataNum, NewCompressionFlags);
		break;
	case COMPRESS_GZIP:
		bSuccess = FCompression::CompressMemory(NAME_Gzip, CompressionStart, CompressedSize, UncompressedData, UncompressedDataNum, NewCompressionFlags);
		break;
	case COMPRESS_Custom:
		bSuccess = FCompression::CompressMemory(TEXT("Oodle"), CompressionStart, CompressedSize, UncompressedData, UncompressedDataNum, NewCompressionFlags);
		break;
	default:
		ensure(false);
	}
	check(bSuccess);

	CompressedData.SetNum(CompressedSize + sizeof(UncompressedSize));
	CompressedData.Add(CompressionFlags);
}

bool FVoxelSerializationUtilities::DecompressData(const TArray<uint8>& CompressedData, TArray<uint8>& UncompressedData)
{
	VOXEL_FUNCTION_COUNTER();

	if (CompressedData.Num() == 0)
	{
		return false;
	}

	const ECompressionFlags CompressionFlags = ECompressionFlags(CompressedData.Last());

	int32 UncompressedSize;
	FMemory::Memcpy(&UncompressedSize, CompressedData.GetData(), sizeof(UncompressedSize));
	UncompressedData.SetNum(UncompressedSize);
	const uint8* CompressionStart = CompressedData.GetData() + sizeof(UncompressedSize);
	const int32 CompressionSize = CompressedData.Num() - 1 - sizeof(UncompressedSize);

	bool bSuccess = false;
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
	default:
		ensure(false);
	}

	return bSuccess;
}
