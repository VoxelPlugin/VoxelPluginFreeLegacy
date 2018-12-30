// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Misc/MessageDialog.h"
#include "VoxelDebug/VoxelCrashReporter.h"

class FVoxelCrashReporterEditor : public IVoxelCrashReporter
{
public:
	virtual bool ShowError(const FString& Error, bool bCanIgnore) override
	{
		auto Result = FMessageDialog::Open(bCanIgnore ? EAppMsgType::YesNo : EAppMsgType::Ok, FText::FromString(TEXT("Error: ") + Error + (bCanIgnore ? TEXT("\n\nHide future errors?") : TEXT(""))));

		switch (Result)
		{
		case EAppReturnType::Yes:
			return true;
		default:
			return false;
		}
	}

	static void Register()
	{
		FVoxelCrashReporter::CrashReporter = MakeShareable(new FVoxelCrashReporterEditor());
	}
};
