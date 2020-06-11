// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMinimal.h"

namespace FVoxelUtilities
{
#if CPUPROFILERTRACE_ENABLED
	using FCpuProfilerIdTraceType = UE_25_SWITCH(uint16, uint32);
	
	template<typename TUnique, typename T, typename... TArgs>
	FCpuProfilerIdTraceType GetStatsIdFromStringFormat(TUnique Unique, const T& Format, TArgs... Args)
	{
		thread_local TMap<FString, FCpuProfilerIdTraceType>* IdMap = nullptr;
		if (!IdMap)
		{
			IdMap = new TMap<FString, FCpuProfilerIdTraceType>();
		}
		const FString Name = FString::Printf(Format, Args...);
		FCpuProfilerIdTraceType* Id = IdMap->Find(Name);
		if (!Id && ensureMsgf(IdMap->Num() < 1000, TEXT("More than 1000 different stats for %s!"), Format))
		{
			Id = &IdMap->Add(Name, FCpuProfilerTrace::OutputEventType(*Name ONLY_UE_24_AND_LOWER(, CpuProfilerGroup_Default)));
		}
		return *Id;
	};

#define VOXEL_SCOPE_COUNTER_FORMAT(Format, ...) \
	FCpuProfilerTrace::FEventScope PREPROCESSOR_JOIN(CpuProfilerEventScope, __LINE__)(FVoxelUtilities::GetStatsIdFromStringFormat([](){}, TEXT(Format), ##__VA_ARGS__) ONLY_UE_25_AND_HIGHER(, CpuChannel));
#else
#define VOXEL_SCOPE_COUNTER_FORMAT(Format, ...) VOXEL_ASYNC_SCOPE_COUNTER(Format)
#endif
}