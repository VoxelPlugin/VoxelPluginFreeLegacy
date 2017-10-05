// Copyright 2017 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "LandscapeGrassType.h"
#include "VoxelGrassType.generated.h"

USTRUCT()
struct FVoxelGrassVariety
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Grass)
		UStaticMesh* GrassMesh;

	/* Instances per 10 square meters. */
	UPROPERTY(EditAnywhere, Category = Grass)
		float GrassDensity;

	/* Inclusive max depth of chunks that render this */
	UPROPERTY(EditAnywhere, Category = Grass)
		int32 CullDepth;

	/* The distance where instances will begin to fade out if using a PerInstanceFadeAmount material node. 0 disables. */
	UPROPERTY(EditAnywhere, Category = Grass)
		int32 StartCullDistance;

	/**
	* The distance where instances will have completely faded out when using a PerInstanceFadeAmount material node. 0 disables.
	* When the entire cluster is beyond this distance, the cluster is completely culled and not rendered at all.
	*/
	UPROPERTY(EditAnywhere, Category = Grass)
		int32 EndCullDistance;

	/**
	* Specifies the smallest LOD that will be used for this component.
	* If -1 (default), the MinLOD of the static mesh asset will be used instead.
	*/
	UPROPERTY(EditAnywhere, Category = Grass)
		int32 MinLOD;

	/** Specifies grass instance scaling type */
	UPROPERTY(EditAnywhere, Category = Grass)
		EGrassScaling Scaling;

	/** Specifies the range of scale, from minimum to maximum, to apply to a grass instance's X Scale property */
	UPROPERTY(EditAnywhere, Category = Grass)
		FFloatInterval ScaleX;

	/** Specifies the range of scale, from minimum to maximum, to apply to a grass instance's Y Scale property */
	UPROPERTY(EditAnywhere, Category = Grass)
		FFloatInterval ScaleY;

	/** Specifies the range of scale, from minimum to maximum, to apply to a grass instance's Z Scale property */
	UPROPERTY(EditAnywhere, Category = Grass)
		FFloatInterval ScaleZ;

	/** Whether the grass instances should be placed at random rotation (true) or all at the same rotation (false) */
	UPROPERTY(EditAnywhere, Category = Grass)
		bool RandomRotation;

	/** Whether the grass instances should be tilted to the normal of the landscape (true), or always vertical (false) */
	UPROPERTY(EditAnywhere, Category = Grass)
		bool AlignToSurface;

	/**
	* Lighting channels that the grass will be assigned. Lights with matching channels will affect the grass.
	* These channels only apply to opaque materials, direct lighting, and dynamic lighting and shadowing.
	*/
	UPROPERTY(EditAnywhere, AdvancedDisplay, Category = Grass)
		FLightingChannels LightingChannels;

	/** Whether the grass instances should receive decals. */
	UPROPERTY(EditAnywhere, Category = Grass)
		bool bReceivesDecals;

	FVoxelGrassVariety()
		: GrassMesh(nullptr)
		, GrassDensity(400)
		, CullDepth(0)
		, StartCullDistance(10000.0f)
		, EndCullDistance(10000.0f)
		, MinLOD(-1)
		, Scaling(EGrassScaling::Uniform)
		, ScaleX(1.0f, 1.0f)
		, ScaleY(1.0f, 1.0f)
		, ScaleZ(1.0f, 1.0f)
		, RandomRotation(true)
		, AlignToSurface(true)
		, bReceivesDecals(true)
	{
	}
};

UCLASS(MinimalAPI)
class UVoxelGrassType : public UObject
{
	GENERATED_BODY()

public:
	UVoxelGrassType(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{

	}

	UPROPERTY(EditAnywhere, Category = Grass)
		TArray<FVoxelGrassVariety> GrassVarieties;
};
