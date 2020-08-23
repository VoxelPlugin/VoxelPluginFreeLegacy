// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMessages.h"
#include "VoxelFeedbackContext.h"
#include "VoxelAssets/VoxelHeightmapAssetData.h"
#include "VoxelUtilities/VoxelSerializationUtilities.h"

template<typename T>
void TVoxelHeightmapAssetData<T>::SetSize(int64 NewWidth, int64 NewHeight, bool bCreateMaterials, EVoxelMaterialConfig InMaterialConfig)
{
	VOXEL_FUNCTION_COUNTER();

	check(NewWidth > 0 && NewHeight > 0);

	const int64 NumHeights = NewWidth * NewHeight;
	Heights.Empty(NumHeights);
	Heights.SetNumUninitialized(NumHeights);

	if (bCreateMaterials)
	{
		int64 NumMaterials = NewWidth * NewHeight;
		switch (InMaterialConfig)
		{
		case EVoxelMaterialConfig::RGB: NumMaterials *= 4; break;
		case EVoxelMaterialConfig::SingleIndex: NumMaterials *= 1; break;
		case EVoxelMaterialConfig::MultiIndex: NumMaterials *= 7; break;
		default: ensure(false);
		}

		Materials.Empty(NumMaterials);
		Materials.SetNumUninitialized(NumMaterials);
	}
	else
	{
		Materials.Empty();
	}

	Width = NewWidth;
	Height = NewHeight;

	MinHeight = PositiveInfinity<T>();
	MaxHeight = NegativeInfinity<T>();

	MaterialConfig = InMaterialConfig;

	UpdateStats();
	InitializeHeightRangeMips();
}

