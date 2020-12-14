// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelDistance.h"
#include "VoxelSpawners/VoxelFoliageDensity.h"
#include "VoxelSpawners/VoxelInstancedMeshSettings.h"
#include "VoxelGenerators/VoxelGeneratorOutputPicker.h"
#include "VoxelFoliage.generated.h"

UENUM()
enum class EVoxelFoliageSpawnType
{
	// Will line trace the voxel geometry to find spawning locations. Works with any kind of world/shapes
	Ray,
	// These spawners uses a height output from the generator to spawn, allowing for large spawn distance.
	Height
};

UENUM()
enum class EVoxelFoliageRandomGenerator : uint8
{
	// Evenly distributed points
	Sobol,
	// More uneven points than Sobol. Unreal uses Halton to spawn grass in the default Landscape system
	Halton
};

UENUM()
enum class EVoxelFoliageRotation : uint8
{
	AlignToSurface,
	AlignToWorldUp,
	RandomAlign
};

UENUM()
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

UCLASS(Abstract, Blueprintable, EditInlineNew)
class VOXEL_API UVoxelFoliage : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Foliage")
	UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Foliage")
	TArray<UMaterialInterface*> Materials;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Foliage")
	TSubclassOf<AVoxelFoliageActor> ActorClass;
	
	// Used to autocomplete generator output names
	UPROPERTY(EditAnywhere, Category = "Voxel Foliage")
	FVoxelGeneratorPicker MainGeneratorForDropdowns;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel Foliage")
	FVoxelInstancedMeshSettings InstancedMeshSettings;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	EVoxelFoliageSpawnType SpawnType = EVoxelFoliageSpawnType::Ray;
	
	// Average distance between the instances
	// Num Instances = Area in voxels / Square(DistanceBetweenInstancesInVoxel)
	// Not a density because the values would be too small to store in a float
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FVoxelDistance DistanceBetweenInstances = FVoxelDistance::Voxels(10);
	
	// Chunk size
	// SpawnType = Height: make that as big as possible
	// SpawnType = Ray: bigger values will improve performance at the cost of accuracy, as a lower resolution mesh will be used
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta = (DisplayName = "Chunk Size", ClampMin = 32))
	int32 ChunkSize = 32;

	// Generation distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "Spawn", meta = (DisplayName = "Generation Distance"))
	FVoxelDistance GenerationDistance = FVoxelDistance::Voxels(32);

	// The final density will be the multiplication of all of these
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TArray<FVoxelFoliageDensity> Densities;
	
	// Controls the spawning pattern
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	EVoxelFoliageRandomGenerator RandomGenerator = EVoxelFoliageRandomGenerator::Halton;

	// Will spawn foliage on the entire map, with no distance limit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Spawn")
	bool bInfiniteGenerationDistance = false;

	// The name of the custom graph output used to determine the height
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn|Height", meta = (DisplayName = "Height Graph Output Name (height only)"))
	FVoxelGeneratorOutputPicker HeightGraphOutputName_HeightOnly = "Height";
		
	// Controls whether to compute the density or the height first. Try both and see which is faster
	// If false, the following are true when querying the density:
	// - for flat worlds: Z = Height
	// - for sphere worlds: Length(X, Y, Z) = Height
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Spawn|Height", meta = (DisplayName = "Compute Density First (height only)"))
	bool bComputeDensityFirst_HeightOnly = false;

	// If true, will not spawn height instances if they are now floating due to user edits or additional 3D noise in the generator
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Spawn|Height", meta = (DisplayName = "Check If Floating (height only)"))
	bool bCheckIfFloating_HeightOnly = true;

	// If true, will not spawn height instances if they are now covered due to user edits or additional 3D noise in the generator
	UPROPERTY(EditAnywhere, BlueprintReadWrite, AdvancedDisplay, Category = "Spawn|Height", meta = (DisplayName = "Check If Covered (height only)"))
	bool bCheckIfCovered_HeightOnly = true;

public:
	// Min/max angle between object up vector and generator up vector in degrees
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement - Slope", meta = (UIMin = 0, ClampMin = 0, UIMax = 180, ClampMax = 180))
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save")
	FGuid Guid;

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel Foliage")
	FVoxelInstancedMeshKey GetMeshKey() const;

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