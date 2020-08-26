// Copyright 2020 Phyronnaz

#include "VoxelMessages.h"
#include "Logging/MessageLog.h"
#include "Misc/UObjectToken.h"
#include "Misc/MessageDialog.h"

FVoxelMessages::FLogMessageDelegate FVoxelMessages::LogMessageDelegate;
FVoxelMessages::FShowNotificationDelegate FVoxelMessages::ShowNotificationDelegate;

void FVoxelMessages::LogMessage(const TSharedRef<FTokenizedMessage>& Message, EVoxelShowNotification ShouldShow)
{
	if (!ensure(IsInGameThread())) return;
	
	if (LogMessageDelegate.IsBound())
	{
		LogMessageDelegate.Broadcast(Message, ShouldShow);
	}
	else
	{
		FMessageLog("PIE").AddMessage(Message);
	}
}

void FVoxelMessages::LogMessage(const FText& Message, EMessageSeverity::Type Severity, EVoxelShowNotification ShouldShow, const UObject* Object)
{
	if (!ensure(IsInGameThread())) return;
	
	TSharedRef<FTokenizedMessage> NewMessage = FTokenizedMessage::Create(Severity);
	if (Object)
	{
		NewMessage->AddToken(FUObjectToken::Create(Object));
		NewMessage->AddToken(FTextToken::Create(FText::FromString(": ")));
	}
	NewMessage->AddToken(FTextToken::Create(Message));
	LogMessage(NewMessage, ShouldShow);
}

void FVoxelMessages::ShowNotification(const FNotification& Notification)
{
	ensure(Notification.UniqueId != 0);
	if (!ensure(IsInGameThread())) return;
	
	if (ShowNotificationDelegate.IsBound())
	{
		ShowNotificationDelegate.Broadcast(Notification);
	}
}