template<typename T>
void TVoxelHeightmapAssetData<T>::SetAllHeightsTo(T NewHeight)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	for (auto& HeightIt : Heights)
	{
		HeightIt = NewHeight;
	}
	for (auto& HeightRangeMip : HeightRangeMips)
	{
		for (auto& HeightRange : HeightRangeMip.Data)
		{
			HeightRange = NewHeight;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
const TVoxelRange<T>& TVoxelHeightmapAssetData<T>::GetHeightRange(int64 X, int64 Y, int64 Mip, EVoxelSamplerMode SamplerMode) const
{
	int64 LocalX;
	int64 LocalY;
	GetHeightRangeLocalCoordinates(Mip, X, Y, LocalX, LocalY);

	FixHeightRangeLocalCoordinates(Mip, LocalX, LocalY, SamplerMode);

	return GetHeightRangeLocal(Mip, LocalX, LocalY);
}

template<typename T>
TVoxelRange<T> TVoxelHeightmapAssetData<T>::GetHeightRange(TVoxelRange<int64> X, TVoxelRange<int64> Y, EVoxelSamplerMode SamplerMode) const
{
	if (!ensure(X.Min < X.Max && Y.Min < Y.Max) || !ensure(GetNumHeightRangeMips() > 0))
	{
		return { MinHeight, MaxHeight };
	}

	int64 MinX = X.Min;
	int64 MaxX = X.Max;

	int64 MinY = Y.Min;
	int64 MaxY = Y.Max;

	if (SamplerMode == EVoxelSamplerMode::Clamp)
	{
		// Clamp min and max

		MinX = FMath::Clamp<int64>(MinX, 0, GetWidth() - 1);
		MinY = FMath::Clamp<int64>(MinY, 0, GetHeight() - 1);

		MaxX = FMath::Clamp<int64>(MaxX, 1, GetWidth());
		MaxY = FMath::Clamp<int64>(MaxY, 1, GetHeight());
	}
	else
	{
		// Tile min, and set max to be max one tile after
		// The coordinates will be tiled again when sampling the mips

		int64 SizeX = MaxX - MinX;
		int64 SizeY = MaxY - MinY;

		TileCoordinates(MinX, MinY);

		// Also tile Size to make sure it's under the heightmap size
		TileCoordinates(SizeX, SizeY);
		ensureVoxelSlow(SizeX < GetWidth());
		ensureVoxelSlow(SizeY < GetHeight());

		// If the size is a multiple or ours, fixup
		if (SizeX == 0) SizeX = GetWidth();
		if (SizeY == 0) SizeY = GetHeight();

		MaxX = MinX + SizeX;
		MaxY = MinY + SizeY;

		ensureVoxelSlowNoSideEffects(MaxX - MinX <= GetWidth());
		ensureVoxelSlowNoSideEffects(MaxY - MinY <= GetHeight());
	}

	const int64 SizeX = MaxX - MinX;
	const int64 SizeY = MaxY - MinY;

	const int64 MaxSize = FMath::Max(SizeX, SizeY);

	int64 Mip = FVoxelUtilities::GetDepthFromSize<RENDER_CHUNK_SIZE>(MaxSize);
	Mip = FMath::Clamp<int64>(Mip, 0, GetNumHeightRangeMips() - 1);

	const int64 MipPixelSize = RENDER_CHUNK_SIZE << Mip;

	const int64 LocalMinX = FVoxelUtilities::DivideFloor(MinX, MipPixelSize);
	const int64 LocalMinY = FVoxelUtilities::DivideFloor(MinY, MipPixelSize);

	// Note: since MaxX/Y are excluded, LocalMaxX/Y are too
	const int64 LocalMaxX = FVoxelUtilities::DivideCeil(MaxX, MipPixelSize);
	const int64 LocalMaxY = FVoxelUtilities::DivideCeil(MaxY, MipPixelSize);

	const int64 LocalSizeX = LocalMaxX - LocalMinX;
	const int64 LocalSizeY = LocalMaxY - LocalMinY;

	checkVoxelSlow(0 < LocalSizeX);
	checkVoxelSlow(0 < LocalSizeY);

	ensureVoxelSlow(LocalSizeX <= 2);
	ensureVoxelSlow(LocalSizeY <= 2);

	// Combine the range of all the overlapping pixels
	TOptional<TVoxelRange<T>> Range;
	for (int64 LocalX = LocalMinX; LocalX < LocalMaxX; LocalX++)
	{
		for (int64 LocalY = LocalMinY; LocalY < LocalMaxY; LocalY++)
		{
			int64 FixedLocalX = LocalX;
			int64 FixedLocalY = LocalY;

			// Tile the coordinates if needed
			FixHeightRangeLocalCoordinates(Mip, FixedLocalX, FixedLocalY, SamplerMode);
			ensureVoxelSlow(SamplerMode == EVoxelSamplerMode::Tile || (LocalX == FixedLocalX && LocalY == FixedLocalY));

			const auto LocalRange = GetHeightRangeLocal(Mip, FixedLocalX, FixedLocalY);

			Range = Range.IsSet() ? TVoxelRange<T>::Union(Range.GetValue(), LocalRange) : LocalRange;
		}
	}

	// Crashed
	ensureMsgfVoxelSlowNoSideEffects(
		Range.IsSet(),
		TEXT("LocalMinX: %lld; LocalMaxX: %lld; LocalMinY: %lld; LocalMaxY: %lld; Width: %lld; Height: %lld; MinX: %lld; MaxX: %lld; MinY: %lld; MaxY: %lld"),
		LocalMinX, LocalMaxX, LocalMinY, LocalMaxY, Width, Height, MinX, MaxX, MinY, MaxY);
	return Range.Get({ MinHeight, MaxHeight });
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FORCEINLINE void TVoxelHeightmapAssetData<T>::GetHeightRangeLocalCoordinates(int64 Mip, int64 X, int64 Y, int64& LocalX, int64& LocalY) const
{
	checkVoxelSlow(IsValidIndex(X, Y));
	checkVoxelSlow(HeightRangeMips.IsValidIndex(Mip));

	constexpr int64 BaseMipDepth = FVoxelUtilities::IntLog2(RENDER_CHUNK_SIZE);
	const int64 MipDepth = BaseMipDepth + Mip;

	// Find the mip coordinates by dividing and flooring
	LocalX = X >> MipDepth;
	LocalY = Y >> MipDepth;
}

template<typename T>
void TVoxelHeightmapAssetData<T>::FixHeightRangeLocalCoordinates(int64 Mip, int64& LocalX, int64& LocalY, EVoxelSamplerMode SamplerMode) const
{
	auto& HeightRangeMip = HeightRangeMips[Mip];

	if (SamplerMode == EVoxelSamplerMode::Clamp)
	{
		LocalX = FMath::Clamp<int64>(LocalX, 0, HeightRangeMip.Width - 1);
		LocalY = FMath::Clamp<int64>(LocalY, 0, HeightRangeMip.Height - 1);
	}
	else
	{
		LocalX = FVoxelUtilities::PositiveMod(LocalX, HeightRangeMip.Width);
		LocalY = FVoxelUtilities::PositiveMod(LocalY, HeightRangeMip.Height);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
void TVoxelHeightmapAssetData<T>::InitializeHeightRangeMips()
{
	const int64 NumHeightRangeMips = 1 + FVoxelUtilities::GetDepthFromSize<RENDER_CHUNK_SIZE>(FMath::Max(Width, Height));
	check(NumHeightRangeMips >= 1);

	HeightRangeMips.Empty();
	for (int64 MipIndex = 0; MipIndex < NumHeightRangeMips; MipIndex++)
	{
		const int64 MipPixelSize = RENDER_CHUNK_SIZE << MipIndex;

		TVoxelRange<T> Range;
		Range.Min = PositiveInfinity<T>();
		Range.Max = NegativeInfinity<T>();

		FHeightRangeMip NewMip;
		NewMip.Width = FVoxelUtilities::DivideCeil(Width, MipPixelSize);
		NewMip.Height = FVoxelUtilities::DivideCeil(Height, MipPixelSize);
		NewMip.Data.Init(Range, NewMip.Width * NewMip.Height);

		HeightRangeMips.Add(NewMip);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
void TVoxelHeightmapAssetData<T>::SetHeight(int64 X, int64 Y, T NewHeight)
{
	Heights[GetIndex(X, Y)] = NewHeight;

	MaxHeight = FMath::Max(MaxHeight, NewHeight);
	MinHeight = FMath::Min(MinHeight, NewHeight);

	for (int64 Mip = 0; Mip < GetNumHeightRangeMips(); Mip++)
	{
		int64 LocalX;
		int64 LocalY;
		GetHeightRangeLocalCoordinates(Mip, X, Y, LocalX, LocalY);

		auto& Range = GetHeightRangeLocal(Mip, LocalX, LocalY);
		Range.Min = FMath::Min(Range.Min, NewHeight);
		Range.Max = FMath::Max(Range.Max, NewHeight);
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
void TVoxelHeightmapAssetData<T>::SetMaterial_RGB(int64 X, int64 Y, FColor Color)
{
	checkVoxelSlow(MaterialConfig == EVoxelMaterialConfig::RGB);
	const int64 Index = GetIndex(X, Y);

	Materials[4 * Index + 0] = Color.R;
	Materials[4 * Index + 1] = Color.G;
	Materials[4 * Index + 2] = Color.B;
	Materials[4 * Index + 3] = Color.A;
}

template<typename T>
void TVoxelHeightmapAssetData<T>::SetMaterial_SingleIndex(int64 X, int64 Y, uint8 SingleIndex)
{
	checkVoxelSlow(MaterialConfig == EVoxelMaterialConfig::SingleIndex);
	Materials[GetIndex(X, Y)] = SingleIndex;
}

template<typename T>
void TVoxelHeightmapAssetData<T>::SetMaterial_MultiIndex(int64 X, int64 Y, const FVoxelMaterial& Material)
{
	checkVoxelSlow(MaterialConfig == EVoxelMaterialConfig::MultiIndex);
	const int64 Index = GetIndex(X, Y);

	Materials[7 * Index + 0] = Material.GetMultiIndex_Blend0();
	Materials[7 * Index + 1] = Material.GetMultiIndex_Blend1();
	Materials[7 * Index + 2] = Material.GetMultiIndex_Blend2();
	Materials[7 * Index + 3] = Material.GetMultiIndex_Index0();
	Materials[7 * Index + 4] = Material.GetMultiIndex_Index1();
	Materials[7 * Index + 5] = Material.GetMultiIndex_Index2();
	Materials[7 * Index + 6] = Material.GetMultiIndex_Index3();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FORCEINLINE FVoxelMaterial TVoxelHeightmapAssetData<T>::GetMaterialUnsafe(int64 Index) const
{
	FVoxelMaterial Material(ForceInit);
	switch (MaterialConfig)
	{
	case EVoxelMaterialConfig::RGB:
		Material.SetR(Materials[4 * Index + 0]);
		Material.SetG(Materials[4 * Index + 1]);
		Material.SetB(Materials[4 * Index + 2]);
		Material.SetA(Materials[4 * Index + 3]);
		break;
	case EVoxelMaterialConfig::SingleIndex:
		Material.SetSingleIndex(Materials[Index]);
		break;
	case EVoxelMaterialConfig::MultiIndex:
	default:
		Material.SetMultiIndex_Blend0(Materials[7 * Index + 0]);
		Material.SetMultiIndex_Blend1(Materials[7 * Index + 1]);
		Material.SetMultiIndex_Blend2(Materials[7 * Index + 2]);
		Material.SetMultiIndex_Index0(Materials[7 * Index + 3]);
		Material.SetMultiIndex_Index1(Materials[7 * Index + 4]);
		Material.SetMultiIndex_Index2(Materials[7 * Index + 5]);
		Material.SetMultiIndex_Index3(Materials[7 * Index + 6]);
		break;
	}
	return Material;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
FORCEINLINE void TVoxelHeightmapAssetData<T>::ClampCoordinates(int64& X, int64& Y) const
{
	X = FMath::Clamp<int64>(X, 0, GetWidth() - 1);
	Y = FMath::Clamp<int64>(Y, 0, GetHeight() - 1);
}

template<typename T>
FORCEINLINE void TVoxelHeightmapAssetData<T>::TileCoordinates(int64& X, int64& Y) const
{
	X = FVoxelUtilities::PositiveMod(X, GetWidth());
	Y = FVoxelUtilities::PositiveMod(Y, GetHeight());
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
T TVoxelHeightmapAssetData<T>::GetHeight(int64 X, int64 Y, EVoxelSamplerMode Mode) const
{
	if (!IsValidIndex(X, Y))
	{
		if (Mode == EVoxelSamplerMode::Tile)
		{
			TileCoordinates(X, Y);
		}
		else
		{
			ClampCoordinates(X, Y);
		}
		checkVoxelSlow(IsValidIndex(X, Y));
	}
	return GetHeightUnsafe(X, Y);
}

template<typename T>
FVoxelMaterial TVoxelHeightmapAssetData<T>::GetMaterial(int64 X, int64 Y, EVoxelSamplerMode Mode) const
{
	if (!IsValidIndex(X, Y))
	{
		if (Mode == EVoxelSamplerMode::Tile)
		{
			TileCoordinates(X, Y);
		}
		else
		{
			ClampCoordinates(X, Y);
		}
		checkVoxelSlow(IsValidIndex(X, Y));
	}
	return GetMaterialUnsafe(X, Y);
}

template<typename T>
float TVoxelHeightmapAssetData<T>::GetHeight(float X, float Y, EVoxelSamplerMode Mode) const
{
	const int64 MinX = FMath::FloorToInt(X);
	const int64 MinY = FMath::FloorToInt(Y);

	const int64 MaxX = FMath::CeilToInt(X);
	const int64 MaxY = FMath::CeilToInt(Y);

	const float AlphaX = X - MinX;
	const float AlphaY = Y - MinY;

	return FVoxelUtilities::BilinearInterpolation<float>(
		GetHeight(MinX, MinY, Mode),
		GetHeight(MaxX, MinY, Mode),
		GetHeight(MinX, MaxY, Mode),
		GetHeight(MaxX, MaxY, Mode),
		AlphaX,
		AlphaY);
}

template<typename T>
FVoxelMaterial TVoxelHeightmapAssetData<T>::GetMaterial(float X, float Y, EVoxelSamplerMode Mode) const
{
	if (HasMaterials())
	{
		return GetMaterial(FMath::RoundToInt(X), FMath::RoundToInt(Y), Mode);
	}
	else
	{
		return FVoxelMaterial::Default();
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename T>
void TVoxelHeightmapAssetData<T>::Serialize(FArchive& Ar, uint32 MaterialConfigFlag, FVoxelHeightmapAssetDataVersion::Type Version, bool& bNeedToSave)
{
	VOXEL_FUNCTION_COUNTER();
	
	FVoxelScopedSlowTask Serializing(3.f);
	
	Serializing.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Serializing heights"));
	if (Version < FVoxelHeightmapAssetDataVersion::UseTArray64)
	{
		TArray<T> OldHeights;
		if (Version == FVoxelHeightmapAssetDataVersion::BeforeCustomVersionWasAdded)
		{
			Ar << OldHeights;
		}
		else
		{
			OldHeights.BulkSerialize(Ar);
		}
		Heights = OldHeights;
	}
	else
	{
		Heights.BulkSerialize(Ar);
	}

	Serializing.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Serializing materials"));
	if (Version < FVoxelHeightmapAssetDataVersion::NoVoxelMaterialInHeightmapAssets)
	{
		TNoGrowArray<FVoxelMaterial> LegacyMaterials;
		// Note: don't do the cast for newer versions
		FVoxelSerializationUtilities::SerializeMaterials(Ar, LegacyMaterials, MaterialConfigFlag, FVoxelSerializationVersion::Type(Version));

		// Assume RGB
		Materials.Reserve(LegacyMaterials.Num() * 4);
		for (auto& Material : LegacyMaterials)
		{
			const FColor Color = Material.GetColor();
			Materials.Add(Color.R);
			Materials.Add(Color.G);
			Materials.Add(Color.B);
			Materials.Add(Color.A);
		}
	}
	else if (Version < FVoxelHeightmapAssetDataVersion::FixMissingMaterialsInHeightmapAssets)
	{
		// Do nothing
	}
	else if (Version < FVoxelHeightmapAssetDataVersion::UseTArray64)
	{
		TArray<uint8> OldMaterials;
		OldMaterials.BulkSerialize(Ar);
		Materials = OldMaterials;
	}
	else
	{
		Materials.BulkSerialize(Ar);
	}

	if (Version < FVoxelHeightmapAssetDataVersion::UseTArray64)
	{
		int32 Width32;
		int32 Height32;
		Ar << Width32;
		Ar << Height32;
		Width = Width32;
		Height = Height32;
	}
	else
	{
		Ar << Width;
		Ar << Height;
	}

	Ar << MaxHeight;
	Ar << MinHeight;

	if (Version >= FVoxelHeightmapAssetDataVersion::NoVoxelMaterialInHeightmapAssets)
	{
		Ar << MaterialConfig;
	}

	if (Width * Height != Heights.Num())
	{
		Ar.SetError();
	}

	if (Materials.Num() > 0)
	{
		int64 MaterialSize = 0;
		switch (MaterialConfig)
		{
		case EVoxelMaterialConfig::RGB:
			MaterialSize = 4;
			break;
		case EVoxelMaterialConfig::SingleIndex:
			MaterialSize = 1;
			break;
		case EVoxelMaterialConfig::DoubleIndex_DEPRECATED:
			MaterialSize = 0;
			MaterialConfig = EVoxelMaterialConfig::RGB;
			Materials.Empty();
			FVoxelMessages::Error("Cannot load double index heightmap materials, removing them. You'll need to reimport your weightmaps");
			break;
		case EVoxelMaterialConfig::MultiIndex:
			MaterialSize = 7;
			break;
		default:
			Ar.SetError();
		}

		if (MaterialSize * Width * Height != Materials.Num())
		{
			Ar.SetError();
		}
	}

	Serializing.EnterProgressFrame(1.f, VOXEL_LOCTEXT("Recomputing height range mips"));
	if (Version < FVoxelHeightmapAssetDataVersion::SerializeHeightRangeMips)
	{
		VOXEL_SCOPE_COUNTER("Recomputing height range mips");

		const double StartTime = FPlatformTime::Seconds();

		InitializeHeightRangeMips();
		for (int64 X = 0; X < Width; X++)
		{
			for (int64 Y = 0; Y < Height; Y++)
			{
				const T LocalHeight = GetHeightUnsafe(X, Y);
				for (int64 Mip = 0; Mip < GetNumHeightRangeMips(); Mip++)
				{
					int64 LocalX;
					int64 LocalY;
					GetHeightRangeLocalCoordinates(Mip, X, Y, LocalX, LocalY);

					auto& Range = GetHeightRangeLocal(Mip, LocalX, LocalY);
					Range.Min = FMath::Min(Range.Min, LocalHeight);
					Range.Max = FMath::Max(Range.Max, LocalHeight);
				}
			}
		}

		const double EndTime = FPlatformTime::Seconds();

		bNeedToSave = true;

		int64 Size = HeightRangeMips.GetAllocatedSize();
		for (auto& Mip : HeightRangeMips)
		{
			Size += Mip.Data.GetAllocatedSize();
		}
		LOG_VOXEL(Log, TEXT("Recomputing height range mips took %fs. Using %fMB for %lldx%lld"), EndTime - StartTime, Size / double(1 << 20), Width, Height);
	}
	else
	{
		Ar << HeightRangeMips;
	}

	UpdateStats();
}

template<typename T>
void TVoxelHeightmapAssetData<T>::UpdateStats()
{
	DEC_VOXEL_MEMORY_STAT_BY(STAT_VoxelHeightmapAssetMemory, AllocatedSize);
	AllocatedSize = Heights.GetAllocatedSize() + Materials.GetAllocatedSize() + HeightRangeMips.GetAllocatedSize();
	for (auto& Mip : HeightRangeMips)
	{
		AllocatedSize += Mip.Data.GetAllocatedSize();
	}
	INC_VOXEL_MEMORY_STAT_BY(STAT_VoxelHeightmapAssetMemory, AllocatedSize);
}