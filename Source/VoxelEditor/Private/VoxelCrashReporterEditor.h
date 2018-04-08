// Copyright 2018 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "MessageDialog.h"
#include "VoxelCrashReporter.h"

class FVoxelCrashReporterEditor : public IVoxelCrashReporter
{
public:
	virtual bool ShowError(const FString& Error) override
	{
		auto Result = FMessageDialog::Open(EAppMsgType::YesNo, FText::Format(FText::FromString(TEXT("Error: {0}")), FText::FromString(Error)));

		switch (Result)
		{
		case EAppReturnType::No:
			return false;
		case EAppReturnType::Yes:
			return true;
		default:
			check(false);
			return false;
		}
	}
};
