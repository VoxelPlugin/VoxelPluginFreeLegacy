// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CollisionQueryParams.h"
#include "VoxelTools/VoxelSurfaceTools.h"
#include "VoxelProjectionTools.generated.h"

class AVoxelWorld;

USTRUCT(BlueprintType, meta = (HasNativeMake="Voxel.VoxelProjectionTools.MakeVoxelLineTraceParameters"))
struct FVoxelLineTraceParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECollisionChannel::ECC_WorldDynamic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	TArray<TEnumAsByte<ECollisionChannel>> CollisionChannelsToIgnore;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	TArray<AActor*> ActorsToIgnore;
	
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Voxel")
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType = EDrawDebugTrace::None;
	
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Voxel")
	FLinearColor TraceColor = FLinearColor::Red;
	
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Voxel")
	FLinearColor TraceHitColor = FLinearColor::Green;
	
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Voxel")
	float DrawTime = 5.0f;

	FCollisionQueryParams GetParams() const;
	FCollisionResponseContainer GetResponseContainer() const;
	void DrawDebug(const UWorld* World, const FVector& Start, const FVector& End, bool bHit, const FHitResult& OutHit) const;
};

UENUM(BlueprintType)
enum class EVoxelProjectionShape : uint8
{
	Circle,
	Square
};

USTRUCT(BlueprintType)
struct FVoxelProjectionHit
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FIntVector VoxelPosition = FIntVector(ForceInit);

	// Position on the projection plane
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVector2D PlanePosition = FVector2D(ForceInit);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FHitResult Hit = FHitResult(ForceInit);
};

UCLASS()
class VOXEL_API UVoxelProjectionTools : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Needed because else we need to add 2 MakeArrays node...
	
	// Make voxel line trace parameters
	UFUNCTION(BlueprintPure, Category = "Voxel", meta = (AdvancedDisplay = "CollisionChannelsToIgnore, ActorsToIgnore, DrawDebugType, TraceColor, TraceHitColor, DrawTime", AutoCreateRefTerm = "CollisionChannelsToIgnore, ActorsToIgnore"))
	static FVoxelLineTraceParameters MakeVoxelLineTraceParameters(
		TArray<TEnumAsByte<ECollisionChannel>> CollisionChannelsToIgnore,
		TArray<AActor*> ActorsToIgnore,
		TEnumAsByte<ECollisionChannel> CollisionChannel = ECollisionChannel::ECC_WorldDynamic,
		TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType = EDrawDebugTrace::None,
		FLinearColor TraceColor = FLinearColor::Red,
		FLinearColor TraceHitColor = FLinearColor::Green,
		float DrawTime = 5.0f);

