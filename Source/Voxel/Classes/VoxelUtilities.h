#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.h"
#include "VoxelType.h"

class VOXEL_API FVoxelUtilities
{
public:
	/**
	 * If A and B have the same sign
	 */
	FORCEINLINE static bool HaveSameSign(float A, float B)
	{
		// A > 0: see VoxelPolygonizer convention on positive values
		return (A > 0 && B > 0) || (A <= 0 && B <= 0);
	}

	/**
	 * If Value ~= 1 ignore, if Value > 0 use if same sign, else use
	 */
	FORCEINLINE static EVoxelValueType GetValueTypeFromValue(float Value)
	{
		if (Value > 1 - KINDA_SMALL_NUMBER)
		{
			return EVoxelValueType::IgnoreValue;
		}
		else if (Value > 0)
		{
			return EVoxelValueType::UseValueIfSameSign;
		}
		else
		{
			return EVoxelValueType::UseValue;
		}
	}
	
	/**
	 * If Value ~= 1 ignore material else use
	 */
	FORCEINLINE static EVoxelMaterialType GetMaterialTypeFromValue(float Value)
	{
		if (Value <= 0)
		{
			return EVoxelMaterialType::UseMaterial;
		}
		else
		{
			return EVoxelMaterialType::IgnoreMaterial;
		}
	}

	/**
	 * If Value ~= 1 ignore, if Value > 0 use if same sign, else use
	 * If Value ~= 1 ignore material else use
	 */
	FORCEINLINE static FVoxelType GetVoxelTypeFromValue(float Value)
	{
		return FVoxelType(GetValueTypeFromValue(Value), GetMaterialTypeFromValue(Value));
	}

	/**
	 * Compress the materials using Rule Length Encoding
	 * @param	InData	The data to compress
	 * @return	OutData	The compressed data
	 */
	static void CompressRLE(const TArray<FVoxelMaterial>& InData, TArray<uint8>& OutData)
	{
		OutData.Reset();
		int32 StartIndex = 0;
		while (StartIndex < InData.Num())
		{
			const FVoxelMaterial& Material = InData[StartIndex];
			int32 EndIndex = StartIndex;
			while (EndIndex < InData.Num() && InData[EndIndex] == Material && EndIndex - StartIndex < 0xFFFF - 1)
			{
				EndIndex++;
			}
			uint16 Size = EndIndex - StartIndex;
			OutData.Add(Size & 0xFF);
			OutData.Add(Size >> 8);
			OutData.Add(Material.Index1);
			OutData.Add(Material.Index2);
			OutData.Add(Material.Alpha);
			OutData.Add(Material.VoxelActor);

			StartIndex = EndIndex;
		}
		check(StartIndex == InData.Num());
	}

	/**
	 * Decompress the materials compressed by Rule Length Encoding
	 * @param	InData	The compressed data
	 * @return	OutData	The decompressed data
	 */
	static void DecompressRLE(const TArray<uint8>& InData, TArray<FVoxelMaterial>& OutData)
	{
		OutData.Reset();
		int32 StartIndex = 0;
		while (StartIndex < InData.Num())
		{
			const uint16 Size = (uint16)InData[StartIndex] | (((uint16)InData[StartIndex + 1]) << 8);
			const FVoxelMaterial Material(InData[StartIndex + 2], InData[StartIndex + 3], InData[StartIndex + 4], InData[StartIndex + 5]);
			int32 CopyStart = OutData.Num();
			OutData.AddUninitialized(Size);
			for (int i = 0; i < Size; i++)
			{
				OutData[CopyStart + i] = Material;
			}
			StartIndex += 6;
		}
		check(StartIndex == InData.Num());
	}

	/**
	 * Compress the values using Rule Length Encoding
	 * @param	InData	The data to compress
	 * @return	OutData	The compressed data
	 */
	static void CompressRLE(const TArray<float>& InData, TArray<uint8>& OutData)
	{
		OutData.Reset();
		union
		{
			float f;
			uint8 b[4];
		} U;

		int32 StartIndex = 0;
		while (StartIndex < InData.Num())
		{
			const float& Value = InData[StartIndex];
			int32 EndIndex = StartIndex;
			while (EndIndex < InData.Num() && FMath::Abs(InData[EndIndex] - Value) < SMALL_NUMBER && EndIndex - StartIndex < 0xFFFF - 1)
			{
				EndIndex++;
			}
			uint16 Size = EndIndex - StartIndex;
			OutData.Add(Size & 0xFF);
			OutData.Add(Size >> 8);
			U.f = Value;
			OutData.Add(U.b[0]);
			OutData.Add(U.b[1]);
			OutData.Add(U.b[2]);
			OutData.Add(U.b[3]);

			StartIndex = EndIndex;
		}
		check(StartIndex == InData.Num());
	}

