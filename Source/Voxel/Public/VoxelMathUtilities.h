// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"
#include "IntBox.h"
#include "Containers/StaticArray.h"
#include "VoxelIntVectorUtilities.h"

namespace FVoxelUtilities
{
#define CHECK_CHUNK_SIZE() static_assert(VoxelGlobalsUtils::IsPowerOfTwo(ChunkSize), "ChunkSize must be a power of 2")
	// Get required depth such that ChunkSize << Depth >= Size
	template<uint32 ChunkSize>
	inline int32 GetDepthFromSize(uint32 Size)
	{
		CHECK_CHUNK_SIZE();
		if (Size <= 0)
		{
			return 0;
		}
		else
		{
			const int32 Depth = 31 - FPlatformMath::CountLeadingZeros(Size / ChunkSize);
			if (ChunkSize << Depth == Size)
			{
				return Depth;
			}
			else
			{
				return Depth + 1;
			}
		}
	}
	
	template<uint32 ChunkSize>
	inline constexpr uint32 GetSizeFromDepth(int32 Depth)
	{
		CHECK_CHUNK_SIZE();
		return ChunkSize << Depth;
	}

	template<uint32 ChunkSize>
	inline int32 GetDepthFromBounds(const FIntBox& Bounds)
	{
		CHECK_CHUNK_SIZE();
		return GetDepthFromSize<ChunkSize>(Bounds.Size().GetMax());
	}

	template<uint32 ChunkSize>
	inline FIntBox GetBoundsFromDepth(int32 Depth)
	{
		CHECK_CHUNK_SIZE();
		const FIntVector Size = FIntVector((ChunkSize << Depth) / 2);
		return FIntBox(-Size, Size);
	}
	
	template<uint32 ChunkSize>
	inline FIntBox GetCustomBoundsForDepth(FIntBox Bounds, int32 Depth)
	{
		CHECK_CHUNK_SIZE();
		Bounds = Bounds.MakeMultipleOfBigger(ChunkSize);
		Bounds = FVoxelUtilities::GetBoundsFromDepth<ChunkSize>(Depth).Overlap(Bounds);
		check(Bounds.IsMultipleOf(ChunkSize));
		return Bounds;
	}

	template<uint32 ChunkSize>
	inline FIntBox GetBoundsFromPositionAndDepth(const FIntVector& Position, int32 Depth)
	{
		CHECK_CHUNK_SIZE();
		return FIntBox(Position, Position + FIntVector(ChunkSize << Depth));
	}

	// Valid for root node only
	template<uint32 ChunkSize>
	inline int32 GetOctreeDepthContainingBounds(const FIntBox& Bounds)
	{
		CHECK_CHUNK_SIZE();
		const uint32 Max = FMath::Max(FVoxelUtilities::Abs(Bounds.Min).GetMax(), FVoxelUtilities::Abs(Bounds.Max).GetMax());
		return GetDepthFromSize<ChunkSize>(2 * Max); // 2x: octree doesn't start at 0 0 0
	}

	template<uint32 FromChunkSize, uint32 ToChunkSize>
	inline constexpr int32 ConvertDepth(int32 Depth)
	{
		static_assert(VoxelGlobalsUtils::IsPowerOfTwo(FromChunkSize), "FromChunkSize must be a power of 2");
		static_assert(VoxelGlobalsUtils::IsPowerOfTwo(ToChunkSize), "ToChunkSize must be a power of 2");
		
		if (FromChunkSize == ToChunkSize)
		{
			return Depth;
		}
		else if (FromChunkSize < ToChunkSize)
		{
			// Depth should be lower
			return Depth - IntLog2(ToChunkSize / FromChunkSize);
		}
		else
		{
			// FromChunkSize > ToChunkSize
			// Depth should be higher
			return Depth + IntLog2(FromChunkSize / ToChunkSize);
		}
	}

	template<uint32 ChunkSize>
	inline int32 ClampDepth(int32 Depth)
	{
		CHECK_CHUNK_SIZE();
		constexpr int32 ChunkSizeDepth = IntLog2(ChunkSize);
		constexpr int32 MaxDepth = 31;
		// ChunkSizeDepth + Depth <= MaxDepth
		// Depth <= MaxDepth - ChunkSizeDepth
		return FMath::Clamp(Depth, 0, MaxDepth - ChunkSizeDepth);
	}
#undef CHECK_CHUNK_SIZE

	
	inline int32 ClampMesherDepth(int32 Depth)
	{
		// 2x: Bounds.Size() needs to fit in a int32 for Meshers
		return ClampDepth<2 * RENDER_CHUNK_SIZE>(Depth);
	}
	
	template<typename T>
	inline T MergeAsset(T A, T B, bool bSubtractiveAsset)
	{
		return bSubtractiveAsset ? FMath::Max(A, B) : FMath::Min(A, B);
	}

