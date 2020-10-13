// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"
#include "VoxelEnums.h"
#include "VoxelSpawnerConfig.generated.h"

class UVoxelSpawner;
class FVoxelGeneratorInstance;
class UVoxelSpawnerConfig;
class UVoxelSpawnerOutputsConfig;

USTRUCT()
struct FVoxelSpawnerOutputName
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Voxel")
	FName Name;

	FVoxelSpawnerOutputName() = default;
	
	template<typename... TArgs>
	FVoxelSpawnerOutputName(TArgs... Args)
		: Name(Forward<TArgs>(Args)...)
	{
	}

	inline operator FName() const
	{
		return Name;
	}
	inline bool IsNone() const
	{
		return Name.IsNone();
	}
};

UENUM()
enum class EVoxelSpawnerDensityType : uint8
{
	// Use a constant as density
	Constant,
	// Use a generator output
	GeneratorOutput,
	// Use one of the material RGBA channels. Only for Ray Spawners.
	MaterialRGBA,
	// Use the material UV channels. Only for Ray Spawners.
	MaterialUVs,
	// Use a five way blend strength. Only for Ray Spawners.
	MaterialFiveWayBlend,
	// Use a single index channel. Only for Ray Spawners.
	SingleIndex,
	// Use a multi index channel. Only for Ray Spawners.
	MultiIndex
};

UENUM()
enum class EVoxelSpawnerUVAxis
{
	U,
	V
};

UENUM()
enum class EVoxelSpawnerDensityTransform
{
	Identity UMETA(DisplayName = "None"),
	OneMinus UMETA(DisplayName = "1 - X"),
};

USTRUCT()
struct FVoxelSpawnerDensity
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Voxel")
	EVoxelSpawnerDensityType Type = EVoxelSpawnerDensityType::Constant;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	float Constant = 1.f;
	
	// Your generator needs to have a float output named like this
	UPROPERTY(EditAnywhere, Category = "Voxel")
	FVoxelSpawnerOutputName GeneratorOutputName;

	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (DisplayName = "RGBA Channel"))
	EVoxelRGBA RGBAChannel;
	
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (DisplayName = "UV Channel", ClampMin = 0, ClampMax = 3))
	int32 UVChannel;

	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (DisplayName = "UV Axis"))
	EVoxelSpawnerUVAxis UVAxis;
	
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (ClampMin = 0, ClampMax = 4))
	int32 FiveWayBlendChannel = 0;
	
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (ClampMin = 0, ClampMax = 255))
	TArray<int32> SingleIndexChannels = { 0 };
	
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (ClampMin = 0, ClampMax = 255))
	TArray<int32> MultiIndexChannels = { 0 };

	// Transform to apply to the density
	UPROPERTY(EditAnywhere, Category = "Voxel")
	EVoxelSpawnerDensityTransform Transform = EVoxelSpawnerDensityTransform::Identity;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UENUM()
enum class EVoxelSpawnerConfigElementRandomGenerator : uint8
{
	// Evenly distributed points
	Sobol,
	// More uneven points than Sobol. Unreal uses Halton to spawn grass in the default Landscape system
	Halton
};

UENUM()
enum class EVoxelSpawnerType
{
	// Will line trace the voxel geometry to find spawning locations. Works with any kind of world/shapes
	Ray,
	// These spawners uses a height output from the generator to spawn, allowing for large spawn distance.
	Height
};

USTRUCT()
struct FVoxelSpawnerConfigSpawner
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Voxel")
	UVoxelSpawner* Spawner = nullptr;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	EVoxelSpawnerType SpawnerType = EVoxelSpawnerType::Ray;
	
