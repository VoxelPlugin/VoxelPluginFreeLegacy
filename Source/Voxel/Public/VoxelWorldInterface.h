// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VoxelWorldInterface.generated.h"

UENUM(BlueprintType)
enum class EVoxelWorldCoordinatesRounding : uint8
{
	RoundToNearest,
	RoundUp,
	RoundDown
};

UCLASS(Abstract)
class VOXEL_API AVoxelWorldInterface : public AActor
{
	GENERATED_BODY()

public:
	virtual FIntVector GlobalToLocal(const FVector& Position, EVoxelWorldCoordinatesRounding Rounding = EVoxelWorldCoordinatesRounding::RoundToNearest) const { unimplemented(); return {}; }
	virtual FVector GlobalToLocalFloat(const FVector& Position) const { unimplemented(); return {}; }

	virtual FVector LocalToGlobal(const FIntVector& Position) const { unimplemented(); return {}; }
	virtual FVector LocalToGlobalFloat(const FVector& Position) const { unimplemented(); return {}; }
};