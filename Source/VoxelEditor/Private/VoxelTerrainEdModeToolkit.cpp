// Copyright 2018 Phyronnaz

#include "VoxelTerrainEdModeToolkit.h"
#include "VoxelTerrainEdMode.h"
#include "VoxelTerrainEdModeStyle.h"
#include "VoxelTerrainEdModeDetails.h"
#include "VoxelTerrainEdModeCommands.h"
#include "VoxelTerrainEdModeData.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "PropertyEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailPropertyRow.h"
#include "DetailWidgetRow.h"
#include "IDetailGroup.h"

#define LOCTEXT_NAMESPACE "FVoxelTerrainEdModeToolkit"

FVoxelTerrainEdModeToolkit::~FVoxelTerrainEdModeToolkit()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.UnregisterCustomClassLayout(FName("VoxelTerrainCustomLayout"));
}

void AddCommandTool(const TSharedRef<FUICommandList>& CommandList, FName ToolName, const TSharedPtr<const FUICommandInfo>& Command, FVoxelTerrainEdModeToolkit* Toolkit)
{
	CommandList->MapAction(Command,
						   FUIAction(FExecuteAction::CreateRaw(   Toolkit, &FVoxelTerrainEdModeToolkit::OnChangeTool,  FName(ToolName)),
                                     FCanExecuteAction::CreateRaw(Toolkit, &FVoxelTerrainEdModeToolkit::IsToolEnabled, FName(ToolName)),
                                     FIsActionChecked::CreateRaw( Toolkit, &FVoxelTerrainEdModeToolkit::IsToolActive,  FName(ToolName))));
}

void AddCommandMode(const TSharedRef<FUICommandList>& CommandList, FName ModeName, const TSharedPtr<const FUICommandInfo>& Command, FVoxelTerrainEdModeToolkit* Toolkit)
{
	CommandList->MapAction(Command,
						   FUIAction(FExecuteAction::CreateRaw(   Toolkit, &FVoxelTerrainEdModeToolkit::OnChangeMode,  FName(ModeName)),
                                     FCanExecuteAction::CreateRaw(Toolkit, &FVoxelTerrainEdModeToolkit::IsModeEnabled, FName(ModeName)),
                                     FIsActionChecked::CreateRaw( Toolkit, &FVoxelTerrainEdModeToolkit::IsModeActive,  FName(ModeName))));
}

void FVoxelTerrainEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	FVoxelTerrainEdModeCommands::Register();
	
	// Commands for buttons
	TSharedRef<FUICommandList> CommandList = GetToolkitCommands();
	{
		const FVoxelTerrainEdModeCommands& Commands = FVoxelTerrainEdModeCommands::Get();

		for (auto& Tool : FVoxelEditorToolOrMode::GetToolsCommands())
		{
			AddCommandTool(CommandList, Tool->GetName(), Tool->GetCommand(), this);
		}
		for (auto& Mode : FVoxelEditorToolOrMode::GetModesCommands())
		{
			AddCommandMode(CommandList, Mode->GetName(), Mode->GetCommand(), this);
		}
	}

	// Add buttons
	FToolBarBuilder ToolsSwitchButtons(CommandList, FMultiBoxCustomization::None);
	for (auto& Tool : FVoxelEditorToolOrMode::GetToolsCommands())
	{
		ToolsSwitchButtons.AddToolBarButton(Tool->GetCommand(), NAME_None, Tool->GetLabel(), Tool->GetToolTip(), FSlateIcon(FVoxelTerrainEdModeStyle::Get()->GetStyleSetName(), Tool->GetIconName()));
	}
	FToolBarBuilder ModeSwitchButtons(CommandList, FMultiBoxCustomization::None);
	for (auto& Mode : FVoxelEditorToolOrMode::GetModesCommands())
	{
		ModeSwitchButtons.AddToolBarButton(Mode->GetCommand(), NAME_None, Mode->GetLabel(), Mode->GetToolTip(), FSlateIcon(FVoxelTerrainEdModeStyle::Get()->GetStyleSetName(), Mode->GetIconName()));
	}

	// Settings panel
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		FDetailsViewArgs DetailsViewArgs(false, false, false, FDetailsViewArgs::HideNameArea);

		DetailsPanel = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
		DetailsPanel->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateRaw(this, &FVoxelTerrainEdModeToolkit::GetIsPropertyVisible));

		FVoxelTerrainEdMode* VoxelTerrainEdMode = (FVoxelTerrainEdMode*)GetEditorMode();
		if (VoxelTerrainEdMode)
		{
			DetailsPanel->SetObject(VoxelTerrainEdMode->EdModeSettings, true);
		}

		PropertyEditorModule.RegisterCustomClassLayout(FName("VoxelTerrainEdModeData"), FOnGetDetailCustomizationInstance::CreateStatic(&FVoxelTerrainEdModeDetails::MakeInstance));
	}

	SAssignNew(ToolkitWidget, SScrollBox)
		.IsEnabled(this, &FVoxelTerrainEdModeToolkit::GetIsEnabled)
		+ SScrollBox::Slot()
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(SBox)
			[
				ToolsSwitchButtons.MakeWidget()
			]
		]
		+ SScrollBox::Slot()
		.Padding(5)
		.HAlign(HAlign_Center)
		[
			SNew(SBox)
			[
				ModeSwitchButtons.MakeWidget()
			]
		]
		+ SScrollBox::Slot()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SButton)
				.ContentPadding(2)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.OnClicked(this, &FVoxelTerrainEdModeToolkit::Undo)
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(LOCTEXT("Undo", "Undo"))
				]
			]
			+ SHorizontalBox::Slot()
			[
				SNew(SButton)
				.ContentPadding(2)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.OnClicked(this, &FVoxelTerrainEdModeToolkit::Redo)
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(LOCTEXT("Redo", "Redo"))
				]
			]

		]
		+ SScrollBox::Slot()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SButton)
				.ContentPadding(2)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.OnClicked(this, &FVoxelTerrainEdModeToolkit::Undo10)
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(LOCTEXT("Undo x10", "Undo x10"))
				]
			]
			+ SHorizontalBox::Slot()
			[
				SNew(SButton)
				.ContentPadding(2)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.OnClicked(this, &FVoxelTerrainEdModeToolkit::Redo10)
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(LOCTEXT("Redo x10", "Redo x10"))
				]
			]

		]
		+ SScrollBox::Slot()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				SNew(SButton)
				.ContentPadding(2)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.OnClicked(this, &FVoxelTerrainEdModeToolkit::Undo100)
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(LOCTEXT("Undo x100", "Undo x100"))
				]
			]
			+ SHorizontalBox::Slot()
			[
				SNew(SButton)
				.ContentPadding(2)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				.OnClicked(this, &FVoxelTerrainEdModeToolkit::Redo100)
				[
					SNew(STextBlock)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(LOCTEXT("Redo x100", "Redo x100"))
				]
			]

		]
		+ SScrollBox::Slot()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(0)
			[
				DetailsPanel.ToSharedRef()
			]
		];

	FModeToolkit::Init(InitToolkitHost);
}

FName FVoxelTerrainEdModeToolkit::GetToolkitFName() const
{
	return FName("VoxelTerrainEdMode");
}

FText FVoxelTerrainEdModeToolkit::GetBaseToolkitName() const
{
	return NSLOCTEXT("VoxelTerrainEdModeToolkit", "DisplayName", "VoxelTerrainEdMode Tool");
}

class FEdMode* FVoxelTerrainEdModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FVoxelTerrainEdMode::EM_VoxelTerrainEdModeId);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////// Modes //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelTerrainEdModeToolkit::OnChangeMode(FName ModeName)
{
	FVoxelTerrainEdMode* VoxelTerrainEdMode = (FVoxelTerrainEdMode*)GetEditorMode();
	if (VoxelTerrainEdMode)
	{
		VoxelTerrainEdMode->SetCurrentMode(ModeName);
		// Refresh details panel
		DetailsPanel->SetObject(VoxelTerrainEdMode->EdModeSettings, true);
	}
}

bool FVoxelTerrainEdModeToolkit::IsModeEnabled(FName ModeName) const
{
	FVoxelTerrainEdMode* VoxelTerrainEdMode = (FVoxelTerrainEdMode*)GetEditorMode();
	
	return VoxelTerrainEdMode && !(ModeName == "FlattenMode" && VoxelTerrainEdMode->GetCurrentTool() != "ProjectionTool");
}