	inline float SmoothUnion(float DistanceA, float DistanceB, float K)
	{
		const float H = FMath::Clamp(0.5f + 0.5f * (DistanceB - DistanceA) / K, 0.0f, 1.0f);
		return FMath::Lerp(DistanceB, DistanceA, H) - K * H * (1.0f - H);
	}
	inline float SmoothSubtraction(float DistanceA, float DistanceB, float K)
	{
		const float H = FMath::Clamp(0.5f - 0.5f * (DistanceB + DistanceA) / K, 0.0f, 1.0f);
		return FMath::Lerp(DistanceB, -DistanceA, H) + K * H * (1.0f - H);
	}
	inline float SmoothIntersection(float DistanceA, float DistanceB, float K)
	{
		const float H = FMath::Clamp(0.5f - 0.5f * (DistanceB - DistanceA) / K, 0.0f, 1.0f);
		return FMath::Lerp(DistanceB, DistanceA, H) + K * H * (1.0f - H);
	}

	// Falloff: between 0 and 1
	template<typename TVector>
	inline float RoundCylinder(const TVector& PositionRelativeToCenter, float Radius, float Height, float Falloff)
	{
		// https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
		const float InternalRadius = Radius * (1.f - Falloff);
		const float ExternalRadius = Radius * Falloff;

		const float DistanceToCenterXY = FVector2D(PositionRelativeToCenter.X, PositionRelativeToCenter.Y).Size();
		const float DistanceToCenterZ = FMath::Abs(PositionRelativeToCenter.Z);

		const float SidesSDF = DistanceToCenterXY - InternalRadius;
		const float TopSDF = DistanceToCenterZ - Height / 2 + ExternalRadius;
		
		return
			FMath::Min(FMath::Max(SidesSDF, TopSDF), 0.0f) +
			FVector2D(FMath::Max(SidesSDF, 0.f), FMath::Max(TopSDF, 0.f)).Size() +
			-ExternalRadius;
	}

	inline TStaticArray<float, 5> ConvertRGBAToFiveWayBlendStrengths(const FVector4& Color)
	{
		TStaticArray<float, 5> Strengths;

		// Unpack the strengths from the lerp values
		// From Wolfram:
		// 
		// lerp[u_, v_, w_] := u * (1 - w) + v * w
		// Collect[FullSimplify[lerp[lerp[lerp[lerp[a,b, R], c, G], d, B], e, A]], {a, b, c, d, e}] // InputForm
		//
		// gives:
		// a*(1 - A)*(1 - B)*(1 - G)*(1 - R) +
		// b*(1 - A)*(1 - B)*(1 - G)*R +
		// c*(1 - A)*(1 - B)*G +
		// d*(1 - A)*B +
		// e*A

		{
			const float R = Color.X;
			const float G = Color.Y;
			const float B = Color.Z;
			const float A = Color.W;

			Strengths[0] = (1.f - A) * (1.f - B) * (1.f - G) * (1.f - R);
			Strengths[1] = (1.f - A) * (1.f - B) * (1.f - G) * R;
			Strengths[2] = (1.f - A) * (1.f - B) * G;
			Strengths[3] = (1.f - A) * B;
			Strengths[4] = A;
		}

		ensureVoxelSlowNoSideEffects(FMath::IsNearlyEqual(Strengths[0] + Strengths[1] + Strengths[2] + Strengths[3] + Strengths[4], 1.f, KINDA_SMALL_NUMBER));

		return Strengths;
	}

	inline FVector4 ConvertFiveWayBlendStrengthsToRGBA(TStaticArray<float, 5> Strengths)
	{
		// Normalize: we want the sum to be 1
		const float Sum = Strengths[0] + Strengths[1] + Strengths[2] + Strengths[3] + Strengths[4];
		if (Sum != 0.f) // This can very rarely happen if we subtracted all the data when editing
		{
			Strengths[0] /= Sum;
			Strengths[1] /= Sum;
			Strengths[2] /= Sum;
			Strengths[3] /= Sum;
			Strengths[4] /= Sum;
		}

		const auto SafeDivide = [](float X, float Y)
		{
			// Here we resolve Y * A = X. If Y = 0, X should be 0 and A can be anything (here return 0)
			ensureVoxelSlowNoSideEffects(Y != 0.f || FMath::IsNearlyZero(X));
			return Y == 0.f ? 0.f : X / Y;
		};

		// Pack them back in: do the maths in reverse order
		const float A = Strengths[4];
		const float B = SafeDivide(Strengths[3], ((1.f - A)));
		const float G = SafeDivide(Strengths[2], ((1.f - A) * (1.f - B)));
		const float R = SafeDivide(Strengths[1], ((1.f - A) * (1.f - B) * (1.f - G)));

		return FVector4(R, G, B, A);
	}
}