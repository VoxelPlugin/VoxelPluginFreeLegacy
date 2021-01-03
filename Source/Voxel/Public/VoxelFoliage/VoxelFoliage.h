// Copyright 2021 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelFoliage/VoxelFoliageDensity.h"
#include "VoxelFoliage/VoxelFoliageSpawnSettings.h"
#include "VoxelFoliage/VoxelInstancedMeshSettings.h"
#include "VoxelFoliage.generated.h"

UENUM(BlueprintType)
enum class EVoxelFoliageRotation : uint8
{
	AlignToSurface,
	AlignToWorldUp,
	RandomAlign
};

UENUM(BlueprintType)
enum class EVoxelFoliageScaling : uint8
{
	/** Instances will have uniform X, Y and Z scales */
	Uniform,
	/** Instances will have random X, Y and Z scales */
	Free,
	/** X and Y will be the same random scale, Z will be another */
	LockXY
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelFoliageScale
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EVoxelFoliageScaling Scaling = EVoxelFoliageScaling::Uniform;

	/** Specifies the range of scale, from minimum to maximum, to apply to an actor instance's X Scale property */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVoxelFloatInterval ScaleX = { 1.0f, 1.0f };

	/** Specifies the range of scale, from minimum to maximum, to apply to an actor instance's Y Scale property */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVoxelFloatInterval ScaleY = { 1.0f, 1.0f };

	/** Specifies the range of scale, from minimum to maximum, to apply to an actor instance's Z Scale property */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVoxelFloatInterval ScaleZ = { 1.0f, 1.0f };
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelFoliageMesh
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Foliage")
	UStaticMesh* Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Foliage")
	TArray<UMaterialInterface*> Materials;

	// Relative to the other strength in this array - they will be normalized
	// Has no impact if the array has only one element
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Foliage", meta = (UIMin = 0, UIMax = 1))
	float Strength = 1.f;
};

UCLASS(Blueprintable)
class VOXEL_API UVoxelFoliage : public UObject
{
	GENERATED_BODY()

public:
	// The meshes to use - if you use multiple ones, the hits will be split among them based on their strength
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Foliage")
	TArray<FVoxelFoliageMesh> Meshes;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Foliage")
	TSubclassOf<AVoxelFoliageActor> ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Foliage")
	FVoxelInstancedMeshSettings InstanceSettings;

public:
	// Used to autocomplete generator output names
	UPROPERTY(EditAnywhere, Category = "Voxel Foliage", meta = (DisplayName = "Generator (for autocomplete only)"))
	FVoxelGeneratorPicker OutputPickerGenerator;

public:
	// The final density will be the multiplication of all of these
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TArray<FVoxelFoliageDensity> Densities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (ShowOnlyInnerProperties))
	FVoxelFoliageSpawnSettings SpawnSettings;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement - Slope", meta = (InlineEditConditionToggle))
	bool bEnableSlopeRestriction = true;
	
	// Min/max angle between object up vector and generator up vector in degrees
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement - Slope", meta = (EditCondition = "bEnableSlopeRestriction", UIMin = 0, ClampMin = 0, UIMax = 180, ClampMax = 180))
	FVoxelFloatInterval GroundSlopeAngle = { 0, 90 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement - Height", meta = (InlineEditConditionToggle))
	bool bEnableHeightRestriction = false;
	
	// In voxels. Only spawn instances if the instance voxel Z position is in this interval.
	// TODO: optimize to not generate chunks that do not match this restriction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement - Height", meta = (EditCondition = "bEnableHeightRestriction"))
	FVoxelFloatInterval HeightRestriction = { -100.f, 100.f };

	// In voxels, the size of the fade on the edges of HeightRestriction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement - Height", meta = (EditCondition = "bEnableHeightRestriction", UIMin = 0, ClampMin = 0))
	float HeightRestrictionFalloff = 0.f;
	
	// Specifies instance scaling type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement - Scale")
	FVoxelFoliageScale Scaling;

	// Vertical to use for the instances
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement - Rotation")
	EVoxelFoliageRotation RotationAlignment = EVoxelFoliageRotation::AlignToWorldUp;

	// If selected, foliage instances will have a random yaw rotation around their vertical axis applied
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement - Rotation")
	bool bRandomYaw = true;

	// A random pitch adjustment can be applied to each instance, up to the specified angle in degrees, from the original vertical
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement - Rotation", meta = (UIMin = 0, ClampMin = 0, UIMax = 180, ClampMax = 180))
	float RandomPitchAngle = 6;

	// Apply an offset to the instance position. Applied before the rotation. In cm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement - Offset")
	FVector LocalPositionOffset = FVector::ZeroVector;
	
	// Apply an offset to the instance rotation. Applied after the local position offset, and before the rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement - Offset")
	FRotator LocalRotationOffset = FRotator::ZeroRotator;
	
	// Apply an offset to the instance position. Applied after the rotation. In cm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement - Offset")
	FVector GlobalPositionOffset = FVector::ZeroVector;
	
	// In local space. Increase this if your foliage is enabling physics too soon. In cm
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement - Offset")
	FVector FloatingDetectionOffset = FVector(0, 0, -10);

public:
	// Whether to save the instances that are removed
	// If false will also respawn instances if they are out of range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	bool bSave = true;

	// If false, instances that are out of range will be despawned. If true, they will stay forever.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	bool bDoNotDespawn = false;

	// Also used as a seed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FGuid Guid;

public:
	//~ Begin UObject Interface
	virtual void PostDuplicate(bool bDuplicateForPIE) override;
#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject Interface

#if WITH_EDITOR
	bool NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent) const;
#endif
};