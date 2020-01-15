// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSpawners/VoxelSpawner.h"
#include "VoxelBasicSpawner.generated.h"

UENUM()
enum class EVoxelBasicSpawnerScaling : uint8
{
	/** Instances will have uniform X, Y and Z scales */
	Uniform,
	/** Instances will have random X, Y and Z scales */
	Free,
	/** X and Y will be the same random scale, Z will be another */
	LockXY
};

USTRUCT()
struct FVoxelBasicSpawnerScaleSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Config")
	EVoxelBasicSpawnerScaling Scaling = EVoxelBasicSpawnerScaling::Uniform;

	/** Specifies the range of scale, from minimum to maximum, to apply to an actor instance's X Scale property */
	UPROPERTY(EditAnywhere, Category = "Config")
	FFloatInterval ScaleX = FFloatInterval(1.0f, 1.0f);

	/** Specifies the range of scale, from minimum to maximum, to apply to an actor instance's Y Scale property */
	UPROPERTY(EditAnywhere, Category = "Config")
	FFloatInterval ScaleY = FFloatInterval(1.0f, 1.0f);

	/** Specifies the range of scale, from minimum to maximum, to apply to an actor instance's Z Scale property */
	UPROPERTY(EditAnywhere, Category = "Config")
	FFloatInterval ScaleZ = FFloatInterval(1.0f, 1.0f);
	
	inline FVector GetScale(const FRandomStream& Stream) const
	{
		FVector Scale;
		switch (Scaling)
		{
		case EVoxelBasicSpawnerScaling::Uniform:
			Scale.X = ScaleX.Interpolate(Stream.GetFraction());
			Scale.Y = Scale.X;
			Scale.Z = Scale.X;
			break;
		case EVoxelBasicSpawnerScaling::Free:
			Scale.X = ScaleX.Interpolate(Stream.GetFraction());
			Scale.Y = ScaleY.Interpolate(Stream.GetFraction());
			Scale.Z = ScaleZ.Interpolate(Stream.GetFraction());
			break;
		case EVoxelBasicSpawnerScaling::LockXY:
			Scale.X = ScaleX.Interpolate(Stream.GetFraction());
			Scale.Y = Scale.X;
			Scale.Z = ScaleZ.Interpolate(Stream.GetFraction());
			break;
		default:
			check(false);
		}
		return Scale;
	}
};

UENUM()
enum class EVoxelBasicSpawnerRotation : uint8
{
	AlignToSurface,
	AlignToWorldUp,
	DoNotAlign
};

UCLASS(Abstract)
class VOXEL_API UVoxelBasicSpawner : public UVoxelSpawner
{
	GENERATED_BODY()

public:
	// Min/max angle between object up vector and world generator up vector in degrees
	UPROPERTY(EditAnywhere, Category = "Placement", meta = (UIMin = 0, ClampMin = 0, UIMax = 180, ClampMax = 180))
	FFloatInterval GroundSlopeAngle = { 0, 90 };
	
	// Specifies instance scaling type
	UPROPERTY(EditAnywhere, Category = "Placement - Scale")
	FVoxelBasicSpawnerScaleSettings Scaling;

	// Vertical to use for the instances
	UPROPERTY(EditAnywhere, Category = "Placement - Rotation")
	EVoxelBasicSpawnerRotation RotationAlignment = EVoxelBasicSpawnerRotation::AlignToWorldUp;

	// If selected, foliage instances will have a random yaw rotation around their vertical axis applied
	UPROPERTY(EditAnywhere, Category = "Placement - Rotation")
	bool bRandomYaw = true;

	// A random pitch adjustment can be applied to each instance, up to the specified angle in degrees, from the original vertical
	UPROPERTY(EditAnywhere, Category = "Placement - Rotation", meta = (UIMin = 0, ClampMin = 0, UIMax = 180, ClampMax = 180))
	float RandomPitchAngle = 6;

	// Apply an offset to the instance position. Applied before the rotation offset
	UPROPERTY(EditAnywhere, Category = "Placement - Offset")
	FVector PositionOffset = FVector::ZeroVector;
	
	// Apply an offset to the instance rotation. Applied after the position offset
	UPROPERTY(EditAnywhere, Category = "Placement - Offset")
	FRotator RotationOffset = FRotator::ZeroRotator;
};

