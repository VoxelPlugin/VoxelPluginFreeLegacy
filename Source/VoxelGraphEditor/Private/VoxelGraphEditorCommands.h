// Copyright 2020 Phyronnaz

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

class FVoxelGraphEditorCommands : public TCommands<FVoxelGraphEditorCommands>
{
public:
	// Constructor
	FVoxelGraphEditorCommands()
		: TCommands<FVoxelGraphEditorCommands>
		(
		"VoxelGraphEditor", // Context name for icons
		VOXEL_LOCTEXT("Voxel Graph Editor"), // Localized context name for displaying
		NAME_None, // Parent
		"VoxelGraphStyle" // Icon Style Set
		)
	{
	}

	// Compile the graph to C++
	TSharedPtr<FUICommandInfo> CompileToCpp;

	// Compile the nodes
	TSharedPtr<FUICommandInfo> RecreateNodes;
	
	// Enable auto preview update
	TSharedPtr<FUICommandInfo> ToggleAutomaticPreview;

	// Update preview
	TSharedPtr<FUICommandInfo> UpdatePreview;
	TSharedPtr<FUICommandInfo> UpdateVoxelWorlds;
	TSharedPtr<FUICommandInfo> ClearNodesMessages;

	TSharedPtr<FUICommandInfo> ShowAxisDependencies;

	TSharedPtr<FUICommandInfo> ShowStats;
	TSharedPtr<FUICommandInfo> ShowValues;

	// Adds an input to the node
	TSharedPtr<FUICommandInfo> AddInput;

	// Removes an input from the node
	TSharedPtr<FUICommandInfo> DeleteInput;

	TSharedPtr<FUICommandInfo> TogglePinPreview;
	TSharedPtr<FUICommandInfo> SplitPin;
	TSharedPtr<FUICommandInfo> CombinePin;

	// Local variables
	TSharedPtr<FUICommandInfo> SelectLocalVariableUsages;
	TSharedPtr<FUICommandInfo> SelectLocalVariableDeclaration;
	TSharedPtr<FUICommandInfo> ConvertVariablesToReroute;
	TSharedPtr<FUICommandInfo> ConvertRerouteToVariables;

	TSharedPtr<FUICommandInfo> ReconstructNode;

#define LOCTEXT_NAMESPACE "Voxel"
	// Initialize commands
	virtual void RegisterCommands() override
	{
		UI_COMMAND(CompileToCpp, "Compile To C++", "Create C++ file from graph", EUserInterfaceActionType::Button, FInputChord());
		UI_COMMAND(RecreateNodes, "Recreate Nodes", "Reconstruct all the nodes", EUserInterfaceActionType::Button, FInputChord());
		UI_COMMAND(ToggleAutomaticPreview, "Automatic Preview", "Enable Automatic Preview", EUserInterfaceActionType::ToggleButton, FInputChord());
		UI_COMMAND(UpdatePreview, "Update Preview", "Update preview", EUserInterfaceActionType::Button, FInputChord(EKeys::F5));
		UI_COMMAND(UpdateVoxelWorlds, "Update Voxel Worlds", "Update voxel worlds", EUserInterfaceActionType::Button, FInputChord(EKeys::F5, false, true, false, false));
		
		UI_COMMAND(ClearNodesMessages, "Clear Messages", "Clear nodes messages", EUserInterfaceActionType::Button, FInputChord());

		UI_COMMAND(ShowAxisDependencies, "Show Axis Dependencies", "Show Axis Dependencies", EUserInterfaceActionType::Button, FInputChord());

		UI_COMMAND(ShowStats, "Show Stats", "Show statistics about the previewed voxel. Click the graph preview to change the previewed voxel.", EUserInterfaceActionType::ToggleButton, FInputChord());
		UI_COMMAND(ShowValues, "Show Values", "Show the values of all the nodes on the previewed voxel. Click the graph preview to change the previewed voxel.", EUserInterfaceActionType::ToggleButton, FInputChord());

		UI_COMMAND(AddInput, "Add Input", "Adds an input to the node", EUserInterfaceActionType::Button, FInputChord());
		UI_COMMAND(DeleteInput, "Delete Input", "Removes an input from the node", EUserInterfaceActionType::Button, FInputChord());

		UI_COMMAND(TogglePinPreview, "Toggle pin preview", "Makes the preview viewport start/stop previewing this pin", EUserInterfaceActionType::Button, FInputChord());
		UI_COMMAND(SplitPin, "Split vector pin", "Split this pin into X Y Z pins", EUserInterfaceActionType::Button, FInputChord());
		UI_COMMAND(CombinePin, "Combine pins into vector", "Combine this pin with its neighbors to make a vector", EUserInterfaceActionType::Button, FInputChord());

		UI_COMMAND(SelectLocalVariableUsages, "Select Usages", "Select this variable usages", EUserInterfaceActionType::Button, FInputChord());
		UI_COMMAND(SelectLocalVariableDeclaration, "Select Declaration", "Select this variable declaration", EUserInterfaceActionType::Button, FInputChord());
		UI_COMMAND(ConvertVariablesToReroute, "Convert to reroute", "Convert this variable to a reroute node", EUserInterfaceActionType::Button, FInputChord());
		UI_COMMAND(ConvertRerouteToVariables, "Convert to variables", "Convert this reroute node to local variables", EUserInterfaceActionType::Button, FInputChord());

		UI_COMMAND(ReconstructNode, "Reconstruct Node", "Recreate this node to update for pins changes", EUserInterfaceActionType::Button, FInputChord());
	}
#undef LOCTEXT_NAMESPACE
};