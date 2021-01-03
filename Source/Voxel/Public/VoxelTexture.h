// Copyright 2021 Phyronnaz

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
	uint64 GetUniqueId() const
	{
		return DataPtr->Id;
	}
	
	int32 GetSizeX() const
	{
		return DataPtr->SizeX;
	}
	int32 GetSizeY() const
	{
		return DataPtr->SizeY;
	}
	const TArray<T>& GetTextureData() const
	{
		return DataPtr->TextureData;
	}
	T GetMin() const
	{
		return DataPtr->Min;
	}
	T GetMax() const
	{
		return DataPtr->Max;
	}
	
	T SampleRaw(int32 X, int32 Y, EVoxelSamplerMode Mode) const
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
	U Sample(int32 X, int32 Y, EVoxelSamplerMode Mode) const
	{
		return U(SampleRaw(X, Y, Mode));
	}
	template<typename U>
	U Sample(v_flt X, v_flt Y, EVoxelSamplerMode Mode) const
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
		const uint64 Id = VOXEL_UNIQUE_ID();
		
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

	friend class FVoxelTextureHelpers;
};

// TODO: function to clear render target cache

class VOXEL_API FVoxelTextureHelpers
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

public:
	template<typename T>
	static bool GetTextureById(uint64 Id, TVoxelTexture<T>& OutTexture);
	template<typename T>
	static void AddTextureToIdCache(const TVoxelTexture<T>& Texture);

	static void ClearIdCache();
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelTextureStructBase
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Voxel")
	uint64 Id = 0;
};

template<typename T>
struct TVoxelTextureStructBase : FVoxelTextureStructBase
{
	TVoxelTextureStructBase() = default;
	TVoxelTextureStructBase(const TVoxelTexture<T>& NewTexture)
	{
		Set(NewTexture);
	}
	
	const TVoxelTexture<T>& Get() const
	{
		if (Texture.GetUniqueId() != Id)
		{
			LoadTexture();
		}
		return Texture;
	}
	void Set(const TVoxelTexture<T>& NewTexture)
	{
		Id = NewTexture.GetUniqueId();
		Texture = NewTexture;
		SaveTexture();
	}

	const TVoxelTexture<T>* operator->() const
	{
		return &Get();
	}
	const TVoxelTexture<T>& operator*() const
	{
		return Get();
	}

private:
	mutable TVoxelTexture<T> Texture;
	
	void SaveTexture() const;
	void LoadTexture() const;
};

template<>
VOXEL_API void TVoxelTextureStructBase<float>::SaveTexture() const;

template<>
VOXEL_API void TVoxelTextureStructBase<FColor>::SaveTexture() const;

template<>
VOXEL_API void TVoxelTextureStructBase<float>::LoadTexture() const;

template<>
VOXEL_API void TVoxelTextureStructBase<FColor>::LoadTexture() const;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelFloatTexture
#if CPP
	: TVoxelTextureStructBase<float>
#else
	: public FVoxelTextureStructBase
#endif
{
	GENERATED_BODY()

	using TVoxelTextureStructBase<float>::TVoxelTextureStructBase;
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelColorTexture
#if CPP
	: TVoxelTextureStructBase<FColor>
#else
	: public FVoxelTextureStructBase
#endif
{
	GENERATED_BODY()
	
	using TVoxelTextureStructBase<FColor>::TVoxelTextureStructBase;
};