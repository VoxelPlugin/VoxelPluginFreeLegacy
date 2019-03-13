// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelConfigEnums.h"
#include "VoxelStats.generated.h"

#define RECORD_QUERIES_STATS 0

UENUM()
enum class EVoxelStatsType
{
	NormalMarchingCubes,
	TransitionsMarchingCubes,
	NormalCubic,
	TransitionsCubic
};

#if STATS
struct FVoxelStatsElement
{
	FVoxelStatsElement() = default;

	void StartStat(const FName& Name, bool bIsActiveWork = true);

	void SetLOD(int LOD);
	void SetType(EVoxelStatsType Type);
	void SetIsEmpty(bool bIsEmpty);
	void SetNumIndices(int NumIndices);
	void SetNumVertices(int NumVertices);
	void SetMaterialConfig(EVoxelMaterialConfig MaterialConfig);
	void SetIsCanceled(bool bIsCanceled);

private:
	void SetValue(const FName& Name, const FString& Value)
	{
		Values.Add({ Name, Value });
	}
	void SetFloatValue(const FName& Name, double Value, bool bIsActiveWork)
	{
		FloatValues.Add({ Name, Value, bIsActiveWork });
	}
	void AddValueAccessTime(double Time)
	{
		ValueAccessTimes.Add(Time);
	}
	void AddMaterialAccessTime(double Time)
	{
		MaterialAccessTimes.Add(Time);
	}

	FString GetValue(const FName& Name) const;
	double GetFloatValue(const FName& Name) const;

private:
	FName PendingStat;
	double PendingTime;
	bool bPendingIsActiveWork;

	struct FValue
	{
		FName Name;
		FString Value;
	};
	TArray<FValue> Values;

	struct FFloatValue
	{
		FName Name;
		double Value;
		bool bIsActiveWork;
	};
	TArray<FFloatValue> FloatValues;

	TArray<double> ValueAccessTimes;
	TArray<double> MaterialAccessTimes;

	friend class FVoxelStats;
	friend struct FVoxelScopeValueAccessCounter;
	friend struct FVoxelScopeMaterialAccessCounter;
};

class FVoxelStats
{
public:
	static void AddElement(FVoxelStatsElement Element);

	static void StopRecordingAndSaveStatsFile();

	static void StartRecording();

private:
	static TArray<FVoxelStatsElement> Elements;
	static FCriticalSection CriticalSection;
	static bool bRecord;
};
#else
struct FVoxelStatsElement
{
	FVoxelStatsElement() = default;
	
	void StartStat(const FName& Name, bool bIsActiveWork = true) {}

	void SetLOD(int LOD) {}
	void SetType(EVoxelStatsType Type) {}
	void SetIsEmpty(bool bIsEmpty) {}
	void SetNumIndices(int NumIndices) {}
	void SetNumVertices(int NumVertices) {}
	void SetMaterialConfig(EVoxelMaterialConfig MaterialConfig) {}
	void SetIsCanceled(bool bIsCanceled) {}
};

class FVoxelStats
{
public:
	static void AddElement(const FVoxelStatsElement& Element) {}
	static void StopRecordingAndSaveStatsFile() {}
	static void StartRecording() {}
};
#endif

#if STATS && RECORD_QUERIES_STATS
struct FVoxelScopeValueAccessCounter
{
	FVoxelScopeValueAccessCounter(FVoxelStatsElement& Stats)
		: Stats(Stats)
		, StartTime(FPlatformTime::Seconds())
	{
	}
	~FVoxelScopeValueAccessCounter()
	{
		Stats.AddValueAccessTime((FPlatformTime::Seconds() - StartTime) * 1000);
	}

private:
	FVoxelStatsElement& Stats;
	const double StartTime;
};

struct FVoxelScopeMaterialAccessCounter
{
	FVoxelScopeMaterialAccessCounter(FVoxelStatsElement& Stats)
		: Stats(Stats)
		, StartTime(FPlatformTime::Seconds())
	{
	}
	~FVoxelScopeMaterialAccessCounter()
	{
		Stats.AddMaterialAccessTime((FPlatformTime::Seconds() - StartTime) * 1000);
	}

private:
	FVoxelStatsElement& Stats;
	const double StartTime;
};
#else
struct FVoxelScopeValueAccessCounter
{
	FVoxelScopeValueAccessCounter(FVoxelStatsElement& Stats) {}
};

struct FVoxelScopeMaterialAccessCounter
{
	FVoxelScopeMaterialAccessCounter(FVoxelStatsElement& Stats) {}
};
#endif

UCLASS()
class VOXEL_API UVoxelStatsHelper : public UObject
{ 
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void StopRecordingStatsAndSaveStatsFile()
	{
		FVoxelStats::StopRecordingAndSaveStatsFile();
	}
	
	UFUNCTION(BlueprintCallable, Category = "Voxel")
	static void StartRecordingStats()
	{
		FVoxelStats::StartRecording();
	}
};