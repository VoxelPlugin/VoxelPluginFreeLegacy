// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelIntBox.h"
#include "VoxelSharedPtr.h"
#include "VoxelSurfaceEdits.generated.h"

USTRUCT(BlueprintType)
struct FVoxelSurfaceEditsVoxelBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FIntVector Position = FIntVector(ForceInit);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVector Normal = FVector(ForceInit);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float Value = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	FVector SurfacePosition = FVector(ForceInit);
};

USTRUCT(BlueprintType)
struct FVoxelSurfaceEditsVoxel : public FVoxelSurfaceEditsVoxelBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voxel")
	float Strength = 1;

public:
	FVoxelSurfaceEditsVoxel() = default;
	FVoxelSurfaceEditsVoxel(const FVoxelSurfaceEditsVoxelBase& Other)
		: FVoxelSurfaceEditsVoxelBase(Other)
	{
	}
};

///////////////////////////////////////////////////////////////////////////////

struct FVoxelSurfaceEditsVoxelsInfo
{
	bool bHasValues = false;
	bool bHasExactDistanceField = false;
	bool bHasNormals = false;
	bool bHasSurfacePositions = false;
	bool bIs2D = false;
};

USTRUCT(BlueprintType)
struct FVoxelSurfaceEditsVoxels
{
	GENERATED_BODY()

	FVoxelSurfaceEditsVoxelsInfo Info;
	TVoxelSharedRef<const TArray<FVoxelSurfaceEditsVoxelBase>> Voxels = MakeVoxelShared<TArray<FVoxelSurfaceEditsVoxelBase>>();
};

USTRUCT(BlueprintType)
struct FVoxelSurfaceEditsProcessedVoxels
{
	GENERATED_BODY()
		
	FVoxelIntBox Bounds;
	FVoxelSurfaceEditsVoxelsInfo Info;
	TVoxelSharedRef<const TArray<FVoxelSurfaceEditsVoxel>> Voxels = MakeVoxelShared<TArray<FVoxelSurfaceEditsVoxel>>();
};

///////////////////////////////////////////////////////////////////////////////

namespace EVoxelSurfaceEditsStackElementFlags
{
	enum Type : uint32
	{
		None         = 0,
		NeedValues   = 1 << 0,
		NeedNormals  = 1 << 1,
		ShouldBeLast = 1 << 2,
	};
}

USTRUCT(BlueprintType)
struct FVoxelSurfaceEditsStackElement
{
	GENERATED_BODY()

	using FApply = TFunction<void(const FVoxelSurfaceEditsVoxelsInfo& /*Info*/, TArray<FVoxelSurfaceEditsVoxel>& /*Voxels*/)>;

	FString Name;
	uint32 Flags = EVoxelSurfaceEditsStackElementFlags::None;
	FApply Apply;

	FVoxelSurfaceEditsStackElement() = default;

	FVoxelSurfaceEditsStackElement(const FString& Name, uint32 Flags, const FApply& Apply)
		: Name(Name)
		, Flags(Flags)
		, Apply(Apply)
	{
	}
};

USTRUCT(BlueprintType)
struct VOXEL_API FVoxelSurfaceEditsStack
{
	GENERATED_BODY()
	
	TArray<FVoxelSurfaceEditsStackElement> Stack;

	void Add(const FVoxelSurfaceEditsStackElement& Element) { Stack.Add(Element); }

	bool HasErrors(const FVoxelSurfaceEditsVoxels& Voxels, FString& OutErrors) const;
	// Set bComputeBounds if you already have valid bounds & will use the Impl functions
	FVoxelSurfaceEditsProcessedVoxels Execute(const FVoxelSurfaceEditsVoxels& Voxels, bool bComputeBounds = true) const;
};