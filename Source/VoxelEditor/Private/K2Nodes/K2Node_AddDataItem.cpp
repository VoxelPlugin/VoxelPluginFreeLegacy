// Copyright 2021 Phyronnaz

#include "K2Node_AddDataItem.h"

#include "VoxelGenerators/VoxelGeneratorInstanceWrapper.h"
#include "VoxelPlaceableItems/VoxelPlaceableItemManager.h"

#include "AssetRegistryModule.h"
#include "KismetCompiler.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node_CallFunction.h"
#include "K2Node_MakeArray.h"
#include "K2Node_MakeStruct.h"

const FName UK2Node_AddDataItem::PC_Generator = "Generator";
const FName UK2Node_AddDataItem::PC_Bounds = "Bounds";
const FName UK2Node_AddDataItem::PC_Mask = "Mask";

UEdGraphPin* FindOutputStructPinChecked(UEdGraphNode* Node)
{
	check(NULL != Node);
	UEdGraphPin* OutputPin = NULL;
	for (int32 PinIndex = 0; PinIndex < Node->Pins.Num(); ++PinIndex)
	{
		UEdGraphPin* Pin = Node->Pins[PinIndex];
		if (Pin && (EGPD_Output == Pin->Direction))
		{
			OutputPin = Pin;
			break;
		}
	}
	check(NULL != OutputPin);
	return OutputPin;
}

void UK2Node_AddDataItem::AllocateDefaultPins()
{
    Super::AllocateDefaultPins();
	
    // Execution pins
    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
    CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

	if (!ensure(DataItemConfig))
	{
		return;
	}

	if (DataItemConfig->HasAnyFlags(RF_NeedLoad))
	{
		// Preload to get correct pins
		PreloadObject(const_cast<UVoxelGraphDataItemConfig*>(DataItemConfig));
	}
	
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UVoxelPlaceableItemManager::StaticClass(), UEdGraphSchema_K2::PSC_Self);
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Object, UVoxelGeneratorInstanceWrapper::StaticClass(), PC_Generator);
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, FVoxelIntBox::StaticStruct(), PC_Bounds);
	auto* MaskPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Int, UEdGraphSchema_K2::PSC_Bitmask, StaticEnum<EVoxel32BitMask>(), PC_Mask);
	MaskPin->DefaultValue = "1";
	
	for (auto& Parameter : DataItemConfig->Parameters)
	{
	    CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Float, Parameter);
	}
}

FText UK2Node_AddDataItem::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return GetTitle(DataItemConfig ? DataItemConfig->GetName() : FString());
}

FText UK2Node_AddDataItem::GetTooltipText() const
{
    return GetTooltip(DataItemConfig ? DataItemConfig->GetName() : FString());
}

