// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelSpawners/VoxelInstancedMeshSettings.h"
#include "VoxelMeshSpawner.generated.h"

class UStaticMesh;
class UMaterialInterface;

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
	
	FVector GetScale(const FRandomStream& Stream) const
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
	RandomAlign
};

UENUM()
enum class EVoxelMeshSpawnerInstanceRandom : uint8
{
	// Random number
	// Use GetVoxelSpawnerActorInstanceRandom to get it
	// Will have the same value in the spawned actor as in the instance
	Random,
	// Get the voxel material in the shader
	// Use GetVoxelMaterialFromInstanceRandom
	VoxelMaterial,
	// Get a voxel graph output color in the shader
	// Use GetColorFromInstanceRandom
	ColorOutput
};

UCLASS()
class VOXEL_API UVoxelMeshSpawner : public UObject
{
	GENERATED_BODY()

public:
	// Mesh to spawn. Can be left to null if AlwaysSpawnActor is true
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Settings")
	UStaticMesh* Mesh = nullptr;

	// Per mesh section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "General Settings")
	TMap<int32, UMaterialInterface*> MaterialsOverrides;
	
	// Average distance between the instances, in voxels
	// Num Instances = Area in voxels / Square(DistanceBetweenInstancesInVoxel)
	// Not a density because the values would be too small to store in a float
	UPROPERTY(EditAnywhere, Category = "General Settings", meta = (ClampMin = 0))
	float DistanceBetweenInstancesInVoxel = 10;

	// What to send through InstanceRandom
	// Check enum values tooltips
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Settings")
	EVoxelMeshSpawnerInstanceRandom InstanceRandom = EVoxelMeshSpawnerInstanceRandom::Random;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General Settings")
	FName ColorOutputName;

	// If true, will divide DistanceBetweenInstancesInVoxel by the VoxelSize before using it
	UPROPERTY(EditAnywhere, Category = "General Settings", AdvancedDisplay)
	bool bDistanceBetweenInstancesIsInCm = false;
	
public:
	// Actor to spawn to replace the instanced mesh. After spawn, the SetStaticMesh event will be called on the actor with Mesh as argument
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Settings", meta = (ShowOnlyInnerProperties))
	FVoxelSpawnerActorSettings ActorSettings;

	// Will always spawn an actor instead of an instanced mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Actor Settings")
	bool bAlwaysSpawnActor = false;

public:
	UPROPERTY(EditAnywhere, Category = "Instance Settings", meta = (ShowOnlyInnerProperties))
	FVoxelInstancedMeshSettings InstancedMeshSettings;

public:
	// Min/max angle between object up vector and generator up vector in degrees
	UPROPERTY(EditAnywhere, Category = "Placement", meta = (UIMin = 0, ClampMin = 0, UIMax = 180, ClampMax = 180))
	FFloatInterval GroundSlopeAngle = { 0, 90 };

	UPROPERTY(EditAnywhere, Category = "Placement", meta = (InlineEditConditionToggle))
	bool bEnableHeightRestriction = false;
	
	// In voxels. Only spawn instances if the instance voxel Z position is in this interval.
	// TODO: optimize to not generate chunks that do not match this restriction
	UPROPERTY(EditAnywhere, Category = "Placement", meta = (EditCondition = "bEnableHeightRestriction"))
	FFloatInterval HeightRestriction = { -100.f, 100.f };

	// In voxels, the size of the fade on the edges of HeightRestriction
	UPROPERTY(EditAnywhere, Category = "Placement", meta = (EditCondition = "bEnableHeightRestriction", UIMin = 0, ClampMin = 0))
	float HeightRestrictionFalloff = 0.f;
	
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

	// Apply an offset to the instance position. Applied before the rotation. In cm
	UPROPERTY(EditAnywhere, Category = "Placement - Offset")
	FVector LocalPositionOffset = FVector::ZeroVector;
	
	// Apply an offset to the instance rotation. Applied after the local position offset, and before the rotation
	UPROPERTY(EditAnywhere, Category = "Placement - Offset")
	FRotator LocalRotationOffset = FRotator::ZeroRotator;
	
	// Apply an offset to the instance position. Applied after the rotation. In cm
	UPROPERTY(EditAnywhere, Category = "Placement - Offset")
	FVector GlobalPositionOffset = FVector::ZeroVector;
	
	// In local space. Increase this if your foliage is enabling physics too soon. In cm
	UPROPERTY(EditAnywhere, Category = "Placement - Offset")
	FVector FloatingDetectionOffset = FVector(0, 0, -10);

protected:
	//~ Begin UObject Interface
	virtual void Serialize(FArchive& Ar) override;
	//~ End UObject Interface

public:
#if WITH_EDITOR
	bool NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent) const;
#endif
};