// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelConfigEnums.h"
#include "VoxelSpawnerConfig.generated.h"

class UVoxelSpawner;
class FVoxelWorldGeneratorInstance;
class UVoxelSpawnerConfig;
class UVoxelSpawnerOutputsConfig;

UENUM()
enum class EVoxelSpawnerConfigElementRandomGenerator : uint8
{
	// Evenly distributed points
	Sobol,
	// More uneven points than Sobol. Unreal uses Halton to spawn grass in the default Landscape system
	Halton
};

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
enum class EVoxelSpawnerChannel : uint8
{
	R,
	G,
	B,
	A,
	None
};

inline EVoxelRGBA GetRGBALayerFromSpawnerChannel(EVoxelSpawnerChannel Channel)
{
	ensure(Channel != EVoxelSpawnerChannel::None);
	return EVoxelRGBA(Channel);
}

USTRUCT()
struct FVoxelSpawnerConfigElementAdvanced_Base
{
	GENERATED_BODY()

	// If you want to control your foliage seeds from your voxel world. Can be left to the default values
	// Name referencing to the voxel world seed map
	UPROPERTY(EditAnywhere, Category = "Voxel")
	FName SeedName = "FoliageSeed";

	// If you want to control your foliage seeds from your voxel world. Can be left to the default values
	// Seed if SeedName is not found in the voxel world seed map
	UPROPERTY(EditAnywhere, Category = "Voxel")
	uint32 DefaultSeed = 1337;

	// Controls the spawning pattern
	UPROPERTY(EditAnywhere, Category = "Voxel")
	EVoxelSpawnerConfigElementRandomGenerator RandomGenerator = EVoxelSpawnerConfigElementRandomGenerator::Halton;
};

USTRUCT()
struct FVoxelSpawnerConfigElementAdvanced_Height : public FVoxelSpawnerConfigElementAdvanced_Base
{
	GENERATED_BODY()
		
	// Controls whether to compute the density or the height first. Try both and see which is faster
	UPROPERTY(EditAnywhere, Category = "Voxel")
	bool bComputeDensityFirst = false;
};

USTRUCT()
struct FVoxelSpawnerConfigElementAdvanced_Ray : public FVoxelSpawnerConfigElementAdvanced_Base
{
	GENERATED_BODY()
	
	// If the specific foliage channel is painted, its value will be used as density instead of the generator one
	UPROPERTY(EditAnywhere, Category = "Voxel")
	EVoxelSpawnerChannel Channel = EVoxelSpawnerChannel::None;
};

USTRUCT()
struct FVoxelSpawnerConfigElement_Base
{
	GENERATED_BODY()

	// Your world generator needs to have a float output named like this. If empty, will use a constant density of 0
	UPROPERTY(EditAnywhere, Category = "Voxel")
	FVoxelSpawnerOutputName DensityGraphOutputName;
	
	UPROPERTY(EditAnywhere, Category = "Voxel")
	UVoxelSpawner* Spawner = nullptr;

public:
	float DistanceBetweenInstancesInVoxel = 0;
	uint32 FinalSeed = 0;
};

USTRUCT()
struct FVoxelSpawnerConfigElement_Height : public FVoxelSpawnerConfigElement_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Voxel")
	FVoxelSpawnerConfigElementAdvanced_Height Advanced;
};

USTRUCT()
struct FVoxelSpawnerConfigElement_Ray : public FVoxelSpawnerConfigElement_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Voxel")
	FVoxelSpawnerConfigElementAdvanced_Ray Advanced;
};

USTRUCT()
struct FVoxelSpawnerConfigHeightGroup
{
	GENERATED_BODY()

	// The name of the custom graph output used to determine the height
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (HideConstantOutputs))
	FVoxelSpawnerOutputName HeightGraphOutputName = "Height";

	// Size of a foliage chunk, used to determine spawning distance.
	// Keep ChunkSize as high as possible and GenerationDistanceInChunks as low as possible for better perf
	UPROPERTY(EditAnywhere, Category = "Voxel")
	uint32 ChunkSize = 32;

	// Relative to ChunkSize. You can increase it if you want the spawning to be in more of a circle shape. Higher value might add a game thread cost
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (ClampMax = 16, UIMin = 1, UIMax = 16))
	uint32 GenerationDistanceInChunks = 2;

	UPROPERTY(VisibleAnywhere, Category = "Voxel")
	uint32 GenerationDistanceInVoxels = 0;

	UPROPERTY(EditAnywhere, Category = "Voxel")
	TArray<FVoxelSpawnerConfigElement_Height> Spawners;
};

USTRUCT()
struct FVoxelSpawnerConfigRayGroup
{
	GENERATED_BODY()

	// The LOD of the mesh to trace rays against
	// High LOD = faster but less precise
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (ClampMin = 0, ClampMax = 24, UIMin = 0, UIMax = 24))
	uint32 LOD = 0;

	// Chunk size, set by the LOD
	UPROPERTY(VisibleAnywhere, Category = "Voxel")
	uint32 ChunkSize = 32;
	
	UPROPERTY(EditAnywhere, Category = "Voxel", meta = (UIMin = 1, UIMax = 16))
	uint32 GenerationDistanceInChunks = 2;

	UPROPERTY(VisibleAnywhere, Category = "Voxel")
	uint32 GenerationDistanceInVoxels = 0;
	
	UPROPERTY(EditAnywhere, Category = "Voxel")
	TArray<FVoxelSpawnerConfigElement_Ray> Spawners;
};

UENUM()
enum class EVoxelSpawnerConfigRayWorldType : uint8
{
	Flat,
	Sphere
};

UCLASS()
class VOXEL_API UVoxelSpawnerConfig : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	EVoxelSpawnerConfigRayWorldType WorldType;
	
	UPROPERTY(EditAnywhere, Category = "Config")
	UVoxelSpawnerOutputsConfig* WorldGeneratorOutputs;

	// Will linetrace the voxel geometry to find spawning locations. Works with any kind of world/shapes
	UPROPERTY(EditAnywhere, Category = "Ray Spawners")
	TArray<FVoxelSpawnerConfigRayGroup> RaySpawners;
	
	// These spawners uses a height output from the world generator to spawn, allowing for large spawn distance.
	UPROPERTY(EditAnywhere, Category = "Height Spawners")
	TArray<FVoxelSpawnerConfigHeightGroup> HeightSpawners;

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostLoad() override;
	
	void UpdateReadOnlyProperties();
#endif
};