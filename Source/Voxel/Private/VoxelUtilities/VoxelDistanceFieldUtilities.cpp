// Copyright Voxel Plugin SAS. All Rights Reserved.

#include "VoxelUtilities/VoxelDistanceFieldUtilities.h"
#include "VoxelUtilities/VoxelDistanceFieldUtilities.inl"
#include "VoxelUtilities/VoxelMathUtilities.h"
#include "VoxelShaders/VoxelDistanceFieldShader.h"

#include "Async/ParallelFor.h"

FColor FVoxelDistanceFieldUtilities::GetDistanceFieldColor(float Value)
{
	// Credit for this snippet goes to Inigo Quilez
	
	FLinearColor Color = FLinearColor::White - FMath::Sign(Value) * FLinearColor(0.1, 0.4, 0.7, 0.f);
	Color *= 1.0 - FMath::Exp(-3.0 * FMath::Abs(Value));
	Color *= 0.8 + 0.2 * FMath::Cos(150.0 * Value);
	Color = FMath::Lerp(Color, FLinearColor::White, 1.0 - FMath::SmoothStep(0.0, 0.01, FMath::Abs<double>(Value)));
	Color.A = 1.f;
	return FLinearColor(Color.ToFColor(false)).ToFColor(false);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDistanceFieldUtilities::JumpFlood(const FIntVector& Size, TArray<FVector3f>& InOutSurfacePositions, bool bMultiThreaded, int32 MaxPasses_Debug)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	check(InOutSurfacePositions.Num() == Size.X * Size.Y * Size.Z);
	
	if (false)
	{
		const auto DataPtr = MakeVoxelShared<TArray<FVector3f>>(MoveTemp(InOutSurfacePositions));

		const auto Helper = MakeVoxelShared<FVoxelDistanceFieldShaderHelper>();
		Helper->StartCompute(Size, DataPtr, MaxPasses_Debug);
		Helper->WaitForCompletion();

		InOutSurfacePositions = MoveTemp(*DataPtr);
	}
	else
	{
		bool bUseTempAsSrc = false;
		
		TArray<FVector3f> Temp;
		Temp.Empty(InOutSurfacePositions.Num());
		Temp.SetNumUninitialized(InOutSurfacePositions.Num());
		
		const int32 PowerOfTwo = FMath::CeilLogTwo(Size.GetMax());
		for (int32 Pass = 0; Pass < PowerOfTwo; Pass++)
		{
			if (MaxPasses_Debug == Pass)
			{
				break;
			}
			
			// -1: we want to start with half the size
			const int32 Step = 1 << (PowerOfTwo - 1 - Pass);
			JumpFloodStep_CPU(
				Size, 
				bUseTempAsSrc ? Temp : InOutSurfacePositions,
				bUseTempAsSrc ? InOutSurfacePositions : Temp,
				Step,
				bMultiThreaded);

			bUseTempAsSrc = !bUseTempAsSrc;
		}

		if (bUseTempAsSrc)
		{
			InOutSurfacePositions = MoveTemp(Temp);
		}
	}
}

