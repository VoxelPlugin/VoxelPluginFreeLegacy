// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMaterial.generated.h"


/**
 * A VoxelMaterial is defined by 2 material indices, and an alpha to lerp between them
 */
USTRUCT(BlueprintType)
struct FVoxelMaterial
{
	GENERATED_BODY()
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Index1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Index2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Alpha;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 VoxelActor;

	FVoxelMaterial() 
		: Index1(0)
		, Index2(0)
		, Alpha(0)
		, VoxelActor(0)
	{

	}

	FVoxelMaterial(uint8 Index1, uint8 Index2, uint8 Alpha, uint8 VoxelActor)
		: Index1(Index1)
		, Index2(Index2)
		, Alpha(Alpha)
		, VoxelActor(VoxelActor)
	{

	}

	FVoxelMaterial(FColor Color) 
		: Index1(Color.R)
		, Index2(Color.G)
		, Alpha(Color.B)
		, VoxelActor(Color.A)
	{

	}

	FORCEINLINE FColor ToFColor() const
	{
		return FColor(Index1, Index2, Alpha, VoxelActor);
	}

	FORCEINLINE bool HasSameIndexesAs(const FVoxelMaterial& Other) const
	{
		return (Index1 == Other.Index1) && (Index2 == Other.Index2);
	}

	FORCEINLINE bool operator==(const FVoxelMaterial& Other) const
	{
		return (Index1 == Other.Index1) && (Index2 == Other.Index2) && (Alpha == Other.Alpha) && (VoxelActor == Other.VoxelActor);
	}
};

FORCEINLINE FArchive& operator<<(FArchive &Ar, FVoxelMaterial& Material)
{
	Ar << Material.Index1;
	Ar << Material.Index2;
	Ar << Material.Alpha;
	Ar << Material.VoxelActor;

	return Ar;
}