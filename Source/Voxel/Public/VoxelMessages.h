// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Logging/TokenizedMessage.h"

enum class EVoxelShowNotification : uint8
{
	Show,
	Hide
};

struct VOXEL_API FVoxelMessages
{
	DECLARE_MULTICAST_DELEGATE_TwoParams(FLogMessageDelegate, const TSharedRef<FTokenizedMessage>&, EVoxelShowNotification);
	static FLogMessageDelegate LogMessageDelegate;

	struct FButton
	{
		FString Text;
		FString Tooltip;
		FSimpleDelegate OnClick;
		bool bCloseOnClick = true;
	};
	struct FNotification
	{
		uint64 UniqueId = 0;
		FString Message;
		FSimpleDelegate OnClose;		
		float Duration = 10.f;

		TArray<FButton> Buttons;
	};
	DECLARE_MULTICAST_DELEGATE_OneParam(FShowNotificationDelegate, const FNotification&);
	static FShowNotificationDelegate ShowNotificationDelegate;
	
public:
	static void LogMessage(const TSharedRef<FTokenizedMessage>& Message, EVoxelShowNotification ShouldShow);
	static void LogMessage(const FText& Message, EMessageSeverity::Type Severity, EVoxelShowNotification ShouldShow, const UObject* Object = nullptr);
	static void ShowNotification(const FNotification& Notification);
	
public:
	template<EVoxelShowNotification ShouldShow = EVoxelShowNotification::Show>
	static void Error(const FString& Message, const UObject* Object = nullptr)
	{
		Error<ShouldShow>(FText::FromString(Message), Object);
	}
	template<EVoxelShowNotification ShouldShow = EVoxelShowNotification::Show>
	static void Error(const FText& Message, const UObject* Object = nullptr)
	{
		LogMessage(Message, EMessageSeverity::Error, ShouldShow, Object);
	}
	
	template<EVoxelShowNotification ShouldShow = EVoxelShowNotification::Show>
	static void Warning(const FString& Message, const UObject* Object = nullptr)
	{
		Warning<ShouldShow>(FText::FromString(Message), Object);
	}
	template<EVoxelShowNotification ShouldShow = EVoxelShowNotification::Show>
	static void Warning(const FText& Message, const UObject* Object = nullptr)
	{
		LogMessage(Message, EMessageSeverity::Warning, ShouldShow, Object);
	}
	
	template<EVoxelShowNotification ShouldShow = EVoxelShowNotification::Show>
	static void Info(const FString& Message, const UObject* Object = nullptr)
	{
		Info<ShouldShow>(FText::FromString(Message), Object);
	}
	template<EVoxelShowNotification ShouldShow = EVoxelShowNotification::Show>
	static void Info(const FText& Message, const UObject* Object = nullptr)
	{
		LogMessage(Message, EMessageSeverity::Info, ShouldShow, Object);
	}

public:
	template<EVoxelShowNotification ShouldShow = EVoxelShowNotification::Show>
	static void CondError(bool bCond, const FString& Message, const UObject* Object = nullptr)
	{
		CondError<ShouldShow>(bCond, FText::FromString(Message), Object);
	}
	template<EVoxelShowNotification ShouldShow = EVoxelShowNotification::Show>
	static void CondError(bool bCond, const FText& Message, const UObject* Object = nullptr)
	{
		if (bCond)
		{
			LogMessage(Message, EMessageSeverity::Error, ShouldShow, Object);
		}
	}
	
	template<EVoxelShowNotification ShouldShow = EVoxelShowNotification::Show>
	static void CondWarning(bool bCond, const FString& Message, const UObject* Object = nullptr)
	{
		CondWarning<ShouldShow>(bCond, FText::FromString(Message), Object);
	}
	template<EVoxelShowNotification ShouldShow = EVoxelShowNotification::Show>
	static void CondWarning(bool bCond, const FText& Message, const UObject* Object = nullptr)
	{
		if (bCond)
		{
			LogMessage(Message, EMessageSeverity::Warning, ShouldShow, Object);
		}
	}
	
	template<EVoxelShowNotification ShouldShow = EVoxelShowNotification::Show>
	static void CondInfo(bool bCond, const FString& Message, const UObject* Object = nullptr)
	{
		CondInfo<ShouldShow>(bCond, FText::FromString(Message), Object);
	}
	template<EVoxelShowNotification ShouldShow = EVoxelShowNotification::Show>
	static void CondInfo(bool bCond, const FText& Message, const UObject* Object = nullptr)
	{
		if (bCond)
		{
			LogMessage(Message, EMessageSeverity::Info, ShouldShow, Object);
		}
	}
};