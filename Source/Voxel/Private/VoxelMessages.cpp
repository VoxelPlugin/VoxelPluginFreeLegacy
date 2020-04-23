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

void FVoxelMessages::ShowNotification(
	uint64 UniqueId,
	const FText& Message,
	const FText& ButtonText,
	const FText& ButtonTooltip,
	const FSimpleDelegate& OnClick)
{
	if (!ensure(IsInGameThread())) return;
	
	if (ShowNotificationDelegate.IsBound())
	{
		ShowNotificationDelegate.Broadcast(UniqueId, Message, ButtonText, ButtonTooltip, OnClick);
	}
}

void FVoxelMessages::ShowVoxelPluginProError(const FString& Message, const UObject* Object)
{
	if (!ensure(IsInGameThread())) return;
	
	// This URL is to record click statistics
	// It will always redirect to https://buy.voxelplugin.com
	const FString URL = "http://bit.ly/voxelpluginpro_popup";
	const auto Popup = [=]()
	{
		FString Error;
		FPlatformProcess::LaunchURL(*URL, nullptr, &Error);
		if (!Error.IsEmpty())
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("Failed to open " + URL + "\n:" + Error));
		}
	};
	ShowNotification(GetTypeHash(Message), Message, "Get Voxel Plugin Pro", "https://buy.voxelplugin.com", FSimpleDelegate::CreateLambda(Popup));
	LogMessage(FText::FromString(Message), EMessageSeverity::Error, EVoxelShowNotification::Hide, Object);
}
