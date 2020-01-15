// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGlobals.h"

namespace FVoxelUtilities
{
#if CPUPROFILERTRACE_ENABLED
	template<typename TUnique, typename T, typename... TArgs>
	uint16 GetStatsIdFromStringFormat(TUnique Unique, const T& Format, TArgs... Args)
	{
		thread_local TMap<FString, uint16>* IdMap = nullptr;
		if (!IdMap)
		{
			IdMap = new TMap<FString, uint16>();
		}
		const FString Name = FString::Printf(Format, Args...);
		uint16* Id = IdMap->Find(Name);
		if (!Id && ensureMsgf(IdMap->Num() < 1000, TEXT("More than 1000 different stats for %s!"), Format))
		{
			Id = &IdMap->Add(Name, FCpuProfilerTrace::OutputEventType(*Name, CpuProfilerGroup_Default));
		}
		return *Id;
	};

#define VOXEL_SCOPE_COUNTER_FORMAT(Format, ...) \
	FCpuProfilerTrace::FEventScope PREPROCESSOR_JOIN(CpuProfilerEventScope, __LINE__)(FVoxelUtilities::GetStatsIdFromStringFormat([](){}, TEXT(Format), ##__VA_ARGS__));
#else
#define VOXEL_SCOPE_COUNTER_FORMAT(Format, ...) VOXEL_SCOPE_COUNTER(Format)
#endif
}