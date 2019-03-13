// Copyright 2019 Phyronnaz

#include "VoxelDebug/VoxelStats.h"
#include "VoxelGlobals.h"

#include "HAL/IConsoleManager.h"
#include "Misc/ScopeLock.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/Build.h"
#include "UObject/Package.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "Voxel"
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

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void FVoxelStatsElement::StartStat(const FName& Name, bool bIsActiveWork)
{
	double Now = FPlatformTime::Seconds();
	if (!PendingStat.IsNone())
	{
		double Duration = (Now - PendingTime) * 1000.f;
		SetFloatValue(PendingStat, Duration, bPendingIsActiveWork);
	}
	PendingStat = Name;
	PendingTime = Now;
	bPendingIsActiveWork = bIsActiveWork;
}

void FVoxelStatsElement::SetLOD(int LOD)
{
	PendingTime = FPlatformTime::Seconds();
	SetValue("LOD", FString::FromInt(LOD));
}

void FVoxelStatsElement::SetType(EVoxelStatsType Type)
{
	const static UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EVoxelStatsType"));
	SetValue("Type", Enum->GetNameStringByValue((int64)Type));
}

void FVoxelStatsElement::SetIsEmpty(bool bIsEmpty)
{
	SetValue("bIsEmpty", bIsEmpty ? "true" : "false");
}

void FVoxelStatsElement::SetNumIndices(int NumIndices)
{
	SetValue("NumIndices", LexToString(NumIndices));
}

void FVoxelStatsElement::SetNumVertices(int NumVertices)
{
	SetValue("NumVertices", LexToString(NumVertices));
}

void FVoxelStatsElement::SetMaterialConfig(EVoxelMaterialConfig MaterialConfig)
{
	const static UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EVoxelMaterialConfig"));
	SetValue("MaterialConfig", Enum->GetNameStringByValue((int64)MaterialConfig));
}

void FVoxelStatsElement::SetIsCanceled(bool bIsCanceled)
{
	SetValue("bIsCanceled", bIsCanceled ? "true" : "false");
}

FString FVoxelStatsElement::GetValue(const FName& Name) const
{
	auto* Result = Values.FindByPredicate([&](auto& Item) { return Item.Name == Name; });
	return Result ? Result->Value : "";
}

double FVoxelStatsElement::GetFloatValue(const FName& Name) const
{
	auto* Result = FloatValues.FindByPredicate([&](auto& Item) { return Item.Name == Name; });
	return Result ? Result->Value : -1;
}

inline FString ToString(const TArray<double>& Array)
{
	FString Result = "\"";
	if (Array.Num() > 0)
	{
		Result += LexToString(Array[0]);
		for (int Index = 0; Index < Array.Num() ; Index++)
		{
			Result += ", " + LexToString(Array[Index]);
		}
	}
	Result += "\"";
	return Result;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

TArray<FVoxelStatsElement> FVoxelStats::Elements;
FCriticalSection FVoxelStats::CriticalSection;
bool FVoxelStats::bRecord = false;

void FVoxelStats::AddElement(FVoxelStatsElement Element)
{
	check(!Element.GetValue("Type").IsEmpty());
	FScopeLock Lock(&CriticalSection);
	if (bRecord)
	{
		// Make sure it's finished
		Element.StartStat("");
		Elements.Add(Element);
	}
}

void FVoxelStats::StopRecordingAndSaveStatsFile()
{
	FScopeLock Lock(&CriticalSection);
	if (!bRecord)
	{
		FNotificationInfo Info = FNotificationInfo(FText());
		Info.ExpireDuration = 10.f;
		UE_LOG(LogVoxel, Error, TEXT("VoxelStats: Can't stop recording as it hasn't started"));
		Info.CheckBoxState = ECheckBoxState::Unchecked;
		Info.Text = LOCTEXT("VoxelStatsErrorNotStarted", "Error: Recording not started!");
		FSlateNotificationManager::Get().AddNotification(Info);
		return;
	}

	FString Text;
	{
		TSet<FName> NamesSet;
		TSet<FName> FloatNamesSet;
		TMap<FName, bool> IsActiveWork;
		for (auto& Element : Elements)
		{
			for (auto& It : Element.Values)
			{
				NamesSet.Add(It.Name);
			}
			for (auto& It : Element.FloatValues)
			{
				FloatNamesSet.Add(It.Name);
				check(!IsActiveWork.Contains(It.Name) || IsActiveWork[It.Name] == It.bIsActiveWork);
				IsActiveWork.FindOrAdd(It.Name) = It.bIsActiveWork;
			}
		}
		auto Names = NamesSet.Array();
		auto FloatNames = FloatNamesSet.Array();

		for (auto& Name : Names)
		{
			Text.Append(Name.ToString() + ";");
		}
		for (auto& Name : FloatNames)
		{
			Text.Append((IsActiveWork[Name] ? "work time:" : "wait time:") + Name.ToString() + ";");
		}
		Text.Append("Value queries;Material queries\n");

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
			Text.Append(ToString(Element.ValueAccessTimes) + ";" + ToString(Element.MaterialAccessTimes) + "\n");
		}
	}

	FString Path = FPaths::ProfilingDir() + "VoxelStats/";

	FString Sep = TEXT("_");
	int32 Year = 0, Month = 0, DayOfWeek = 0, Day = 0, Hour = 0, Min = 0, Sec = 0, MSec = 0;
	FPlatformTime::SystemTime(Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec);
	FString Name = FString::FromInt(Year) +
		Sep + FString::FromInt(Month) +
		Sep + FString::FromInt(Day) +
		Sep + FString::FromInt(Hour) +
		Sep + FString::FromInt(Min) +
		Sep + FString::FromInt(Sec) +
		Sep + FString::FromInt(MSec);

	FString FullPath = Path + Name + ".csv";

	FNotificationInfo Info = FNotificationInfo(FText());
	Info.ExpireDuration = 10.f;
	if (FFileHelper::SaveStringToFile(Text, *FullPath))
	{
		UE_LOG(LogVoxel, Log, TEXT("VoxelStats: Saved! %s"), *FullPath);
		Info.CheckBoxState = ECheckBoxState::Checked;
		Info.Text = FText::Format(LOCTEXT("VoxelStatsSuccess", "Saved! {0}"), FText::FromString(FullPath));
	}
	else
	{
		UE_LOG(LogVoxel, Error, TEXT("VoxelStats: Error when saving"));
		Info.CheckBoxState = ECheckBoxState::Unchecked;
		Info.Text = LOCTEXT("VoxelStatsError", "Error when saving");
	}
	FSlateNotificationManager::Get().AddNotification(Info);
	bRecord = false;
	Elements.Reset();
}

void FVoxelStats::StartRecording()
{
	FScopeLock Lock(&CriticalSection);
	bRecord = true;
	Elements.Reset();

	FNotificationInfo Info = FNotificationInfo(FText());
	Info.Text = LOCTEXT("VoxelStatsStarted", "Recording started");
	FSlateNotificationManager::Get().AddNotification(Info);

	UE_LOG(LogVoxel, Log, TEXT("VoxelStats: Recording started"));
}

#endif
#undef LOCTEXT_NAMESPACE