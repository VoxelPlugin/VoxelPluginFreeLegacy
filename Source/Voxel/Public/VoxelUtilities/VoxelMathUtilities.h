// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelStaticArray.h"
#include "VoxelUtilities/VoxelIntVectorUtilities.h"

namespace FVoxelUtilities
{
#define CHECK_CHUNK_SIZE() static_assert(FVoxelUtilities::IsPowerOfTwo(ChunkSize), "ChunkSize must be a power of 2")
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
	inline int32 GetDepthFromBounds(const FVoxelIntBox& Bounds)
	{
		CHECK_CHUNK_SIZE();
		return GetDepthFromSize<ChunkSize>(Bounds.Size().GetMax());
	}

	template<uint32 ChunkSize>
	inline FVoxelIntBox GetBoundsFromDepth(int32 Depth)
	{
		CHECK_CHUNK_SIZE();
		const FIntVector Size = FIntVector((ChunkSize << Depth) / 2);
		return FVoxelIntBox(-Size, Size);
	}
	
	template<uint32 ChunkSize>
	inline FVoxelIntBox GetCustomBoundsForDepth(FVoxelIntBox Bounds, int32 Depth)
	{
		CHECK_CHUNK_SIZE();
		Bounds = Bounds.MakeMultipleOfBigger(ChunkSize);
		Bounds = FVoxelUtilities::GetBoundsFromDepth<ChunkSize>(Depth).Overlap(Bounds);
		check(Bounds.IsMultipleOf(ChunkSize));
		return Bounds;
	}

	template<uint32 ChunkSize>
	inline FVoxelIntBox GetBoundsFromPositionAndDepth(const FIntVector& Position, int32 Depth)
	{
		CHECK_CHUNK_SIZE();
		return FVoxelIntBox(Position, Position + FIntVector(ChunkSize << Depth));
	}

	// Valid for root node only
	template<uint32 ChunkSize>
	inline int32 GetOctreeDepthContainingBounds(const FVoxelIntBox& Bounds)
	{
		CHECK_CHUNK_SIZE();
		const uint32 Max = FMath::Max(FVoxelUtilities::Abs(Bounds.Min).GetMax(), FVoxelUtilities::Abs(Bounds.Max).GetMax());
		return GetDepthFromSize<ChunkSize>(2 * Max); // 2x: octree doesn't start at 0 0 0
	}

