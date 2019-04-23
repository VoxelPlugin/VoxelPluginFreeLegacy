// Copyright 2019 Phyronnaz

#include "VoxelBlueprintErrors.h"
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"

FVoxelBPErrors::FOnError FVoxelBPErrors::OnScriptException;

void FVoxelBPErrors::LogMessage(const TSharedRef<FTokenizedMessage>& Message)
{
	if (OnScriptException.IsBound())
	{
		OnScriptException.Broadcast(Message);
	}
	else
	{
		FMessageLog("PIE").AddMessage(Message);
	}
}

void FVoxelBPErrors::LogMessage(const FText& Message, EMessageSeverity::Type Severity, const UObject* Object)
{
	TSharedRef<FTokenizedMessage> NewMessage = FTokenizedMessage::Create(Severity);
	if (Object)
	{
		NewMessage->AddToken(FUObjectToken::Create(Object));
		NewMessage->AddToken(FTextToken::Create(FText::FromString(": ")));
	}
	NewMessage->AddToken(FTextToken::Create(Message));
	LogMessage(NewMessage);
}
