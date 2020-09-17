// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "FastNoise/VoxelFastNoise.inl"

class FVoxelFastNoiseTest : public FVoxelFastNoise
{
public:
	FORCENOINLINE v_flt SingleValue_2D_NoInline(uint8 offset, v_flt x, v_flt y) const
	{
		return SingleValue_2D(offset, x, y);
	}
	FORCENOINLINE VectorRegister SingleValue_2D_NoInline(VectorRegisterInt offset, VectorRegister x, VectorRegister y) const
	{
		return SingleValue_2D(offset, x, y);
	}
	
	static void Test()
	{
		FVoxelFastNoiseTest FastNoise;
		FastNoise.SetSeed(0);

		const int32 Num = 10000000;
		
		v_flt Results[256][4];
		for (int32 Index = 0; Index < 256; Index++)
		{
			Results[Index][0] = FastNoise.SingleValue_2D_NoInline(Index, 0.5f, 4.5f);
			Results[Index][1] = FastNoise.SingleValue_2D_NoInline(Index, 1.5f, 5.5f);
			Results[Index][2] = FastNoise.SingleValue_2D_NoInline(Index, 2.5f, 6.5f);
			Results[Index][3] = FastNoise.SingleValue_2D_NoInline(Index, 3.5f, 7.5f);
		}

		for (int32 LoopIndex = 0; LoopIndex < 10; LoopIndex++)
		{
			{
				const double StartTime = FPlatformTime::Seconds();
				for (int32 Index = 0; Index < Num; Index++)
				{
					const uint8 Byte = Index;
					
					v_flt Result[4];
					Result[0] = FastNoise.SingleValue_2D_NoInline(Byte, 0.5f, 4.5f);
					Result[1] = FastNoise.SingleValue_2D_NoInline(Byte, 1.5f, 5.5f);
					Result[2] = FastNoise.SingleValue_2D_NoInline(Byte, 2.5f, 6.5f);
					Result[3] = FastNoise.SingleValue_2D_NoInline(Byte, 3.5f, 7.5f);

					ensure(Result[0] == Results[Byte][0] && Result[1] == Results[Byte][1] && Result[2] == Results[Byte][2] && Result[3] == Results[Byte][3]);
				}
				const double EndTime = FPlatformTime::Seconds();
				LOG_VOXEL(Log, TEXT("no SIMD took %fs (%fns)"), EndTime - StartTime, (EndTime - StartTime) / Num * 1e9);
			}
			{
				const double StartTime = FPlatformTime::Seconds();
				for (int32 Index = 0; Index < Num; Index++)
				{
					const uint8 Byte = Index;
					
					const auto Offset = MakeVectorRegisterInt(Byte, Byte, Byte, Byte);
					const auto X = MakeVectorRegister(0.5f, 1.5f, 2.5f, 3.5f);
					const auto Y = MakeVectorRegister(4.5f, 5.5f, 6.5f, 7.5f);
					const auto VectorResult = FastNoise.SingleValue_2D_NoInline(Offset, X, Y);

					float Result[4];
					VectorStore(VectorResult, Result);
					
					ensure(Result[0] == Results[Byte][0] && Result[1] == Results[Byte][1] && Result[2] == Results[Byte][2] && Result[3] == Results[Byte][3]);
				}
				const double EndTime = FPlatformTime::Seconds();
				LOG_VOXEL(Log, TEXT("SIMD took %fs (%fns)"), EndTime - StartTime, (EndTime - StartTime) / Num * 1e9);
			}
		}

		UE_DEBUG_BREAK();
	}
};