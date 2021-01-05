// Copyright 2021 Phyronnaz

#include "VoxelTools/VoxelCubicLibrary.h"
#include "VoxelTools/VoxelToolHelpers.h"

FIntVector UVoxelCubicLibrary::GetCubicVoxelPositionFromHit(AVoxelWorld* World, FVector HitPosition, FVector HitNormal, bool bSelectVoxelOutside)
{
	CHECK_VOXELWORLD_IS_CREATED();

	const FVector InsideVoxel = HitPosition + HitNormal * World->VoxelSize / 2 * (bSelectVoxelOutside ? 1 : -1);
	return World->GlobalToLocal(InsideVoxel);
}

bool UVoxelCubicLibrary::GetCubicVoxelValue(AVoxelWorld* World, FIntVector Position)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED();

	auto& Data = World->GetSubsystemChecked<FVoxelData>();
	FVoxelReadScopeLock Lock(Data, FVoxelIntBox(Position), FUNCTION_FNAME);
	return !Data.GetValue(Position, 0).IsEmpty();
}

void UVoxelCubicLibrary::SetCubicVoxelValue(AVoxelWorld* World, FIntVector Position, bool bValue)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED_VOID();
	
	auto& Data = World->GetSubsystemChecked<FVoxelData>();
	FVoxelWriteScopeLock Lock(Data, FVoxelIntBox(Position), FUNCTION_FNAME);
	Data.SetValue(Position, bValue ? FVoxelValue::Full() : FVoxelValue::Empty());
}