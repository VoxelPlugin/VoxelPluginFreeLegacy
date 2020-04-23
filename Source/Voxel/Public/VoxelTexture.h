// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "VoxelBaseUtilities.h"
#include "VoxelConfigEnums.h"
#include "VoxelTexture.generated.h"

class UTexture;
class UTexture2D;

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Texture Memory"), STAT_VoxelTextureMemory, STATGROUP_VoxelMemory, VOXEL_API);

template<typename T>
struct TVoxelTexture
{
	inline int32 GetSizeX() const
	{
		return DataPtr->SizeX;
	}
	inline int32 GetSizeY() const
	{
		return DataPtr->SizeY;
	}
	inline const TArray<T>& GetTextureData() const
	{
		return DataPtr->TextureData;
	}
	inline T GetMin() const
	{
		return DataPtr->Min;
	}
	inline T GetMax() const
	{
		return DataPtr->Max;
	}
	
	inline T SampleRaw(int32 X, int32 Y, EVoxelSamplerMode Mode) const
	{
		if (Mode == EVoxelSamplerMode::Clamp)
		{
			X = FMath::Clamp(X, 0, GetSizeX() - 1);
			Y = FMath::Clamp(Y, 0, GetSizeY() - 1);
		}
		else
		{
			X = FVoxelUtilities::PositiveMod(X, GetSizeX());
			Y = FVoxelUtilities::PositiveMod(Y, GetSizeY());
		}
		return GetTextureData()[X + GetSizeX() * Y];
	}
	template<typename U>
	inline U Sample(int32 X, int32 Y, EVoxelSamplerMode Mode) const
	{
		return U(SampleRaw(X, Y, Mode));
	}
	template<typename U>
	inline U Sample(v_flt X, v_flt Y, EVoxelSamplerMode Mode) const
	{
		const int32 MinX = FMath::FloorToInt(X);
		const int32 MinY = FMath::FloorToInt(Y);
		
		const int32 MaxX = FMath::CeilToInt(X);
		const int32 MaxY = FMath::CeilToInt(Y);

		const v_flt AlphaX = X - MinX;
		const v_flt AlphaY = Y - MinY;

		return FVoxelUtilities::BilinearInterpolation<U>(
			Sample<U>(MinX, MinY, Mode),
			Sample<U>(MaxX, MinY, Mode),
			Sample<U>(MinX, MaxY, Mode),
			Sample<U>(MaxX, MaxY, Mode),
			AlphaX,
			AlphaY);
	}

public:
	struct FTextureData
	{
		FTextureData() = default;
		~FTextureData()
		{
			DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelTextureMemory, AllocatedSize);
		}

		inline void SetSize(int32 NewSizeX, int32 NewSizeY)
		{
			check(NewSizeX >= 0 && NewSizeY >= 0);
			SizeX = NewSizeX;
			SizeY = NewSizeY;
			TextureData.SetNumUninitialized(SizeX * SizeY);
			TextureData.Shrink();
			UpdateAllocatedSize();

			Min = TNumericLimits<T>::Max();
			Max = TNumericLimits<T>::Lowest();
		}
		FORCEINLINE void SetValue(int32 X, int32 Y, T Value)
		{
			checkVoxelSlow(0 <= X && X < SizeX);
			checkVoxelSlow(0 <= Y && Y < SizeY);
			SetValue(X + SizeX * Y, Value);
		}
		FORCEINLINE void SetValue(int32 Index, T Value)
		{
			checkVoxelSlow(TextureData.IsValidIndex(Index));
			TextureData.GetData()[Index] = Value;
			Min = FMath::Min(Min, Value);
			Max = FMath::Max(Max, Value);
		}
		inline void UpdateAllocatedSize()
		{
			DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelTextureMemory, AllocatedSize);
			AllocatedSize = TextureData.GetAllocatedSize();
			INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelTextureMemory, AllocatedSize);
		}
	private:
		int32 SizeX = 1;
		int32 SizeY = 1;
		TArray<T> TextureData = { T{} };
		T Min{};
		T Max{};

		int32 AllocatedSize = 0;

		friend TVoxelTexture;
		friend class FVoxelTextureUtilities;
	};
	
	TVoxelTexture()
		: TVoxelTexture(MakeShareable(new FTextureData()))
	{
	}
	explicit TVoxelTexture(const TVoxelSharedRef<const FTextureData>& InDataPtr)
		: DataPtr(InDataPtr)
	{
		check(GetTextureData().Num() == GetSizeX() * GetSizeY());
	}

private:
	TVoxelSharedRef<const FTextureData> DataPtr;
};

// TODO: function to clear render target cache

class VOXEL_API FVoxelTextureUtilities
{
public:
	static TVoxelTexture<FColor> CreateFromTexture_Color(UTexture* Texture);
	static TVoxelTexture<float> CreateFromTexture_Float(UTexture* Texture, EVoxelRGBA Channel);
	
	static bool CanCreateFromTexture(UTexture* Texture, FString& OutError);
	static void FixTexture(UTexture* Texture);

	static void ClearCache();
	static void ClearCache(UTexture* Texture);

	static void CreateOrUpdateUTexture2D(const TVoxelTexture<float>& Texture, UTexture2D*& InOutTexture);
	static void CreateOrUpdateUTexture2D(const TVoxelTexture<FColor>& Texture, UTexture2D*& InOutTexture);

	static TVoxelTexture<FColor> CreateColorTextureFromFloatTexture(const TVoxelTexture<float>& Texture, EVoxelRGBA Channel, bool bNormalize);

	static TVoxelTexture<float> Normalize(const TVoxelTexture<float>& Texture);
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelFloatTexture
{
	GENERATED_BODY()

	TVoxelTexture<float> Texture;
};