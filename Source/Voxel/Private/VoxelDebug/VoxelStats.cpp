// Copyright 2018 Phyronnaz

#include "VoxelDebug/VoxelStats.h"
#include "HAL/IConsoleManager.h"

#if STATS

void StartRecordingStats(const TArray<FString>& Args)
{
	FVoxelStats::StartRecording();
}

void StopRecordingStats(const TArray<FString>& Args)
{
	FVoxelStats::StopRecordingAndSaveStatsFile();
}

FAutoConsoleCommand StartRecordingStatsCmd(
	TEXT("voxel.StartRecordingStats"),
	TEXT("Start Recording Stats"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&StartRecordingStats)
	);

FAutoConsoleCommand StopRecordingStatsCmd(
	TEXT("voxel.StopRecordingStats"),
	TEXT("Stop Recording Stats and save the stats file. Check Output Log for save location"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&StopRecordingStats)
	);

TArray<FVoxelStatsElement> FVoxelStats::Elements;
FCriticalSection FVoxelStats::CriticalSection;
bool FVoxelStats::bRecord = false;
#endif