	template<uint32 FromChunkSize, uint32 ToChunkSize>
	inline constexpr int32 ConvertDepth(int32 Depth)
	{
		static_assert(IsPowerOfTwo(FromChunkSize), "FromChunkSize must be a power of 2");
		static_assert(IsPowerOfTwo(ToChunkSize), "ToChunkSize must be a power of 2");
		
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

	// Falloff: between 0 and 1
	inline v_flt RoundCylinder(const FVoxelVector& PositionRelativeToCenter, v_flt Radius, v_flt Height, v_flt Falloff)
	{
		const v_flt InternalRadius = Radius * (1.f - Falloff);
		const v_flt ExternalRadius = Radius * Falloff;

		const v_flt DistanceToCenterXY = FVector2D(PositionRelativeToCenter.X, PositionRelativeToCenter.Y).Size();
		const v_flt DistanceToCenterZ = FMath::Abs(PositionRelativeToCenter.Z);

		const v_flt SidesSDF = DistanceToCenterXY - InternalRadius;
		const v_flt TopSDF = DistanceToCenterZ - Height / 2 + ExternalRadius;
		
		return
			FMath::Min<v_flt>(FMath::Max<v_flt>(SidesSDF, TopSDF), 0.0f) +
			FVector2D(FMath::Max<v_flt>(SidesSDF, 0.f), FMath::Max<v_flt>(TopSDF, 0.f)).Size() +
			-ExternalRadius;
	}

	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////

	template<typename TIn, typename TOut>
	FORCEINLINE void XWayBlend_AlphasToStrengths_Impl(int32 NumChannels, const TIn& Alphas, TOut& Strengths)
	{
		ensureVoxelSlow(NumChannels > 1);
		
		// Unpack the strengths from the lerp values
		for (int32 Index = 0; Index < NumChannels; Index++)
		{
			Strengths[Index] = Index == 0 ? 1.f : Alphas[Index - 1];
			for (int32 AlphaIndex = Index; AlphaIndex < NumChannels - 1; AlphaIndex++)
			{
				Strengths[Index] *= 1.f - Alphas[AlphaIndex];
			}
		}

#if VOXEL_DEBUG
		float Sum = 0.f;
		for (int32 Index = 0; Index < NumChannels; Index++)
		{
			Sum += Strengths[Index];
		}
		ensure(FMath::IsNearlyEqual(Sum, 1.f, KINDA_SMALL_NUMBER));
#endif
	}

	template<typename TIn, typename TOut>
	FORCEINLINE void XWayBlend_StrengthsToAlphas_Impl(int32 NumChannels, TIn Strengths, TOut& Alphas, uint32 ChannelsToKeepIntact = 0)
	{
		ensureVoxelSlow(NumChannels > 1);
		
		if (!ChannelsToKeepIntact)
		{
			// Normalize: we want the sum to be 1
			float Sum = 0.f;
			for (int32 Index = 0; Index < NumChannels; Index++)
			{
				Sum += Strengths[Index];
			}
			if (Sum != 0.f) // This can very rarely happen if we subtracted all the data when editing
			{
				for (int32 Index = 0; Index < NumChannels; Index++)
				{
					Strengths[Index] /= Sum;
				}
			}
		}
		else
		{
			// Normalize so that Strengths[Index] doesn't change if (1 << Index) & ChannelsToKeepIntact
			{
				// Sum of all the other components
				float SumToKeepIntact = 0.f;
				float SumToChange = 0.f;
				for (int32 Index = 0; Index < NumChannels; Index++)
				{
					if ((1u << Index) & ChannelsToKeepIntact)
					{
						SumToKeepIntact += Strengths[Index];
					}
					else
					{
						SumToChange += Strengths[Index];
					}
				}

				// If the sum to keep intact is above 1, normalize these channels too
				// (but on their own)
				if (SumToKeepIntact > 1.f)
				{
					for (int32 Index = 0; Index < NumChannels; Index++)
					{
						if ((1u << Index) & ChannelsToKeepIntact)
						{
							Strengths[Index] /= SumToKeepIntact;
						}
					}
					SumToKeepIntact = 1.f;
				}
				
				// We need to split this into the other channels.
				const float SumToSplit = 1.f - SumToKeepIntact;
				if (SumToChange == 0.f)
				{
					// If the sum is 0, increase all the other channels the same way
					const float Value = SumToSplit / (NumChannels - FMath::CountBits(ChannelsToKeepIntact));
					
					for (int32 Index = 0; Index < NumChannels; Index++)
					{
						if (!((1u << Index) & ChannelsToKeepIntact))
						{
							Strengths[Index] = Value;
						}
					}
				}
				else
				{
					// Else scale them
					const float Value = SumToSplit / SumToChange;
					
					for (int32 Index = 0; Index < NumChannels; Index++)
					{
						if (!((1u << Index) & ChannelsToKeepIntact))
						{
							Strengths[Index] *= Value;
						}
					}
				}
			}
		}

#if VOXEL_DEBUG
		float Sum = 0.f;
		for (int32 Index = 0; Index < NumChannels; Index++)
		{
			Sum += Strengths[Index];
		}
		ensure(FMath::IsNearlyEqual(Sum, 1.f, 0.001f));
#endif

		const auto SafeDivide = [](float X, float Y)
		{
			// Here we resolve Y * A = X. If Y = 0, X should be 0 and A can be anything (here return 0)
			ensureVoxelSlowNoSideEffects(Y != 0.f || FMath::IsNearlyZero(X));
			return Y == 0.f ? 0.f : X / Y;
		};
		
		// Pack them back in: do the maths in reverse order
		const int32 NumAlphas = NumChannels - 1;
		for (int32 AlphaIndex = NumAlphas - 1; AlphaIndex >= 0; AlphaIndex--)
		{
			float Divisor = 1.f;
			for (int32 DivisorIndex = AlphaIndex + 1; DivisorIndex < NumAlphas; DivisorIndex++)
			{
				Divisor *= 1.f - Alphas[DivisorIndex];
			}

			Alphas[AlphaIndex] = SafeDivide(Strengths[AlphaIndex + 1], Divisor);
			
		}
	}

	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////

	template<int32 NumChannels>
	FORCEINLINE TVoxelStaticArray<float, NumChannels> XWayBlend_AlphasToStrengths_Static(const TVoxelStaticArray<float, NumChannels - 1>& Alphas)
	{
		TVoxelStaticArray<float, NumChannels> Strengths;
		XWayBlend_AlphasToStrengths_Impl(NumChannels, Alphas, Strengths);
		return Strengths;
	}

	template<int32 NumChannels>
	FORCEINLINE TVoxelStaticArray<float, NumChannels - 1> XWayBlend_StrengthsToAlphas_Static(const TVoxelStaticArray<float, NumChannels>& Strengths, uint32 ChannelsToKeepIntact = 0)
	{
		TVoxelStaticArray<float, NumChannels - 1> Alphas;
		XWayBlend_StrengthsToAlphas_Impl(NumChannels, Strengths, Alphas, ChannelsToKeepIntact);
		return Alphas;
	}
	// Avoid mistakes with ChannelsToKeepIntact not being a mask
	template<int32 NumChannels, typename T>
	void XWayBlend_StrengthsToAlphas_Static(const TVoxelStaticArray<float, NumChannels>&, T) = delete;

	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////

	template<typename TOutAllocator = FDefaultAllocator, typename TInArray>
	FORCEINLINE TArray<float, TOutAllocator> XWayBlend_AlphasToStrengths_Dynamic(const TInArray& Alphas)
	{
		TArray<float, TOutAllocator> Strengths;
		Strengths.SetNumUninitialized(Alphas.Num() + 1);
		XWayBlend_AlphasToStrengths_Impl(Alphas.Num() + 1, Alphas, Strengths);
		return Strengths;
	}

	template<typename TOutAllocator = FDefaultAllocator, typename TInArray>
	FORCEINLINE TArray<float, TOutAllocator> XWayBlend_StrengthsToAlphas_Dynamic(const TInArray& Strengths, uint32 ChannelsToKeepIntact = 0)
	{
		TArray<float, TOutAllocator> Alphas;
		Alphas.SetNumUninitialized(Strengths.Num() - 1);
		XWayBlend_StrengthsToAlphas_Impl(Strengths.Num(), Strengths, Alphas, ChannelsToKeepIntact);
		return Alphas;
	}
	// Avoid mistakes with ChannelsToKeepIntact not being a mask
	template<typename TInArray, typename T>
	void XWayBlend_StrengthsToAlphas_Dynamic(const TInArray&, T) = delete;

	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////
	
	template<int32 N, typename T, typename ArrayType, typename TLambda>
	FORCEINLINE TVoxelStaticArray<TTuple<int32, T>, N> FindTopXElements_Impl(const ArrayType& Array, TLambda LessThan = TLess<T>())
	{
		checkVoxelSlow(Array.Num() >= N);

		// Biggest elements on top
		TVoxelStaticArray<TTuple<int32, T>, N> Stack;

		// Fill stack with first N values
		for (int32 Index = 0; Index < N; Index++)
		{
			Stack[Index].template Get<0>() = Index;
			Stack[Index].template Get<1>() = Array[Index];
		}

		// Sort the stack
		Algo::Sort(Stack, [&](const auto& A, const auto& B) { return LessThan(B.template Get<1>(), A.template Get<1>()); });

		for (int32 Index = N; Index < Array.Num(); Index++)
		{
			const T& ArrayValue = Array[Index];
			if (!LessThan(Stack[N - 1].template Get<1>(), ArrayValue))
			{
				// Smaller than the entire stack
				continue;
			}

			// Find the element to replace
			int32 StackToReplace = N - 1;
			while (StackToReplace >= 1 && LessThan(Stack[StackToReplace - 1].template Get<1>(), ArrayValue))
			{
				StackToReplace--;
			}

			// Move existing elements down
			for (int32 StackIndex = N - 1; StackIndex > StackToReplace; StackIndex--)
			{
				Stack[StackIndex] = Stack[StackIndex - 1];
			}

			// Write new element
			Stack[StackToReplace].template Get<0>() = Index;
			Stack[StackToReplace].template Get<1>() = ArrayValue;
		}

		return Stack;
	}
	template<int32 N, typename T, typename Allocator, typename TLambda>
	FORCEINLINE TVoxelStaticArray<TTuple<int32, T>, N> FindTopXElements(const TArray<T, Allocator>& Array, TLambda LessThan = TLess<T>())
	{
		return FindTopXElements_Impl<N, T>(Array, LessThan);
	}
	template<int32 N, typename T, int32 Num, typename TLambda>
	FORCEINLINE TVoxelStaticArray<TTuple<int32, T>, N> FindTopXElements(const TVoxelStaticArray<T, Num>& Array, TLambda LessThan = TLess<T>())
	{
		return FindTopXElements_Impl<N, T>(Array, LessThan);
	}
}