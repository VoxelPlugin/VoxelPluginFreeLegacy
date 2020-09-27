// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelVector.h"
#include "GameFramework/Actor.h"
#include "VoxelWorldInterface.generated.h"

UENUM(BlueprintType)
enum class EVoxelWorldCoordinatesRounding : uint8
{
	RoundToNearest,
	RoundUp,
	RoundDown
};

#if CPP
class IVoxelWorldInterface
{
public:
	virtual ~IVoxelWorldInterface() = default;

	virtual FIntVector GlobalToLocal(const FVector& Position, EVoxelWorldCoordinatesRounding Rounding = EVoxelWorldCoordinatesRounding::RoundToNearest) const { unimplemented(); return {}; }
	virtual FVoxelVector GlobalToLocalFloat(const FVector& Position) const { unimplemented(); return {}; }

	virtual FVector LocalToGlobal(const FIntVector& Position) const { unimplemented(); return {}; }
	virtual FVector LocalToGlobalFloat(const FVoxelVector& Position) const { unimplemented(); return {}; }
};
#endif

// AActor so we can keep a weak ptr to it
UCLASS(Abstract)
class VOXEL_API AVoxelWorldInterface
	: public AActor
#if CPP
	, public IVoxelWorldInterface
#endif
{
	GENERATED_BODY()
};