public:
	UPROPERTY(EditAnywhere, Category = "Voxel")
	FVoxelSpawnerDensity Density;

	// Final Density = Density * DensityMultiplier. Use this to eg paint an Erase Foliage channel.
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (DisplayName = "Density Multiplier"))
	FVoxelSpawnerDensity DensityMultiplier_RayOnly;

	// The name of the custom graph output used to determine the height
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (DisplayName = "Height Graph Output Name"))
	FVoxelSpawnerOutputName HeightGraphOutputName_HeightOnly = "Height";
	
public:
	// Chunk size, affects the LOD if ray spawner
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (DisplayName = "Chunk Size"))
	uint32 ChunkSize_EditorOnly = 32;

	// The LOD of the mesh to trace rays against
	// High LOD = faster but less precise
	UPROPERTY(VisibleAnywhere, Category = "Voxel")
	int32 LOD = 0;

	// Generation distance in voxels
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (DisplayName = "Generation Distance"))
	uint32 GenerationDistanceInVoxels_EditorOnly = 0;
	
	UPROPERTY(VisibleAnywhere, Category = "Voxel")
	int32 GenerationDistanceInChunks = 2;

	UPROPERTY()
	bool bInfiniteGenerationDistance = false;

public:
	// Whether to save the instances that are removed
	// If false will also respawn instances if they are out of range
	UPROPERTY(EditAnywhere, Category = "Voxel")
	bool bSave = true;

	// If false, instances that are out of range will be despawned. If true, they will stay forever.
	UPROPERTY(EditAnywhere, Category = "Voxel")
	bool bDoNotDespawn = false;

	// Seed for this spawner. Note that changing this is not required to get unique results per spawner.
	UPROPERTY(EditAnywhere, Category = "Voxel")
	int32 Seed = 1337;
	
public:
	// Controls the spawning pattern
	UPROPERTY(EditAnywhere, Category = "Voxel")
	EVoxelSpawnerConfigElementRandomGenerator RandomGenerator = EVoxelSpawnerConfigElementRandomGenerator::Halton;

	// Unique ID used when saving spawners to disk
	UPROPERTY(VisibleAnywhere, Category = "Voxel")
	FGuid Guid;
		
	// Controls whether to compute the density or the height first. Try both and see which is faster
	// If false, the following are true when querying the density:
	// - for flat worlds: Z = Height
	// - for sphere worlds: Length(X, Y, Z) = Height
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (DisplayName = "Compute Density First"))
	bool bComputeDensityFirst_HeightOnly = false;

	// If true, will not spawn height instances if they are now floating due to user edits or additional 3D noise in the generator
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (DisplayName = "Check If Floating"))
	bool bCheckIfFloating_HeightOnly = true;

	// If true, will not spawn height instances if they are now covered due to user edits or additional 3D noise in the generator
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (DisplayName = "Check If Covered"))
	bool bCheckIfCovered_HeightOnly = true;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

USTRUCT()
struct FVoxelSpawnerConfigElementAdvanced_Height
{
	GENERATED_BODY()

	UPROPERTY()
	bool bSave = true;

	UPROPERTY()
	bool bDoNotDespawn = false;
	
	UPROPERTY()
	FName SeedName = "FoliageSeed";

	UPROPERTY()
	uint32 DefaultSeed = 1337;

	UPROPERTY()
	EVoxelSpawnerConfigElementRandomGenerator RandomGenerator = EVoxelSpawnerConfigElementRandomGenerator::Halton;
		
	UPROPERTY()
	bool bComputeDensityFirst = false;

	UPROPERTY()
	FGuid Guid;
};

USTRUCT()
struct FVoxelSpawnerConfigElementAdvanced_Ray
{
	GENERATED_BODY()

	UPROPERTY()
	bool bSave = true;

	UPROPERTY()
	bool bDoNotDespawn = false;

	UPROPERTY()
	FName SeedName = "FoliageSeed";

	UPROPERTY()
	uint32 DefaultSeed = 1337;

	UPROPERTY()
	EVoxelSpawnerConfigElementRandomGenerator RandomGenerator = EVoxelSpawnerConfigElementRandomGenerator::Halton;

