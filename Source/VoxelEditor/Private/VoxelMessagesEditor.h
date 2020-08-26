// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelMessages.h"

namespace FVoxelMessagesEditor
{
	void LogMessage(const TSharedRef<FTokenizedMessage>& Message, EVoxelShowNotification ShouldShow);
	void ShowNotification(const FVoxelMessages::FNotification& Notification);
}