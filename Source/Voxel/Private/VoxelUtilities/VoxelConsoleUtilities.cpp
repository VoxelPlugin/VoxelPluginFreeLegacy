// Copyright 2021 Phyronnaz

#include "VoxelUtilities/VoxelConsoleUtilities.h"
#include "VoxelWorld.h"
#include "EngineUtils.h"

FConsoleCommandWithWorldAndArgsDelegate FVoxelUtilities::CreateVoxelWorldCommandWithArgs(TFunction<void(AVoxelWorld& World, const TArray<FString>& Args)> Lambda)
{
	return FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([=](const TArray<FString>& Args, UWorld* World)
	{
		for (TActorIterator<AVoxelWorld> It(World); It; ++It)
		{
			if (It->IsCreated())
			{
				Lambda(**It, Args);
			}
		}
	});
}

FConsoleCommandWithWorldDelegate FVoxelUtilities::CreateVoxelWorldCommand(TFunction<void(AVoxelWorld& World)> Lambda)
{
	return FConsoleCommandWithWorldDelegate::CreateLambda([=](UWorld* World)
	{
		for (TActorIterator<AVoxelWorld> It(World); It; ++It)
		{
			if (It->IsCreated())
			{
				Lambda(**It);
			}
		}
	});
}