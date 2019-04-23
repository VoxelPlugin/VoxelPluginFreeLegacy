// Copyright 2019 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Logging/TokenizedMessage.h"

struct VOXEL_API FVoxelBPErrors
{
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnError, const TSharedRef<FTokenizedMessage>&);
	static FOnError OnScriptException;

	static void LogMessage(const TSharedRef<FTokenizedMessage>& Message);
	static void LogMessage(const FText& Message, EMessageSeverity::Type Severity, const UObject* Object = nullptr);

	static inline void Error(const FText& Message, const UObject* Object = nullptr)
	{
		LogMessage(Message, EMessageSeverity::Error, Object);
	}
	static inline void Warning(const FText& Message, const UObject* Object = nullptr)
	{
		LogMessage(Message, EMessageSeverity::Warning, Object);
	}
	static inline void Info(const FText& Message, const UObject* Object = nullptr)
	{
		LogMessage(Message, EMessageSeverity::Info, Object);
	}
};