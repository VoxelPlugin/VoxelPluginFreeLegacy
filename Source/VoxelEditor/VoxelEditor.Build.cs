// Copyright 2019 Phyronnaz

using System.IO;
using UnrealBuildTool;

public class VoxelEditor : ModuleRules
{
    public VoxelEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
#if UE_4_20_OR_LATER
        bLegacyPublicIncludePaths = false;
#endif

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

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
                "KismetWidgets",
                "Projects",
                "Voxel",
                "Landscape",
                "LandscapeEditor",
                "DesktopPlatform",

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

                "AdvancedPreviewScene",
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
