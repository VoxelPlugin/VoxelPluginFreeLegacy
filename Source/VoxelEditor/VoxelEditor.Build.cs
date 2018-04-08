// Copyright 2018 Phyronnaz

using UnrealBuildTool;

public class VoxelEditor : ModuleRules
{
    public VoxelEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[]
            {
                "Public"
            });

        PrivateIncludePaths.AddRange(
            new string[] {
                "VoxelEditor/Private",
                "VoxelEditor/Private/AssetTools",
                "VoxelEditor/Private/Factories",
                "VoxelEditor/Private/Graph",
                "VoxelEditor/Private/Details",
            });

        AddEngineThirdPartyPrivateStaticDependencies(Target,
            "zlib",
            "UElibPNG",
            "UElibJPG"
        );

        DynamicallyLoadedModuleNames.AddRange(
            new string[] {
                "AssetTools",
                "MainFrame",
                "DetailCustomizations",
                "AssetRegistry",
                "PlacementMode",
            });

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "DesktopWidgets",
                "Projects",
                "Voxel",
                "Landscape",
                "LandscapeEditor",

                "Core",
                "CoreUObject",
                "ApplicationCore",
                "InputCore",
                "Engine",
                "UnrealEd",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "RenderCore",
                "LevelEditor",
                "Landscape",
                "PropertyEditor",
                "ClassViewer",
                "GraphEditor",
                "ContentBrowser",

                "BlueprintGraph",
            });

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "AssetTools",
                "UnrealEd",
                "DetailCustomizations",
                "PlacementMode",
            });
    }
}
