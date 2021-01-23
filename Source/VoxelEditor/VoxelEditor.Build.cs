// Copyright 2021 Phyronnaz

using System.IO;
using UnrealBuildTool;

public class VoxelEditor : ModuleRules
{
    public VoxelEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnforceIWYU = true;
        bLegacyPublicIncludePaths = false;

#if UE_4_24_OR_LATER
#else
#endif

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        PrivateIncludePaths.Add(Path.Combine(EngineDirectory, "Source/Editor/PropertyEditor/Private"));

        DynamicallyLoadedModuleNames.AddRange(
            new string[] {
                "VoxelGraphEditor",
                "AssetRegistry",
            });

        PublicDependencyModuleNames.AddRange(
            new string[] {
                "Core",
                "CoreUObject",
                "Engine",
            });

        PrivateDependencyModuleNames.AddRange(
            new string[] {
                "Voxel",
                "VoxelGraph",
                "VoxelFoliage",
                "VoxelEditorDefault",
                "Landscape",
                "LandscapeEditor",
                "PlacementMode",
                "AdvancedPreviewScene",
                "DesktopPlatform",
                "UnrealEd",
                "InputCore",
                "ImageWrapper",
                "Slate",
                "SlateCore",
                "PropertyEditor",
                "EditorStyle",
                "Projects",
                "RHI",
                "MessageLog",
                "RawMesh",
                "DetailCustomizations",
                "WorkspaceMenuStructure",
                "BlueprintGraph",
                "KismetCompiler",
                "ApplicationCore",
                "EngineSettings",
                "ToolMenus",
#if UE_4_26_OR_LATER
                "DeveloperSettings",
#endif
            });

        PrivateIncludePathModuleNames.AddRange(
            new string[] {
                "VoxelGraphEditor"
            });
    }
}