	UPROPERTY()
	FGuid Guid;
};

USTRUCT()
struct FVoxelSpawnerConfigElement_Height
{
	GENERATED_BODY()
	
	UPROPERTY()
	UVoxelSpawner* Spawner = nullptr;

	UPROPERTY()
	FVoxelSpawnerDensity Density;
	
	UPROPERTY()
	FVoxelSpawnerOutputName DensityGraphOutputName_DEPRECATED;

	UPROPERTY()
	FVoxelSpawnerConfigElementAdvanced_Height Advanced;
};

USTRUCT()
struct FVoxelSpawnerConfigElement_Ray
{
	GENERATED_BODY()
	
	UPROPERTY()
	UVoxelSpawner* Spawner = nullptr;

	UPROPERTY()
	FVoxelSpawnerDensity Density;

	UPROPERTY()
	FVoxelSpawnerDensity DensityMultiplier;
	
	UPROPERTY()
	FVoxelSpawnerOutputName DensityGraphOutputName_DEPRECATED;

	UPROPERTY()
	FVoxelSpawnerConfigElementAdvanced_Ray Advanced;
};

USTRUCT()
struct FVoxelSpawnerConfigHeightGroup
{
	GENERATED_BODY()

	UPROPERTY()
	FVoxelSpawnerOutputName HeightGraphOutputName = "Height";

	UPROPERTY()
	uint32 ChunkSize = 32;

	UPROPERTY()
	uint32 GenerationDistanceInChunks = 2;

	UPROPERTY()
	uint32 GenerationDistanceInVoxels_EditorOnly = 0;

	UPROPERTY()
	TArray<FVoxelSpawnerConfigElement_Height> Spawners;
};

USTRUCT()
struct FVoxelSpawnerConfigRayGroup
{
	GENERATED_BODY()

	UPROPERTY()
	uint32 LOD = 0;

	UPROPERTY()
	uint32 ChunkSize_EditorOnly = 32;
	
	UPROPERTY()
	uint32 GenerationDistanceInChunks = 2;

	UPROPERTY()
	uint32 GenerationDistanceInVoxels_EditorOnly = 0;
	
	UPROPERTY()
	TArray<FVoxelSpawnerConfigElement_Ray> Spawners;
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

UENUM()
enum class EVoxelSpawnerConfigRayWorldType : uint8
{
	Flat,
	Sphere
};

USTRUCT(BlueprintType)
struct FVoxelSpawnerConfigFiveWayBlendSetup
{
	GENERATED_BODY()

	// If true, will ignore Alpha
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Voxel")
	bool bFourWayBlend = false;
};

UCLASS()
class VOXEL_API UVoxelSpawnerConfig : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	EVoxelSpawnerConfigRayWorldType WorldType;
	
	UPROPERTY(EditAnywhere, Category = "Config")
	UVoxelSpawnerOutputsConfig* GeneratorOutputs;
	
	UPROPERTY(EditAnywhere, Category = "Config", AdvancedDisplay)
	FVoxelSpawnerConfigFiveWayBlendSetup FiveWayBlendSetup;

public:
	UPROPERTY(EditAnywhere, Category = "Spawners")
	TArray<FVoxelSpawnerConfigSpawner> Spawners;

public:
	UPROPERTY()
	TArray<FVoxelSpawnerConfigRayGroup> RaySpawners_DEPRECATED;
	
	UPROPERTY()
	TArray<FVoxelSpawnerConfigHeightGroup> HeightSpawners_DEPRECATED;
	
public:
#if WITH_EDITOR
	bool NeedsToRebuild(UObject* Object, const FPropertyChangedEvent& PropertyChangedEvent);
#endif
	
protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void PostLoad() override;
	
	void SetReadOnlyPropertiesFromEditorOnly();
	void SetEditorOnlyPropertiesFromReadOnly();
	void FixGuids();
	void FixSpawnerDensityTypes();
};