void FVoxelDistanceFieldUtilities::GetDistancesFromSurfacePositions(const FIntVector& Size, TArrayView<const FVector3f> SurfacePositions, TArrayView<float> InOutDistances)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	check(SurfacePositions.Num() == InOutDistances.Num());
	check(SurfacePositions.Num() == Size.X * Size.Y * Size.Z);
	
	for (int32 X = 0; X < Size.X; X++)
	{
		for (int32 Y = 0; Y < Size.Y; Y++)
		{
			for (int32 Z = 0; Z < Size.Z; Z++)
			{
				float& Distance = FVoxelUtilities::Get3D(InOutDistances, Size, X, Y, Z);

				const FVector3f SurfacePosition = FVoxelUtilities::Get3D(SurfacePositions, Size, X, Y, Z);
				ensureVoxelSlow(IsSurfacePositionValid(SurfacePosition));
				
				// Keep sign
				Distance = FVector3f::Distance(FVector3f(X, Y, Z), SurfacePosition) * FMath::Sign(Distance);
				ensureVoxelSlow(FMath::Abs(Distance) < Size.Size() * 2);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDistanceFieldUtilities::GetSurfacePositionsFromDensities(const FIntVector& Size, TArrayView<const float> Densities, TArrayView<float> OutDistances, TArrayView<FVector3f> OutSurfacePositions)
{
	GetSurfacePositionsFromDensities(Size, Densities, OutDistances, OutSurfacePositions, [](float F) { return F; });
}

void FVoxelDistanceFieldUtilities::GetSurfacePositionsFromDensities(const FIntVector& Size, TArrayView<const FVoxelValue> Densities, TArrayView<float> OutDistances, TArrayView<FVector3f> OutSurfacePositions)
{
	GetSurfacePositionsFromDensities(Size, Densities, OutDistances, OutSurfacePositions, [](FVoxelValue F) { return F.ToFloat(); });
}

void FVoxelDistanceFieldUtilities::GetSurfacePositionsFromDensities(const FIntVector& Size, TArrayView<const FVoxelValue> Densities, TArray<float>& OutDistances, TArray<FVector3f>& OutSurfacePositions)
{
	const int32 Num = Size.X * Size.Y * Size.Z;
	OutDistances.Empty(Num);
	OutDistances.SetNumUninitialized(Num);
	OutSurfacePositions.Empty(Num);
	OutSurfacePositions.SetNumUninitialized(Num);
	GetSurfacePositionsFromDensities(Size, Densities, TArrayView<float>(OutDistances), TArrayView<FVector3f>(OutSurfacePositions));
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDistanceFieldUtilities::DownSample(
	const FIntVector& Size, 
	TArrayView<const float> InDistances, 
	TArrayView<const FVector3f> InSurfacePositions, 
	TArrayView<float> OutDistances, 
	TArrayView<FVector3f> OutSurfacePositions, 
	int32 Divisor,
	bool bShrink)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();
	
	check(Divisor >= 1);
	check(InDistances.GetData() != OutDistances.GetData());
	check(InSurfacePositions.GetData() != OutSurfacePositions.GetData());

	const FIntVector LowSize = FVoxelUtilities::DivideCeil(Size, Divisor);

	for (int32 LowX = 0; LowX < LowSize.X; LowX++)
	{
		for (int32 LowY = 0; LowY < LowSize.Y; LowY++)
		{
			for (int32 LowZ = 0; LowZ < LowSize.Z; LowZ++)
			{
				float BestDistance = MAX_flt;
				FVector3f BestSurfacePosition = MakeInvalidSurfacePosition();
				float Sign = FVoxelUtilities::Get3D(InDistances, Size, LowX * Divisor, LowY * Divisor, LowZ * Divisor);
				
				for (int32 HighX = LowX * Divisor; HighX < FMath::Min(Size.X, (LowX + 1) * Divisor); HighX++)
				{
					for (int32 HighY = LowY * Divisor; HighY < FMath::Min(Size.Y, (LowY + 1) * Divisor); HighY++)
					{
						for (int32 HighZ = LowZ * Divisor; HighZ < FMath::Min(Size.Z, (LowZ + 1) * Divisor); HighZ++)
						{
							FVector3f NeighborSurfacePosition = FVoxelUtilities::Get3D(InSurfacePositions, Size, HighX, HighY, HighZ);
							
							if (IsSurfacePositionValid(NeighborSurfacePosition))
							{
								// Make sure to / Divisor AFTER IsSurfacePositionValid
								NeighborSurfacePosition /= Divisor;
								const float Distance = (NeighborSurfacePosition - FVector3f(LowX, LowY, LowZ)).SizeSquared();
								if (Distance < BestDistance)
								{
									BestDistance = Distance;
									BestSurfacePosition = NeighborSurfacePosition;
									Sign = FVoxelUtilities::Get3D(InDistances, Size, HighX, HighY, HighZ);
								}
							}
						}
					}
				}

				FVoxelUtilities::Get3D(OutSurfacePositions, LowSize, LowX, LowY, LowZ) = BestSurfacePosition;
				FVoxelUtilities::Get3D(OutDistances, LowSize, LowX, LowY, LowZ) = Sign;
			}
		}
	}
}

void FVoxelDistanceFieldUtilities::DownSample(
	FIntVector& Size,
	TArray<float>& Distances,
	TArray<FVector3f>& SurfacePositions,
	int32 Divisor,
	bool bShrink)
{
	ensure(Divisor >= 1);
	if (Divisor <= 1)
	{
		return;
	}
	
	const FIntVector LowSize = FVoxelUtilities::DivideCeil(Size, Divisor);

	const int32 NewSize = LowSize.X * LowSize.Y * LowSize.Z;
	
	TArray<float> NewDistances;
	TArray<FVector3f> NewSurfacePositions;
	NewDistances.Empty(NewSize);
	NewDistances.SetNumUninitialized(NewSize);
	NewSurfacePositions.Empty(NewSize);
	NewSurfacePositions.SetNumUninitialized(NewSize);

	DownSample(Size, Distances, SurfacePositions, NewDistances, NewSurfacePositions, Divisor, bShrink);

	Size = LowSize;
	Distances = MoveTemp(NewDistances);
	SurfacePositions = MoveTemp(NewSurfacePositions);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelDistanceFieldUtilities::JumpFloodStep_CPU(const FIntVector& Size, TArrayView<const FVector3f> InData, TArrayView<FVector3f> OutData, int32 Step, bool bMultiThreaded)
{
	VOXEL_ASYNC_FUNCTION_COUNTER();

	check(InData.Num() == OutData.Num());
	check(InData.Num() == Size.X * Size.Y * Size.Z);
	
	const auto DoWork = [&](int32 X)
	{
		for (int32 Y = 0; Y < Size.Y; Y++)
		{
			for (int32 Z = 0; Z < Size.Z; Z++)
			{
				const FIntVector Position(X, Y, Z);

				float BestDistance = MAX_flt;
				FVector3f BestSurfacePosition = MakeInvalidSurfacePosition();

				for (int32 DX = -1; DX <= 1; ++DX)
				{
					for (int32 DY = -1; DY <= 1; ++DY)
					{
						for (int32 DZ = -1; DZ <= 1; ++DZ)
						{
							const FIntVector NeighborPosition = Position + FIntVector(DX, DY, DZ) * Step;

							if (NeighborPosition.X < 0 ||
								NeighborPosition.Y < 0 ||
								NeighborPosition.Z < 0 ||
								NeighborPosition.X >= Size.X ||
								NeighborPosition.Y >= Size.Y ||
								NeighborPosition.Z >= Size.Z)
							{
								continue;
							}

							const FVector3f NeighborSurfacePosition = FVoxelUtilities::Get3D(InData, Size, NeighborPosition);

							if (IsSurfacePositionValid(NeighborSurfacePosition))
							{
								const float Distance = (NeighborSurfacePosition - FVector3f(Position)).SizeSquared();
								if (Distance < BestDistance)
								{
									BestDistance = Distance;
									BestSurfacePosition = NeighborSurfacePosition;
								}
							}
						}
					}
				}
				FVoxelUtilities::Get3D(OutData, Size, Position) = BestSurfacePosition;
			}
		}
	};

	if (bMultiThreaded)
	{
		ParallelFor(Size.X, DoWork);
	}
	else
	{
		for (int32 X = 0; X < Size.X; X++)
		{
			DoWork(X);
		}
	}
}