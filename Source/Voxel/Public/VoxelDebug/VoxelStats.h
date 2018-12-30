// Copyright 2018 Phyronnaz

#pragma once

#include <chrono>

#include "CoreMinimal.h"
#include "VoxelLogStatDefinitions.h"
#include "Misc/ScopeLock.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/Build.h"
#include "UObject/Package.h"
#include "VoxelStats.generated.h"

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

	void StartStat(const FString& Name)
	{
		auto Now = std::chrono::high_resolution_clock::now();
		if (!PendingStat.IsEmpty())
		{
			float Duration = std::chrono::duration_cast<std::chrono::nanoseconds>(Now - PendingTime).count() / 1000000.f;
			SetFloatValue(PendingStat, Duration);
		}
		PendingStat = Name;
		PendingTime = Now;
	}
	
	void SetLOD(int LOD)
	{
		PendingTime = std::chrono::high_resolution_clock::now();
		SetValue("LOD", FString::FromInt(LOD));
	}
	void SetType(EVoxelStatsType Type)
	{
		const static UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EVoxelStatsType"));
		SetValue("Type", Enum->GetNameStringByValue((int64)Type));
	}

public:
	void SetValue(const FString& Name, const FString& Value)
	{
		check(!Values.Contains(Name));
		Values.Add(Name, Value);
	}
	void SetFloatValue(const FString& Name, float Value)
	{
		check(!Values.Contains(Name));
		FloatValues.Add(Name, Value);
	}

public:
	FString GetValue(const FString& Name) const
	{
		auto* Result = Values.Find(Name);
		return Result ? *Result : "";
	}
	float GetFloatValue(const FString& Name) const
	{
		auto* Result = FloatValues.Find(Name);
		return Result ? *Result : 0;
	}
	float GetTotalTime() const
	{
		float Result = 0;
		for (auto& It : FloatValues)
		{
			Result += It.Value;
		}
		return Result;
	}

private:
	FString PendingStat;
	std::chrono::time_point<std::chrono::high_resolution_clock> PendingTime;
	TMap<FString, FString> Values;
	TMap<FString, float> FloatValues;

	friend class FVoxelStats;
};
#else
struct FVoxelStatsElement
{
	FVoxelStatsElement() = default;
	
	void StartStat(const FString& Name) {}	
	void SetLOD(int LOD) {}
	void SetType(EVoxelStatsType Type) {}

	void SetValue(const FString& Name, const FString& Value) {}
	void SetFloatValue(const FString& Name, float Value) {}

	FString GetValue(const FString& Name) const { return FString(); }
	float GetFloatValue(const FString& Name) const { return 0; }
	float GetTotalTime() const { return 0; }
};
#endif

#if STATS
class FVoxelStats
{
public:
	static void AddElement(FVoxelStatsElement Element)
	{
		FScopeLock Lock(&CriticalSection);
		if (bRecord)
		{
			// Make sure it's finished
			Element.StartStat("");
			Elements.Add(Element);
		}
	}

	static void StopRecordingAndSaveStatsFile()
	{
		FScopeLock Lock(&CriticalSection);
		bRecord = false;
		FString Text;
		{
			TSet<FString> NamesSet;
			TSet<FString> FloatNamesSet;
			for (auto& Element : Elements)
			{
				for (auto& It : Element.Values)
				{
					NamesSet.Add(It.Key);
				}
				for (auto& It : Element.FloatValues)
				{
					FloatNamesSet.Add(It.Key);
				}
			}
			auto Names = NamesSet.Array();
			auto FloatNames = FloatNamesSet.Array();

			for (auto& Name : Names)
			{
				Text.Append(Name + ";");
			}
			for (auto& Name : FloatNames)
			{
				Text.Append(Name + ";");
			}
			Text.Append("Total\n");

			for (auto& Element : Elements)
			{
				for (auto& Name : Names)
				{
					Text.Append(Element.GetValue(Name) + ";");
				}
				for (auto& Name : FloatNames)
				{
					Text.Append(FString::SanitizeFloat(Element.GetFloatValue(Name)) + ";");
				}
				Text.Append(FString::SanitizeFloat(Element.GetTotalTime()) + "\n");
			}
		}

		FString Path = FPaths::ProfilingDir() + "VoxelStats/";

		FString Sep = TEXT("_");
		int32 Year = 0, Month = 0, DayOfWeek = 0, Day = 0, Hour = 0, Min = 0, Sec = 0, MSec = 0;
		FPlatformTime::SystemTime(Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec);
		FString Name = FString::FromInt(Year) +
			Sep + FString::FromInt(Month) +
			Sep + FString::FromInt(DayOfWeek) +
			Sep + FString::FromInt(Day) +
			Sep + FString::FromInt(Hour) +
			Sep + FString::FromInt(Min) +
			Sep + FString::FromInt(Sec) +
			Sep + FString::FromInt(MSec);

		FString FullPath = Path + Name + ".csv";

		if (FFileHelper::SaveStringToFile(Text, *FullPath))
		{
			UE_LOG(LogVoxel, Log, TEXT("VoxelStats: Saved! %s"), *FullPath);
		}
		else
		{
			UE_LOG(LogVoxel, Error, TEXT("VoxelStats: Error when saving"));
		}
	}

	static void StartRecording()
	{
		FScopeLock Lock(&CriticalSection);
		bRecord = true;
		Elements.Reset();

		UE_LOG(LogVoxel, Log, TEXT("VoxelStats: Recording started"));
	}

private:
	static TArray<FVoxelStatsElement> Elements;
	static FCriticalSection CriticalSection;
	static bool bRecord;
};
#else
class FVoxelStats
{
public:
	static void AddElement(const FVoxelStatsElement& Element) {}
	static void StopRecordingAndSaveStatsFile() {}
	static void StartRecording() {}
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