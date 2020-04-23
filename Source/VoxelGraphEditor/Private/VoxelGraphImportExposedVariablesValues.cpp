// Copyright 2020 Phyronnaz

#include "VoxelGraphImportExposedVariablesValues.h"
#include "VoxelWorldGenerator.h"
#include "VoxelGraphGenerator.h"
#include "VoxelNodes/VoxelExposedNodes.h"
#include "VoxelNodes/VoxelGraphMacro.h"
#include "VoxelGraphErrorReporter.h"

#include "ClassViewerFilter.h"
#include "Kismet2/SClassPickerDialog.h"
#include "Engine/Blueprint.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "ScopedTransaction.h"

void ReportNodeError(FVoxelGraphErrorReporter& Reporter, const TArray<UVoxelNode*>& Nodes, const FName& Name)
{
	for (auto* Node : Nodes)
	{
		Reporter.AddMessageToNode(Node, Name.ToString() + " is used multiple times", EVoxelGraphNodeMessageType::FatalError);
	}
}

void DoImport(UVoxelGraphGenerator* Graph, UClass* From)
{
	FVoxelGraphErrorReporter::ClearMessages(Graph);
	FVoxelGraphErrorReporter::ClearNodesMessages(Graph);

	auto* DefaultObject = From->GetDefaultObject();
	TMap<FName, TArray<UVoxelExposedNode*>> NodesMap;

	TSet<UVoxelGraphGenerator*> VisitedGraphs;
	TArray<UVoxelGraphGenerator*> QueuedGraphs;
	TMap<UVoxelGraphGenerator*, TArray<UVoxelNode*>> MacroCallstack;
	TMap<UVoxelNode*, TArray<UVoxelNode*>> NodesCallstack;
	
	QueuedGraphs.Add(Graph);
	while (QueuedGraphs.Num() != 0)
	{
		auto* CurrentGraph = QueuedGraphs.Pop();
		if (VisitedGraphs.Contains(CurrentGraph))
		{
			continue;
		}
		VisitedGraphs.Add(CurrentGraph);

		for (auto* Node : CurrentGraph->AllNodes)
		{
			auto& Callstack = NodesCallstack.FindOrAdd(Node);
			Callstack.Append(MacroCallstack.FindOrAdd(CurrentGraph));
			Callstack.Add(Node);

			if (auto* ExposedNode = Cast<UVoxelExposedNode>(Node))
			{
				NodesMap.FindOrAdd(ExposedNode->UniqueName).Add(ExposedNode);
			}
			else if (auto* MacroNode = Cast<UVoxelGraphMacroNode>(Node))
			{
				if (auto* Macro = MacroNode->Macro)
				{
					MacroCallstack.FindOrAdd(Macro).Add(MacroNode);
					QueuedGraphs.Add(Macro);
				}
			}
		}
	}
	
	FString InvalidNames;
	FString ValidNames;
	int32 Count = 0;

	const FScopedTransaction Transaction(VOXEL_LOCTEXT("Import Exposed Variables"));
	for (TFieldIterator<UProperty> It(From); It; ++It)
	{
		auto* Property = *It;
		FName Name = Property->GetFName();

		if (Name == "Bounds" || Name == "bEnableRangeAnalysis")
		{
			continue;
		}
		
		bool bValid = false;
		for (auto* Node : NodesMap.FindRef(Name))
		{
			Node->Modify();
			if (Node->TryImportFromProperty(Property, DefaultObject))
			{
				Count++;
				bValid = true;
			}
		}
		auto& Names = bValid ? ValidNames : InvalidNames;
		Names += "\t" + Name.ToString() + "\n";
	}

	FMessageDialog::Open(EAppMsgType::Ok, EAppReturnType::Ok, FText::Format(
		VOXEL_LOCTEXT(
			"The following properties were imported successfully: \n{0}\n"
			"The following properties could not be imported:\n {1}"),
		FText::FromString(ValidNames),
		FText::FromString(InvalidNames)));

	FNotificationInfo Info = FNotificationInfo(FText::Format(VOXEL_LOCTEXT("{0} properties imported!"), FText::AsNumber(Count)));
	Info.CheckBoxState = ECheckBoxState::Checked;
	Info.ExpireDuration = 10;
	FSlateNotificationManager::Get().AddNotification(Info);
}

void FVoxelGraphImportExposedVariablesValues::Import(UVoxelGraphGenerator* Graph)
{
	// Fill in options
	FClassViewerInitializationOptions Options;
	Options.Mode = EClassViewerMode::ClassPicker;
	Options.DisplayMode = EClassViewerDisplayMode::ListView;
	Options.bShowObjectRootClass = true;

	// This will allow unloaded blueprints to be shown.
	Options.bShowUnloadedBlueprints = true;

	// Enable Class Dynamic Loading
	Options.bEnableClassDynamicLoading = true;

	class FVoxelWorldGeneratorFilter : public IClassViewerFilter
	{
	public:
		virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return InClass && !InClass->HasAnyClassFlags(CLASS_Abstract | CLASS_HideDropDown) && InClass->IsChildOf(UVoxelWorldGenerator::StaticClass());
		}

		virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
		{
			return InUnloadedClassData->IsChildOf(UVoxelWorldGenerator::StaticClass());
		}
	};

	Options.ClassFilter = MakeShared<FVoxelWorldGeneratorFilter>();

	UClass* ChosenClass = nullptr;
	const bool bPressedOk = SClassPickerDialog::PickClass(VOXEL_LOCTEXT("Pick Class To Import From"), Options, ChosenClass, UBlueprint::StaticClass());
	if (bPressedOk)
	{
		DoImport(Graph, ChosenClass);
	}
}