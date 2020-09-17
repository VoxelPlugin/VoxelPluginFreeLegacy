// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "VoxelGraphSchema.h"
#include "VoxelPinCategory.h"

#include "EdGraphUtilities.h"
#include "KismetPins/SGraphPinExec.h"
#include "KismetPins/SGraphPinBool.h"
#include "KismetPins/SGraphPinInteger.h"
#include "KismetPins/SGraphPinNum.h"
#include "KismetPins/SGraphPinColor.h"
#include "KismetPins/SGraphPinVector.h"

class FVoxelGraphPanelPinFactory : public FGraphPanelPinFactory
{
	virtual TSharedPtr<SGraphPin> CreatePin(UEdGraphPin* InPin) const override
	{
		if (InPin->GetSchema()->IsA(UVoxelGraphSchema::StaticClass()))
		{
			const EVoxelPinCategory Category = FVoxelPinCategory::FromString(InPin->PinType.PinCategory);

			switch (Category)
			{
			case EVoxelPinCategory::Exec:
				return SNew(SGraphPinExec, InPin);
			case EVoxelPinCategory::Boolean:
				return SNew(SGraphPinBool, InPin);
			case EVoxelPinCategory::Int:
				return SNew(SGraphPinInteger, InPin);
			case EVoxelPinCategory::Float:
				return SNew(SGraphPinNum<float>, InPin);
			case EVoxelPinCategory::Material:
				return nullptr;
			case EVoxelPinCategory::Color:
				return SNew(SGraphPinColor, InPin);
			case EVoxelPinCategory::Seed:
				return SNew(SGraphPinInteger, InPin);
			case EVoxelPinCategory::Wildcard:
				return SNew(SGraphPin, InPin);
			case EVoxelPinCategory::Vector:
				return SNew(SGraphPinVector, InPin);
			default:
				check(false);
				return nullptr;
			}
		}
		return nullptr;
	}
};