	/**
	 * Decompress the values compressed by Rule Length Encoding
	 * @param	InData	The compressed data
	 * @return	OutData	The decompressed data
	 */
	static void DecompressRLE(const TArray<uint8>& InData, TArray<float>& OutData)
	{
		OutData.Reset();
		union
		{
			float f;
			uint8 b[4];
		} U;

		int32 StartIndex = 0;
		while (StartIndex < InData.Num())
		{
			const uint16 Size = (uint16)InData[StartIndex] | (((uint16)InData[StartIndex + 1]) << 8);
			U.b[0] = InData[StartIndex + 2];
			U.b[1] = InData[StartIndex + 3];
			U.b[2] = InData[StartIndex + 4];
			U.b[3] = InData[StartIndex + 5];
			const float Value = U.f;

			int32 CopyStart = OutData.Num();
			OutData.AddUninitialized(Size);
			for (int i = 0; i < Size; i++)
			{
				OutData[CopyStart + i] = Value;
			}
			StartIndex += 6;
		}
		check(StartIndex == InData.Num());
	}

	/**
	 * Compress the voxel types using Rule Length Encoding
	 * @param	InData	The data to compress
	 * @return	OutData	The compressed data
	 */
	static void CompressRLE(const TArray<uint8>& InData, TArray<uint8>& OutData)
	{
		OutData.Reset();
		int32 StartIndex = 0;
		while (StartIndex < InData.Num())
		{
			const uint8& Value = InData[StartIndex];
			int32 EndIndex = StartIndex;
			while (EndIndex < InData.Num() && FMath::Abs(InData[EndIndex] - Value) < SMALL_NUMBER && EndIndex - StartIndex < 0xFFFF - 1)
			{
				EndIndex++;
			}
			uint16 Size = EndIndex - StartIndex;
			OutData.Add(Size & 0xFF);
			OutData.Add(Size >> 8);
			OutData.Add(Value);

			StartIndex = EndIndex;
		}
		check(StartIndex == InData.Num());
	}

	/**
	 * Decompress the voxel types compressed by Rule Length Encoding
	 * @param	InData	The compressed data
	 * @return	OutData	The decompressed data
	 */
	static void DecompressRLE(const TArray<uint8>& InData, TArray<uint8>& OutData)
	{
		OutData.Reset();
		int32 StartIndex = 0;
		while (StartIndex < InData.Num())
		{
			const uint16 Size = (uint16)InData[StartIndex] | (((uint16)InData[StartIndex + 1]) << 8);
			const uint8 Value = InData[StartIndex + 2];

			int32 CopyStart = OutData.Num();
			OutData.AddUninitialized(Size);
			for (int i = 0; i < Size; i++)
			{
				OutData[CopyStart + i] = Value;
			}
			StartIndex += 3;
		}
		check(StartIndex == InData.Num());
	}

	/**
	 * Used to check that compression is working
	 */
	static void TestRLE()
	{
		// Materials
		{
			TArray<FVoxelMaterial> Materials;
			for (int i = 0; i < 10000; i++)
			{
				if (FMath::RandBool() && Materials.Num() > 0)
				{
					Materials.Add(FVoxelMaterial(Materials[Materials.Num() - 1]));
				}
				else
				{
					FVoxelMaterial Random(FMath::RandRange(0, 255), FMath::RandRange(0, 255), FMath::RandRange(0, 255), FMath::RandRange(0, 255));
					Materials.Add(Random);
				}
			}

			TArray<uint8> Data;
			CompressRLE(Materials, Data);
			TArray<FVoxelMaterial> NewMaterials;
			DecompressRLE(Data, NewMaterials);

			check(Materials.Num() == NewMaterials.Num());
			for (int i = 0; i < NewMaterials.Num(); i++)
			{
				check(Materials[i] == NewMaterials[i]);
			}
		}

		// Floats
		{
			TArray<float> Values;
			for (int i = 0; i < 10000; i++)
			{
				if (FMath::RandBool() && Values.Num() > 0)
				{
					float Prev = Values[Values.Num() - 1];
					Values.Add(Prev);
				}
				else
				{
					Values.Add(FMath::FRand());
				}
			}

			TArray<uint8> Data;
			CompressRLE(Values, Data);
			TArray<float> NewValues;
			DecompressRLE(Data, NewValues);

			check(Values.Num() == NewValues.Num());
			for (int i = 0; i < NewValues.Num(); i++)
			{
				check(Values[i] == NewValues[i]);
			}
		}

		// uint8
		{
			TArray<uint8> Values;
			for (int i = 0; i < 10000; i++)
			{
				if (FMath::RandBool() && Values.Num() > 0)
				{
					Values.Add(float(Values[Values.Num() - 1]));
				}
				else
				{
					Values.Add(FMath::RandRange(0, 255));
				}
			}

			TArray<uint8> Data;
			CompressRLE(Values, Data);
			TArray<uint8> NewValues;
			DecompressRLE(Data, NewValues);

			check(Values.Num() == NewValues.Num());
			for (int i = 0; i < NewValues.Num(); i++)
			{
				check(Values[i] == NewValues[i]);
			}
		}
	}
};