public:
	/**
	 * Find voxels using linetraces
	 * @param World					The voxel world
	 * @param Parameters			Linetraces parameters
	 * @param Position				The center of the linetraces
	 * @param Direction				The direction of the linetraces
	 * @param Radius				The radius in world space (cm)
	 * @param Shape					The shape of the rays start positions
	 * @param NumRays				The approximate number of rays to trace
	 * @param MaxDistance			The max ray distance
	 * @return	Number of rays actually traced (should be close to NumRays)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Projection Tools", meta = (DefaultToSelf = "World"))
	static int32 FindProjectionVoxels(
		TArray<FVoxelProjectionHit>& Hits,
		AVoxelWorld* World,
		FVoxelLineTraceParameters Parameters,
		FVector Position,
		FVector Direction,
		float Radius = 100.f,
		EVoxelProjectionShape Shape = EVoxelProjectionShape::Circle,
		float NumRays = 100.f,
		float MaxDistance = 1e9);
	
	/**
	 * Find voxels using linetraces, asynchronously
	 * @param World					The voxel world
	 * @param Parameters			Linetraces parameters
	 * @param Position				The center of the linetraces
	 * @param Direction				The direction of the linetraces
	 * @param Radius				The radius in world space (cm)
	 * @param Shape					The shape of the rays start positions
	 * @param NumRays				The approximate number of rays to trace
	 * @param MaxDistance			The max ray distance
	 * @return	Number of rays actually traced (should be close to NumRays)
	 */
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Projection Tools", meta = (DefaultToSelf = "World", Latent, LatentInfo="LatentInfo", WorldContext = "WorldContextObject", AdvancedDisplay = "bHideLatentWarnings"))
	static int32 FindProjectionVoxelsAsync(
		UObject* WorldContextObject,
		FLatentActionInfo LatentInfo,
		TArray<FVoxelProjectionHit>& Hits,
		AVoxelWorld* World,
		FVoxelLineTraceParameters Parameters,
		FVector Position,
		FVector Direction,
		float Radius = 100.f,
		EVoxelProjectionShape Shape = EVoxelProjectionShape::Circle,
		float NumRays = 100.f,
		float MaxDistance = 1e9,
		bool bHideLatentWarnings = false);

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Projection Tools")
	static TArray<FIntVector> GetHitsPositions(const TArray<FVoxelProjectionHit>& Hits);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Projection Tools")
	static FVector GetHitsAverageNormal(const TArray<FVoxelProjectionHit>& Hits);
	
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Projection Tools")
	static FVector GetHitsAveragePosition(const TArray<FVoxelProjectionHit>& Hits);

	// For some surface tools you'll need to use CreateSurfaceVoxelsFromHitsWithExactValues instead
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Projection Tools")
	static FVoxelSurfaceEditsVoxels CreateSurfaceVoxelsFromHits(const TArray<FVoxelProjectionHit>& Hits);

	// Will store the voxel values
	UFUNCTION(BlueprintCallable, Category = "Voxel|Tools|Projection Tools", meta = (DefaultToSelf = "World"))
	static FVoxelSurfaceEditsVoxels CreateSurfaceVoxelsFromHitsWithExactValues(AVoxelWorld* World, const TArray<FVoxelProjectionHit>& Hits);
	
public:
	// NumRays might be slightly lower than the actual number of traced rays
	// Returns num rays actually traced
	template<typename T>
	static int32 GenerateRays(
		const FVector& Position, 
		const FVector& Direction, 
		const float Radius,
		const EVoxelProjectionShape Shape,
		const float NumRays,
		const float MaxDistance,
		T Lambda)
	{
		if (!ensure(Direction.IsNormalized())) return 0;
		if (NumRays <= 0) return 0;
		
		const auto GetTangent = [](const FVector& N)
		{
			// Compute tangent
			// N dot T = 0
			// <=> N.X * T.X + N.Y * T.Y + N.Z * T.Z = 0
			// <=> T.Z = -1 / N.Z * (N.X * T.X + N.Y * T.Y) if N.Z != 0
			if (N.Z != 0)
			{
				return FVector(1, 1, -1 / double(N.Z) * (N.X + N.Y)).GetSafeNormal();
			}
			else
			{
				return FVector(0, 0, 1);
			}
		};

		const FVector Tangent = GetTangent(Direction);
		const FVector BiTangent = FVector::CrossProduct(Tangent, Direction).GetSafeNormal();
		// NumRays is the area; get the radius we would get from such area
		const float NumRaysInRadius =
			Shape == EVoxelProjectionShape::Circle
			? FMath::Sqrt(NumRays / PI)
			: FMath::Sqrt(NumRays) / 2;
		const int32 Count = FMath::CeilToInt(NumRaysInRadius);
		const float RadiusSquared = FMath::Square(Radius);

		int32 NumRaysActuallyTraced = 0;
		for (int32 U = -Count; U <= Count; U++)
		{
			for (int32 V = -Count; V <= Count; V++)
			{
				const FVector2D PlanePosition = FVector2D(U, V) * Radius / Count;
				if (Shape == EVoxelProjectionShape::Circle && PlanePosition.SizeSquared() >= RadiusSquared)
				{
					continue;
				}

				const FVector Start = Position + (Tangent * PlanePosition.X + BiTangent * PlanePosition.Y);
				const FVector End = Start + Direction * MaxDistance;
				Lambda(Start, End, PlanePosition);
				NumRaysActuallyTraced++;
			}
		}
		return NumRaysActuallyTraced;
	}
};