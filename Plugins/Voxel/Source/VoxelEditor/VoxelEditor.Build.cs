// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VoxelEditor : ModuleRules
{
    public VoxelEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        DynamicallyLoadedModuleNames.AddRange(
            new string[] {
                "AssetTools",
                "MainFrame",
                "DetailCustomizations",
//				"WorkspaceMenuStructure",
			});

        PrivateIncludePaths.AddRange(
            new string[] {
                "VoxelEditor/Private",
                "VoxelEditor/Private/AssetTools",
                "VoxelEditor/Private/Factories",
                "VoxelEditor/Private/Shared",
                "VoxelEditor/Private/Styles",
                "VoxelEditor/Private/Toolkits",
                "VoxelEditor/Private/Widgets",
            });

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "ContentBrowser",
                "Core",
                "CoreUObject",
                "DesktopWidgets",
                "EditorStyle",
                "Engine",
                "InputCore",
                "Projects",
                "Slate",
                "SlateCore",
                "Voxel",
                "UnrealEd",
            });

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "AssetTools",
                "UnrealEd",
//				"WorkspaceMenuStructure",
			});
    }
}
