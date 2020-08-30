// Copyright 2020 Phyronnaz

#include "VoxelTools/VoxelTestLibrary.h"
#include "VoxelTools/VoxelToolHelpers.h"
#include "VoxelData/VoxelDataIncludes.h"

FVoxelTestValues UVoxelTestLibrary::ReadValues(AVoxelWorld* World, FVoxelIntBox Bounds)
{
	VOXEL_FUNCTION_COUNTER();
	CHECK_VOXELWORLD_IS_CREATED();

	auto& Data = World->GetData();
	
	FVoxelReadScopeLock Lock(Data, Bounds, FUNCTION_FNAME);
	return { MakeSharedCopy(Data.GetValues(Bounds)) };
}

void UVoxelTestLibrary::TestValues(FVoxelTestValues ValuesA, FVoxelTestValues ValuesB)
{
	VOXEL_FUNCTION_COUNTER();

	if (ValuesA.Values->Num() != ValuesB.Values->Num())
	{
		ensure(false);
		return;
	}

	for (int32 Index = 0; Index < ValuesA.Values->Num(); Index++)
	{
		const auto ValueA = (*ValuesA.Values)[Index];
		const auto ValueB = (*ValuesB.Values)[Index];

		if (ValueA != ValueB)
		{
			ensure(false);
			return;
		}
	}
}