bool FVoxelTerrainEdModeToolkit::IsModeActive(FName ModeName) const
{
	FVoxelTerrainEdMode* VoxelTerrainEdMode = (FVoxelTerrainEdMode*)GetEditorMode();

	return VoxelTerrainEdMode && VoxelTerrainEdMode->GetCurrentMode() == ModeName;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////// Tools //////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void FVoxelTerrainEdModeToolkit::OnChangeTool(FName ToolName)
{
	FVoxelTerrainEdMode* VoxelTerrainEdMode = (FVoxelTerrainEdMode*)GetEditorMode();
	if (VoxelTerrainEdMode)
	{
		VoxelTerrainEdMode->SetCurrentTool(ToolName);
		// Refresh details panel
		DetailsPanel->SetObject(VoxelTerrainEdMode->EdModeSettings, true);
	}
}

bool FVoxelTerrainEdModeToolkit::IsToolEnabled(FName ToolName) const
{
	FVoxelTerrainEdMode* VoxelTerrainEdMode = (FVoxelTerrainEdMode*)GetEditorMode();

	return VoxelTerrainEdMode && !(VoxelTerrainEdMode->GetCurrentMode() == "FlattenMode" && ToolName != "ProjectionTool");
}

bool FVoxelTerrainEdModeToolkit::IsToolActive(FName ToolName) const
{
	FVoxelTerrainEdMode* VoxelTerrainEdMode = (FVoxelTerrainEdMode*)GetEditorMode();

	return VoxelTerrainEdMode && VoxelTerrainEdMode->GetCurrentTool() == ToolName;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FReply FVoxelTerrainEdModeToolkit::Undo()
{
	FVoxelTerrainEdMode* VoxelTerrainEdMode = (FVoxelTerrainEdMode*)GetEditorMode();

	if (VoxelTerrainEdMode)
	{
		VoxelTerrainEdMode->Undo(1);
	}

	return FReply::Handled();
}

FReply FVoxelTerrainEdModeToolkit::Redo()
{
	FVoxelTerrainEdMode* VoxelTerrainEdMode = (FVoxelTerrainEdMode*)GetEditorMode();

	if (VoxelTerrainEdMode)
	{
		VoxelTerrainEdMode->Redo(1);
	}

	return FReply::Handled();
}


FReply FVoxelTerrainEdModeToolkit::Undo10()
{
	FVoxelTerrainEdMode* VoxelTerrainEdMode = (FVoxelTerrainEdMode*)GetEditorMode();

	if (VoxelTerrainEdMode)
	{
		VoxelTerrainEdMode->Undo(10);
	}

	return FReply::Handled();
}

FReply FVoxelTerrainEdModeToolkit::Redo10()
{
	FVoxelTerrainEdMode* VoxelTerrainEdMode = (FVoxelTerrainEdMode*)GetEditorMode();

	if (VoxelTerrainEdMode)
	{
		VoxelTerrainEdMode->Redo(10);
	}

	return FReply::Handled();
}

FReply FVoxelTerrainEdModeToolkit::Undo100()
{
	FVoxelTerrainEdMode* VoxelTerrainEdMode = (FVoxelTerrainEdMode*)GetEditorMode();

	if (VoxelTerrainEdMode)
	{
		VoxelTerrainEdMode->Undo(100);
	}

	return FReply::Handled();
}

FReply FVoxelTerrainEdModeToolkit::Redo100()
{
	FVoxelTerrainEdMode* VoxelTerrainEdMode = (FVoxelTerrainEdMode*)GetEditorMode();

	if (VoxelTerrainEdMode)
	{
		VoxelTerrainEdMode->Redo(100);
	}

	return FReply::Handled();
}

bool FVoxelTerrainEdModeToolkit::GetIsEnabled() const
{
	FVoxelTerrainEdMode* VoxelTerrainEdMode = (FVoxelTerrainEdMode*)GetEditorMode();
	return VoxelTerrainEdMode && VoxelTerrainEdMode->IsEnabled();
}

bool FVoxelTerrainEdModeToolkit::GetIsPropertyVisible(const FPropertyAndParent& PropertyAndParent) const
{
	const UProperty& Property = PropertyAndParent.Property;
	FVoxelTerrainEdMode* VoxelTerrainEdMode = (FVoxelTerrainEdMode*)GetEditorMode();

	if (VoxelTerrainEdMode)
	{
		if (Property.HasMetaData("ShowForTools"))
		{
			const FName CurrentToolName = VoxelTerrainEdMode->GetCurrentTool();

			TArray<FString> ShowForTools;
			Property.GetMetaData("ShowForTools").ParseIntoArray(ShowForTools, TEXT(","), true);
			if (!ShowForTools.Contains(CurrentToolName.ToString()))
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		else if (Property.HasMetaData("ShowForModes"))
		{
			const FName CurrentModeName = VoxelTerrainEdMode->GetCurrentMode();

			TArray<FString> ShowForModes;
			Property.GetMetaData("ShowForModes").ParseIntoArray(ShowForModes, TEXT(","), true);
			if (!ShowForModes.Contains(CurrentModeName.ToString()))
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			return true;
		}
	}
	else
	{
		return false;
	}
}

#undef LOCTEXT_NAMESPACE
