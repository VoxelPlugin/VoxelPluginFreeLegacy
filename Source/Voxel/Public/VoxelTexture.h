// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelEnums.h"
#include "VoxelUtilities/VoxelBaseUtilities.h"
#include "VoxelTexture.generated.h"

class UTexture;
class UTexture2D;

DECLARE_VOXEL_MEMORY_STAT(TEXT("Voxel Texture Memory"), STAT_VoxelTextureMemory, STATGROUP_VoxelMemory, VOXEL_API);

template<>
struct TNumericLimits<FColor> 
{
	typedef FColor NumericType;

	static constexpr NumericType Min()
	{
		return FColor(MIN_uint8, MIN_uint8, MIN_uint8, MIN_uint8);
	}

	static constexpr NumericType Max()
	{
		return FColor(MAX_uint8, MAX_uint8, MAX_uint8, MAX_uint8);
	}

	static constexpr NumericType Lowest()
	{
		return Min();
	}
};

namespace FVoxelUtilities
{
	inline float ComponentMin(float A, float B)
	{
		return FMath::Min(A, B);
	}
	inline float ComponentMax(float A, float B)
	{
		return FMath::Max(A, B);
	}
	
	inline FColor ComponentMin(FColor A, FColor B)
	{
		return FColor(FMath::Min(A.R, B.R), FMath::Min(A.G, B.G), FMath::Min(A.B, B.B), FMath::Min(A.A, B.A));
	}
	inline FColor ComponentMax(FColor A, FColor B)
	{
		return FColor(FMath::Max(A.R, B.R), FMath::Max(A.G, B.G), FMath::Max(A.B, B.B), FMath::Max(A.A, B.A));
	}
}

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

		void SetSize(int32 NewSizeX, int32 NewSizeY)
		{
			check(NewSizeX >= 0 && NewSizeY >= 0);
			SizeX = NewSizeX;
			SizeY = NewSizeY;
			TextureData.Empty(SizeX * SizeY);
			TextureData.SetNumUninitialized(SizeX * SizeY);
			UpdateAllocatedSize();

			Min = TNumericLimits<T>::Max();
			Max = TNumericLimits<T>::Lowest();
		}
		void SetBounds(T NewMin, T NewMax)
		{
			Min = NewMin;
			Max = NewMax;
		}
		
		FORCEINLINE void SetValue(int32 X, int32 Y, T Value)
		{
			checkVoxelSlow(0 <= X && X < SizeX);
			checkVoxelSlow(0 <= Y && Y < SizeY);
			SetValue(X + SizeX * Y, Value);
		}
		FORCEINLINE void SetValue(int32 Index, T Value)
		{
			SetValue_NoBounds(Index, Value);
			Min = FVoxelUtilities::ComponentMin(Min, Value);
			Max = FVoxelUtilities::ComponentMax(Max, Value);
		}
		FORCEINLINE void SetValue_NoBounds(int32 Index, T Value)
		{
			checkVoxelSlow(TextureData.IsValidIndex(Index));
			TextureData.GetData()[Index] = Value;
		}
		
	private:
		int32 SizeX = 1;
		int32 SizeY = 1;
		TArray<T> TextureData = { T{} };
		T Min{};
		T Max{};

		int64 AllocatedSize = 0;
		
		void UpdateAllocatedSize()
		{
			DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelTextureMemory, AllocatedSize);
			AllocatedSize = TextureData.GetAllocatedSize();
			INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelTextureMemory, AllocatedSize);
		}

		friend TVoxelTexture;
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

namespace FVoxelTextureUtilities
{
	VOXEL_API TVoxelTexture<FColor> CreateFromTexture_Color(UTexture* Texture);
	VOXEL_API TVoxelTexture<float> CreateFromTexture_Float(UTexture* Texture, EVoxelRGBA Channel);
	
	VOXEL_API bool CanCreateFromTexture(UTexture* Texture, FString& OutError);
	VOXEL_API void FixTexture(UTexture* Texture);

	VOXEL_API void ClearCache();
	VOXEL_API void ClearCache(UTexture* Texture);

	VOXEL_API void CreateOrUpdateUTexture2D(const TVoxelTexture<float>& Texture, UTexture2D*& InOutTexture);
	VOXEL_API void CreateOrUpdateUTexture2D(const TVoxelTexture<FColor>& Texture, UTexture2D*& InOutTexture);

	VOXEL_API TVoxelTexture<FColor> CreateColorTextureFromFloatTexture(const TVoxelTexture<float>& Texture, EVoxelRGBA Channel, bool bNormalize);

	VOXEL_API TVoxelTexture<float> Normalize(const TVoxelTexture<float>& Texture);
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelFloatTexture
{
	GENERATED_BODY()

	FVoxelFloatTexture() = default;
	FVoxelFloatTexture(const TVoxelTexture<float>& Texture)
		: Texture(Texture)
	{
	}
	
	TVoxelTexture<float> Texture;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelColorTexture
{
	GENERATED_BODY()

	FVoxelColorTexture() = default;
	FVoxelColorTexture(const TVoxelTexture<FColor>& Texture)
		: Texture(Texture)
	{
	}

	TVoxelTexture<FColor> Texture;
};