FText UK2Node_AddDataItem::GetMenuCategory() const
{
    return VOXEL_LOCTEXT("Voxel");
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UK2Node_AddDataItem::ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    Super::ExpandNode(CompilerContext, SourceGraph);

    if (!DataItemConfig) 
    {
        CompilerContext.MessageLog.Error(TEXT("DataItemConfig is not set!"), this);
        return;
    }

	TArray<UEdGraphPin*> ParameterInputPins;
	{
		TArray<FName> InputPinsName;
		for (auto& Pin : Pins)
		{
			if (Pin->Direction == EGPD_Input && Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Float)
			{
				ParameterInputPins.Add(Pin);
				InputPinsName.Add(Pin->GetFName());
			}
		}
		if (DataItemConfig->Parameters != InputPinsName)
		{
			CompilerContext.MessageLog.Error(TEXT("Outdated node @@! Right click it and click Refresh Node"), this);
			return;
		}
	}
	
	UK2Node_MakeArray* MakeArrayNode = CompilerContext.SpawnIntermediateNode<UK2Node_MakeArray>(this, SourceGraph);
	MakeArrayNode->AllocateDefaultPins();
	CompilerContext.MessageLog.NotifyIntermediateObjectCreation(MakeArrayNode, this);
	
    UK2Node_MakeStruct* MakeStruct = CompilerContext.SpawnIntermediateNode<UK2Node_MakeStruct>(this, SourceGraph);
	MakeStruct->StructType = FVoxelDataItemConstructionInfo::StaticStruct();
	MakeStruct->AllocateDefaultPins();
	MakeStruct->bMadeAfterOverridePinRemoval = true;
    CompilerContext.MessageLog.NotifyIntermediateObjectCreation(MakeStruct, this);
	
	UEdGraphPin* ArrayOut = MakeArrayNode->GetOutputPin();
	// Connect the output of the "Make Array" pin to the function's "Parameters" pin
	ArrayOut->MakeLinkTo(MakeStruct->FindPinChecked(GET_MEMBER_NAME_STRING_CHECKED(FVoxelDataItemConstructionInfo, Parameters)));
	// This will set the "Make Array" node's type, only works if one pin is connected.
	MakeArrayNode->PinConnectionListChanged(ArrayOut);

	for (int32 Index = 0; Index < ParameterInputPins.Num(); Index++)
	{
		// The "Make Array" node already has one pin available, so don't create one for ArgIdx == 0
		if (Index > 0)
		{
			MakeArrayNode->AddInputPin();
		}

		// Find the input pin on the "Make Array" node by index.
		const FString PinName = FString::Printf(TEXT("[%d]"), Index);
		UEdGraphPin* ArrayPin = MakeArrayNode->FindPinChecked(PinName);

		// Move our input pin to the array one
		CompilerContext.MovePinLinksToIntermediate(*ParameterInputPins[Index], *ArrayPin);
	}
	
    UFunction* BlueprintFunction = UVoxelPlaceableItemManager::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UVoxelPlaceableItemManager, AddDataItem));

    UK2Node_CallFunction* CallFunction = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
    CallFunction->SetFromFunction(BlueprintFunction);
    CallFunction->AllocateDefaultPins();
    CompilerContext.MessageLog.NotifyIntermediateObjectCreation(CallFunction, this);

	FindOutputStructPinChecked(MakeStruct)->MakeLinkTo(CallFunction->FindPinChecked(TEXT("Info")));

	CompilerContext.MovePinLinksToIntermediate(*FindPinChecked(UEdGraphSchema_K2::PSC_Self), *CallFunction->FindPinChecked(UEdGraphSchema_K2::PSC_Self));
	CompilerContext.MovePinLinksToIntermediate(*FindPinChecked(PC_Generator), *MakeStruct->FindPinChecked(GET_FUNCTION_NAME_STRING_CHECKED(FVoxelDataItemConstructionInfo, Generator)));
	CompilerContext.MovePinLinksToIntermediate(*FindPinChecked(PC_Bounds), *MakeStruct->FindPinChecked(GET_FUNCTION_NAME_STRING_CHECKED(FVoxelDataItemConstructionInfo, Bounds)));
	CompilerContext.MovePinLinksToIntermediate(*FindPinChecked(PC_Mask), *MakeStruct->FindPinChecked(GET_FUNCTION_NAME_STRING_CHECKED(FVoxelDataItemConstructionInfo, Mask)));

	CompilerContext.MovePinLinksToIntermediate(*GetExecPin(), *CallFunction->GetExecPin());
	CompilerContext.MovePinLinksToIntermediate(*FindPinChecked(UEdGraphSchema_K2::PN_Then), *CallFunction->GetThenPin());

    BreakAllNodeLinks();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void UK2Node_AddDataItem::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	if (!ActionRegistrar.GetActionKeyFilter())
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		
		FARFilter Filter;
		Filter.ClassNames.Add(UVoxelGraphDataItemConfig::StaticClass()->GetFName());
		Filter.bRecursiveClasses = true;

		TArray<FAssetData> Assets;
		AssetRegistryModule.Get().GetAssets(Filter, Assets);

		for (auto& Asset : Assets)
		{
			UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
			NodeSpawner->DefaultMenuSignature.MenuName = GetTitle(Asset.AssetName.ToString());
			NodeSpawner->DefaultMenuSignature.Tooltip = GetTooltip(Asset.AssetName.ToString());

			// Force load
			// This sucks, but else we get weird errors when assets are loaded as AddDataItem nodes dependencies
			// The item configs cannot have any dependencies, and are relatively small assets, so it should be fine
			Asset.GetAsset();
			
			if (Asset.IsAssetLoaded())
			{
				UVoxelGraphDataItemConfig* NewDataItemConfig = CastChecked<UVoxelGraphDataItemConfig>(Asset.GetAsset());

				NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateLambda(
				[NewDataItemConfig = MakeWeakObjectPtr(NewDataItemConfig)](UEdGraphNode* NewNode, bool /*bIsTemplateNode*/)
				{
					CastChecked<UK2Node_AddDataItem>(NewNode)->DataItemConfig = NewDataItemConfig.Get();
				});
			}
			else
			{
				NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateLambda(
				[Asset](UEdGraphNode* NewNode, bool /*bIsTemplateNode*/)
				{
					CastChecked<UK2Node_AddDataItem>(NewNode)->DataItemConfig = CastChecked<UVoxelGraphDataItemConfig>(Asset.GetAsset());
				});
			}
			
			ActionRegistrar.AddBlueprintAction(Asset, NodeSpawner);
		}
	}
	else
	{
		auto* NewDataItemConfig = Cast<UVoxelGraphDataItemConfig>(ActionRegistrar.GetActionKeyFilter());
		if (NewDataItemConfig)
		{
			UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());

			NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateLambda(
			[NewDataItemConfig = MakeWeakObjectPtr(NewDataItemConfig)](UEdGraphNode* NewNode, bool /*bIsTemplateNode*/)
			{
				CastChecked<UK2Node_AddDataItem>(NewNode)->DataItemConfig = NewDataItemConfig.Get();
			});
			
			NodeSpawner->DefaultMenuSignature.MenuName = GetTitle(NewDataItemConfig->GetName());
			NodeSpawner->DefaultMenuSignature.Tooltip = GetTooltip(NewDataItemConfig->GetName());

			ActionRegistrar.AddBlueprintAction(NewDataItemConfig, NodeSpawner);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

FText UK2Node_AddDataItem::GetTitle(const FString& AssetName)
{
    return FText::Format(VOXEL_LOCTEXT("Add Data Item: {0}"), FText::FromString(FName::NameToDisplayString(AssetName, false)));
}

FText UK2Node_AddDataItem::GetTooltip(const FString& AssetName)
{
    return FText::Format(VOXEL_LOCTEXT("Use this to add a new voxel data item with the parameters defined in {0} to your voxel world"), FText::FromString(FName::NameToDisplayString(AssetName, false)));
}