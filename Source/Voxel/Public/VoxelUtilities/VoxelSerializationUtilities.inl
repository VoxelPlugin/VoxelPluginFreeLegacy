// Copyright 2021 Phyronnaz

#pragma once

#include "VoxelUtilities/VoxelSerializationUtilities.h"

template<typename T>
FORCEINLINE FArchive& operator<<(FArchive& Ar, TVoxelValueImpl<T>& Value)
{
	Ar << Value.GetStorage();
	return Ar;
}

template<typename Array>
void FVoxelSerializationUtilities::SerializeValues(FArchive& Archive, Array& Values, uint32 ValueConfigFlag, FVoxelSerializationVersion::Type VoxelCustomVersion)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	using SizeType = typename TDecay<decltype(Values)>::Type::SizeType;
	
	if (Archive.IsLoading())
	{
		if (VoxelCustomVersion == FVoxelSerializationVersion::BeforeCustomVersionWasAdded)
		{
			TArray<FVoxelValue16> CompatValues;
			Archive << CompatValues;
			Values = FVoxelValueConverter::ConvertValues(TArray64<FVoxelValue16>(CompatValues));
		}
		else if (VoxelCustomVersion < FVoxelSerializationVersion::RemoveEnableVoxelSpawnedActorsEnableVoxelGrass)
		{
			TArray<FVoxelValue16> CompatValues;
			CompatValues.BulkSerialize(Archive);
			for (auto& Value : CompatValues)
			{
				Value.GetStorage() = FVoxelValue16::ClampToStorage(2 * Value.GetStorage());
			}
			Values = FVoxelValueConverter::ConvertValues(TArray64<FVoxelValue16>(CompatValues));
		}
		else if (VoxelCustomVersion < FVoxelSerializationVersion::ValueConfigFlagAndSaveGUIDs)
		{
			TArray<FVoxelValue16> CompatValues;
			CompatValues.BulkSerialize(Archive);
			Values = FVoxelValueConverter::ConvertValues(TArray64<FVoxelValue16>(CompatValues));
		}
		else
		{
			SizeType ValuesSize;
			Archive << ValuesSize;
			
			check(ValueConfigFlag);
			if (ValueConfigFlag & EVoxelValueConfigFlag::OneBitValue)
			{
				check(ValueConfigFlag == EVoxelValueConfigFlag::OneBitValue);
				
				FVoxelBitArray64 CompatValues;
				Archive << CompatValues;
				ensure(CompatValues.Num() == ValuesSize);
				Values = FVoxelValueConverter::ConvertValues(MoveTemp(CompatValues));
			}
			else if (ValueConfigFlag & EVoxelValueConfigFlag::EightBitsValue)
			{
				check(ValueConfigFlag == EVoxelValueConfigFlag::EightBitsValue);
				
				TArray64<FVoxelValue8> CompatValues;
				CompatValues.Empty(ValuesSize);
				CompatValues.SetNumUninitialized(ValuesSize);
				Archive.Serialize(CompatValues.GetData(), ValuesSize * sizeof(FVoxelValue8));
				Values = FVoxelValueConverter::ConvertValues(MoveTemp(CompatValues));
			}
			else
			{
				check(ValueConfigFlag & EVoxelValueConfigFlag::SixteenBitsValue);
				check(ValueConfigFlag == EVoxelValueConfigFlag::SixteenBitsValue);
				
				TArray64<FVoxelValue16> CompatValues;
				CompatValues.Empty(ValuesSize);
				CompatValues.SetNumUninitialized(ValuesSize);
				Archive.Serialize(CompatValues.GetData(), ValuesSize * sizeof(FVoxelValue16));
				Values = FVoxelValueConverter::ConvertValues(MoveTemp(CompatValues));
			}
		}
	}
	else if (Archive.IsSaving())
	{
		SizeType ValuesSize = Values.Num();
		Archive << ValuesSize;
#if ONE_BIT_VOXEL_VALUE
		const bool bIsCorrectType = TIsSame<FVoxelBitArray64, Array>::Value;
		ensure(bIsCorrectType);
		
		Archive << Values;
#else
		Archive.Serialize(Values.GetData(), ValuesSize * sizeof(FVoxelValue));
#endif
	}
}

template<typename TAllocator>
void FVoxelSerializationUtilities::SerializeMaterials(FArchive& Archive, TArray<FVoxelMaterial, TAllocator>& Materials, uint32 MaterialConfigFlag, FVoxelSerializationVersion::Type VoxelCustomVersion)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	using SizeType = typename TAllocator::SizeType;
	static_assert(sizeof(FVoxelMaterial) == FVoxelMaterial::NumChannels, "Serialization below will be broken");

	if (Archive.IsLoading())
	{
		enum ELegacyVoxelMaterialConfigFlag : uint32
		{
			LegacyEnableVoxelColors = 0x01,
			LegacyEnableVoxelSpawnedActors = 0x02,
			LegacyEnableVoxelGrass = 0x04,
			LegacyDisableIndex = 0x10
		};

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

		if (VoxelCustomVersion == FVoxelSerializationVersion::BeforeCustomVersionWasAdded)
		{
			int32 MaterialsSize;
			Archive << MaterialsSize;
			Materials.Empty(MaterialsSize);
			Materials.SetNumUninitialized(MaterialsSize);
			for (int32 I = 0; I < MaterialsSize; I++)
			{
				Materials[I] = LegacySerializeCompat(Archive, MaterialConfigFlag);
			}
		}
		else if (VoxelCustomVersion < FVoxelSerializationVersion::RemoveEnableVoxelSpawnedActorsEnableVoxelGrass)
		{
			int32 MaterialsSize;
			Archive << MaterialsSize;
			Materials.Empty(MaterialsSize);
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
				SizeType MaterialsSize;
				Archive << MaterialsSize;
				Materials.Empty(MaterialsSize);
				Materials.SetNumUninitialized(MaterialsSize);
				Archive.Serialize(Materials.GetData(), MaterialsSize * sizeof(FVoxelMaterial));
			}
			else
			{
				SizeType MaterialsSize;
				Archive << MaterialsSize;
				Materials.Empty(MaterialsSize);
				Materials.SetNumUninitialized(MaterialsSize);
				for (SizeType I = 0; I < MaterialsSize; I++)
				{
					Materials[I] = FVoxelMaterial::SerializeWithCustomConfig(Archive, MaterialConfigFlag);
				}
			}
		}
	}
	else if (Archive.IsSaving())
	{
		SizeType MaterialsSize = Materials.Num();
		Archive << MaterialsSize;
		Archive.Serialize(Materials.GetData(), MaterialsSize * sizeof(FVoxelMaterial));
	}
}